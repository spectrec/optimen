#include <ev.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "log.h"
#include "tbuf.h"
#include "libev.h"
#include "config.h"

struct libev_conn {
	uint32_t conn_id;

	struct ev_io io_w;
	struct ev_timer timer_w;

	libev_cb cb;
	struct libev_conn_ctx ctx;

	struct tbuf read_buffer;
	struct tbuf write_buffer;
};

static uint32_t __conn_id;
static uint32_t __connections_count;

static struct ev_loop *__loop;
static struct ev_io __master_watcher;

static struct ev_signal __sigint_watcher;
static struct ev_signal __sighup_watcher;
static struct ev_signal __sigterm_watcher;
static struct ev_signal __sigquit_watcher;
static struct ev_signal __sigpipe_watcher;

static libev_cb __default_read_cb;
static libev_ctx_init_cb __ctx_init_cb;
static libev_ctx_destroy_cb __ctx_destroy_cb;

static void libev_read_cb(EV_P_ ev_io *w, int revent);
static void libev_write_cb(EV_P_ ev_io *w, int revent);
static void libev_timeout_cb(EV_P_ ev_timer *w, int revents);
static void libev_accept_new_client_cb(EV_P_ ev_io *w, int revents);

static int libev_set_socket_nonblock(int fd)
{
	assert(fd != -1);

	int flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		log_e("can't get socket flags: %s", strerror(errno));
		return -1;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		log_e("can't set socket nonblocking: %s", strerror(errno));
		return -1;
	}

	return 0;
}

#define LISTEN_QUEUE_SIZE 10
static int libev_create_tcp_socket()
{
	const struct config *config = config_get_config();

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		log_e("can't create socket: %s", strerror(errno));

		return -1;
	}

	long reuse_addr = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) != 0) {
		log_e("can't setsockopt: %s", strerror(errno));
		(void)close(fd);

		return -1;
	}

	if (libev_set_socket_nonblock(fd) != 0) {
		(void)close(fd);

		return -1;
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));

	sa.sin_family = AF_INET;
	sa.sin_port = htons(config->listen_port);
	sa.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
		log_e("bind error: %s", strerror(errno));
		(void)close(fd);

		return -1;
	}

	if (listen(fd, LISTEN_QUEUE_SIZE) != 0) {
		log_e("listen error: %s", strerror(errno));
		(void)close(fd);

		return -1;
	}

	return fd;
}

static void libev_cleanup_conn(struct libev_conn *lc)
{
	tbuf_delete(&lc->read_buffer);
	tbuf_delete(&lc->write_buffer);

	lc->cb = NULL;

	__ctx_destroy_cb(&lc->ctx);

	(void)close(lc->io_w.fd);

	ev_io_stop(__loop, &lc->io_w);
	ev_timer_stop(__loop, &lc->timer_w);

	__connections_count--;

	free(lc);
}

static void libev_invert_event(struct libev_conn *lc)
{
	const struct config *config = config_get_config();

	ev_timer_stop(__loop, &lc->timer_w);
	ev_io_stop(__loop, &lc->io_w);

	if ((lc->io_w.events & EV_READ) != 0) {
		ev_timer_set(&lc->timer_w, config->timeout_write, 0.);
		ev_io_set(&lc->io_w, lc->io_w.fd, EV_WRITE);
		ev_set_cb(&lc->io_w, libev_write_cb);
		log_d("conn [%d]: wait for write event", lc->conn_id);
	} else {
		ev_timer_set(&lc->timer_w, config->timeout_read, 0.);
		ev_io_set(&lc->io_w, lc->io_w.fd, EV_READ);
		ev_set_cb(&lc->io_w, libev_read_cb);
		log_d("conn [%d]: wait for read event", lc->conn_id);
	}

	ev_io_start(__loop, &lc->io_w);
	ev_timer_start(__loop, &lc->timer_w);
}

#define DEFAULT_READ_LEN 1024
static void libev_read_cb(EV_P_ ev_io *w, int revent)
{
	struct libev_conn *lc = (struct libev_conn *)(((char *)w) - offsetof(struct libev_conn, io_w));
	const struct config *config = config_get_config();

	(void)revent;

	char buf[DEFAULT_READ_LEN + 1] = {0};
	ssize_t ret = recv(w->fd, buf, sizeof(buf) - 1, 0);

	if (ret < 0) {
		log_e("recv error: %s", strerror(errno));
		libev_cleanup_conn(lc);

		return;
	}

	if (ret == 0) {
		log_i("close connection with client [%d]", lc->conn_id);
		libev_cleanup_conn(lc);

		return;
	}

	tbuf_append(&lc->read_buffer, buf, ret);
	log_d("readed from [%d]: `%.*s'", lc->conn_id, lc->read_buffer.size, lc->read_buffer.data);

	ev_timer_stop(loop, &lc->timer_w);
	ev_timer_set(&lc->timer_w, config->timeout_read, 0.);
	ev_timer_start(loop, &lc->timer_w);

	ret = lc->cb(lc->read_buffer.data, lc->read_buffer.size, &lc->ctx);
	if (ret > 0) {
		tbuf_shrink(&lc->read_buffer, ret);
		log_d("conn [%d], read buffer after shrink: `%.*s'",
		      lc->conn_id, lc->read_buffer.size, lc->read_buffer.data);
	}
}

