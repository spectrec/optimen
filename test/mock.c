#include "mock.h"

#include "log.h"
#include "tbuf.h"
#include "libev.h"

static struct libev_conn_ctx __ctx;
static __thread struct tbuf __optimen_resp;
static libev_cb __optimen_read_cb;
static libev_ctx_init_cb __optimen_init_ctx_cb;
static libev_ctx_destroy_cb __optimen_destroy_ctx_cb;

__attribute__((destructor))
static void cleaner(void)
{
	if (__optimen_destroy_ctx_cb != NULL)
		__optimen_destroy_ctx_cb(&__ctx);
}

void write_log(enum log_level level, const char *fmt, ...)
{
	(void)level;
	(void)fmt;
}

void libev_set_ctx_init_cb(libev_ctx_init_cb cb)
{
	__optimen_init_ctx_cb = cb;
}

void libev_set_ctx_destroy_cb(libev_ctx_destroy_cb cb)
{
	__optimen_destroy_ctx_cb = cb;
}

void libev_set_read_cb(libev_cb new_cb)
{
	__optimen_read_cb = new_cb;
}

void libev_send(const void *data, size_t size, void *ctx)
{
	(void)ctx;

	tbuf_insert(&__optimen_resp, data, size);
}

void send_to_optimen(const char *data, size_t size)
{
	if (__ctx.data == NULL) {
		assert(__optimen_init_ctx_cb != NULL);
		__optimen_init_ctx_cb(&__ctx);
	}

	assert(__optimen_read_cb != NULL);
	__optimen_read_cb(data, size, &__ctx);
}

void recv_from_optimen(const char **data, size_t *size)
{
	assert(data != NULL && size != NULL);

	*data = __optimen_resp.data;
	*size = __optimen_resp.size;
}
