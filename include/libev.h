#ifndef __CONN_H__
#define __CONN_H__

#include <stddef.h>
#include <sys/types.h>

struct libev_conn_ctx {
	void *data;
};

typedef void (*libev_ctx_init_cb)(struct libev_conn_ctx *ctx);
typedef void (*libev_ctx_destroy_cb)(struct libev_conn_ctx *ctx);

typedef ssize_t (*libev_cb)(const char *data, size_t size, struct libev_conn_ctx *ctx);

int libev_loop();
int libev_initialize();

void libev_set_read_cb(libev_cb new_cb);
void libev_set_ctx_init_cb(libev_ctx_init_cb);
void libev_set_ctx_destroy_cb(libev_ctx_destroy_cb);

void libev_send(const void *data, size_t size, void *ctx);

#endif