static void libev_write_cb(EV_P_ ev_io *w, int revent)
{
	struct libev_conn *lc = (struct libev_conn *)(((char *)w) - offsetof(struct libev_conn, io_w));
	const struct config *config = config_get_config();

	(void)revent;

	ssize_t ret = send(w->fd, lc->write_buffer.data, lc->write_buffer.size, 0);
	if (ret < 0) {
		log_e("send error: %s", strerror(errno));
		libev_cleanup_conn(lc);

		return;
	}

	if (ret == 0 && lc->write_buffer.size != 0) {
		log_w("nothing was sent by [%d], try again", lc->conn_id);

		return;
	}

	ev_timer_stop(loop, &lc->timer_w);
	ev_timer_set(&lc->timer_w, config->timeout_write, 0.);
	ev_timer_start(loop, &lc->timer_w);

	tbuf_shrink(&lc->write_buffer, ret);
	log_d("conn [%d], write buffer after shrink: `%.*s'",
	      lc->conn_id, lc->write_buffer.size, lc->write_buffer.data);

	if (lc->write_buffer.size != 0)
		return;

	libev_invert_event(lc);
}

static void libev_timeout_cb(EV_P_ ev_timer *w, int revents)
{
	struct libev_conn *lc = (struct libev_conn *)(((char *)w) - offsetof(struct libev_conn, timer_w));
	(void)loop; (void)revents;

	log_w("timeout on connection: %d, closing ...", lc->conn_id);
	libev_cleanup_conn(lc);
}

static void libev_accept_new_client_cb(EV_P_ ev_io *w, int revents)
{
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);

	(void)revents;

	int client_fd = accept(w->fd, (struct sockaddr *)&sa, &sa_len);
	if (client_fd < 0) {
		log_e("can't accept new client: %s", strerror(errno));

		return;
	}

	const struct config *config = config_get_config();
	if (__connections_count >= config->max_connections) {
		log_e("connections limit, can't accept new one");
		(void)close(client_fd);

		return;
	}

	if (libev_set_socket_nonblock(client_fd) != 0) {
		(void)close(client_fd);

		return;
	}

	assert(__ctx_init_cb != NULL);
	assert(__ctx_destroy_cb != NULL);
	assert(__default_read_cb != NULL);

	struct libev_conn *lc = (struct libev_conn *)calloc(1, sizeof(*lc));
	__ctx_init_cb(&lc->ctx);
	lc->cb = __default_read_cb;

	tbuf_init(&lc->read_buffer);
	tbuf_init(&lc->write_buffer);

	ev_io_init(&lc->io_w, libev_read_cb, client_fd, EV_READ);
	ev_io_start(loop, &lc->io_w);

	ev_timer_init(&lc->timer_w, libev_timeout_cb, config->timeout_read, 0.);
	ev_timer_start(loop, &lc->timer_w);

	lc->conn_id = __conn_id++;
	__connections_count++;

	char ip[INET_ADDRSTRLEN + 1] = {0};
	if (inet_ntop(AF_INET, &sa.sin_addr.s_addr, ip, sizeof(ip) - 1) == NULL)
		log_e("can't convert ip: %s", strerror(errno));

	log_i("accepted new client, id: `%d', from: `%s'", lc->conn_id, ip);
}

static void libev_init_watcher(int fd)
{
	assert(fd != -1);

	ev_io_init(&__master_watcher, libev_accept_new_client_cb, fd, EV_READ);
	ev_io_start(__loop, &__master_watcher);
}

static void libev_signal_handler(EV_P_ ev_signal *w, int revents)
{
	(void)revents;

	switch (w->signum) {
	case SIGPIPE:
		log_w("received SIGPIPE");
		return;

	case SIGINT:
	case SIGTERM:
	case SIGQUIT:
		log_w("received signal `%d': exiting...", w->signum);
		ev_break(loop, EVBREAK_ALL);

		return;

	case SIGHUP:
		log_w("received SIGHUP, reloading config");
		config_deinitialize();

		if (config_initialize(NULL, false) != 0)
			log_e("some errors occurred while reloading config, check logs");
		return;

	default:
		log_e("unknown signal received, num: `%d'", w->signum);
	}
}

static void libev_init_signal_handlers()
{
	ev_signal_init(&__sigpipe_watcher, libev_signal_handler, SIGPIPE);
	ev_signal_start(__loop, &__sigpipe_watcher);

	ev_signal_init(&__sigint_watcher, libev_signal_handler, SIGINT);
	ev_signal_start(__loop, &__sigint_watcher);

	ev_signal_init(&__sigterm_watcher, libev_signal_handler, SIGTERM);
	ev_signal_start(__loop, &__sigterm_watcher);

	ev_signal_init(&__sigquit_watcher, libev_signal_handler, SIGQUIT);
	ev_signal_start(__loop, &__sigquit_watcher);

	ev_signal_init(&__sighup_watcher, libev_signal_handler, SIGHUP);
	ev_signal_start(__loop, &__sighup_watcher);
}

void libev_set_read_cb(libev_cb new_cb)
{
	__default_read_cb = new_cb;
}

void libev_set_ctx_init_cb(libev_ctx_init_cb new_cb)
{
	__ctx_init_cb = new_cb;
}

void libev_set_ctx_destroy_cb(libev_ctx_destroy_cb new_cb)
{
	__ctx_destroy_cb = new_cb;
}

int libev_initialize()
{
	int sock_fd = libev_create_tcp_socket();
	if (sock_fd == -1)
		return -1;

	__loop = EV_DEFAULT;
	libev_init_watcher(sock_fd);

	libev_init_signal_handlers();

	return 0;
}

int libev_loop()
{
	return ev_run(__loop, 0);
}

void libev_send(const void *data, size_t size, void *ctx)
{
	struct libev_conn *lc = (struct libev_conn *)((char *)ctx - offsetof(struct libev_conn, ctx));

	log_i("sending response to client [%d]: `%.*s'", lc->conn_id, size, data);

	tbuf_append(&lc->write_buffer, data, size);
	libev_invert_event(lc);
}
