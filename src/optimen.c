#include "libev.h"
#include "optimen.h"

static ssize_t optimen_process_command_cb(const char *data, size_t size,
					  struct libev_conn_ctx *ctx)
{
	return 0;
}

static void optimen_ctx_init(struct libev_conn_ctx *ctx)
{
}

static void optimen_ctx_destroy(struct libev_conn_ctx *ctx)
{
}

void optimen_initialize()
{
	libev_set_ctx_init_cb(optimen_ctx_init);
	libev_set_ctx_destroy_cb(optimen_ctx_destroy);
	libev_set_read_cb(optimen_process_command_cb);
}
