#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "log.h"
#include "tbuf.h"
#include "libev.h"
#include "optimen.h"

#define STR_SIZE(str_) ((str_)), sizeof((str_)) - 1
#define OK(str_) STR_SIZE("OK " str_ "\r\n")
#define ERROR(str_) STR_SIZE("ERROR " str_ "\r\n")

enum optimen_ret {
	OPTIMEN_RET_OK = 0,
	OPTIMEN_RET_ERROR = 1,
};

enum optimen_command {
	OPTIMEN_COMMAND_UNKNOWN = 0,
	OPTIMEN_COMMAND_LS = 1,
	OPTIMEN_COMMAND_FILE_OPEN = 2,
	OPTIMEN_COMMAND_FILE_READ = 3,
	OPTIMEN_COMMAND_FILE_CLOSE = 4,
	OPTIMEN_COMMAND_MAX,
};

enum optimen_file_type {
	OPTIMEN_FILE_TYPE_REG,
	OPTIMEN_FILE_TYPE_DIR,
	OPTIMEN_FILE_TYPE_OTHER,
};

struct optimen_ctx {
	FILE *f;
	off_t current_offset;
};

typedef enum optimen_ret (* optimen_command_handler_t)(struct tbuf *resp,
						       const char *args,
						       struct optimen_ctx *ctx);

static enum optimen_ret optimen_process_command_unknown(struct tbuf *resp,
							const char *args,
							struct optimen_ctx *ctx);
static enum optimen_ret optimen_process_command_ls(struct tbuf *resp,
						   const char *args,
						   struct optimen_ctx *ctx);
static enum optimen_ret optimen_process_command_file_open(struct tbuf *resp,
							  const char *args,
							  struct optimen_ctx *ctx);
static enum optimen_ret optimen_process_command_file_read(struct tbuf *resp,
							  const char *args,
							  struct optimen_ctx *ctx);
static enum optimen_ret optimen_process_command_file_close(struct tbuf *resp,
							   const char *args,
							   struct optimen_ctx *ctx);

static optimen_command_handler_t optimen_command_handlers[OPTIMEN_COMMAND_MAX] = {
	[OPTIMEN_COMMAND_UNKNOWN] = optimen_process_command_unknown,
	[OPTIMEN_COMMAND_LS] = optimen_process_command_ls,
	[OPTIMEN_COMMAND_FILE_OPEN] = optimen_process_command_file_open,
	[OPTIMEN_COMMAND_FILE_READ] = optimen_process_command_file_read,
	[OPTIMEN_COMMAND_FILE_CLOSE] = optimen_process_command_file_close
};

static enum optimen_command optimen_detect_command(const char *command)
{
	if (strncasecmp(command, STR_SIZE("ls")) == 0)
		return OPTIMEN_COMMAND_LS;
	if (strncasecmp(command, STR_SIZE("file_open")) == 0)
		return OPTIMEN_COMMAND_FILE_OPEN;
	if (strncasecmp(command, STR_SIZE("file_read")) == 0)
		return OPTIMEN_COMMAND_FILE_READ;
	if (strncasecmp(command, STR_SIZE("file_close")) == 0)
		return OPTIMEN_COMMAND_FILE_CLOSE;

	return OPTIMEN_COMMAND_UNKNOWN;
}

static enum optimen_ret optimen_process_command_unknown(struct tbuf *resp,
							const char *args,
							struct optimen_ctx *ctx)
{
	(void)args;
	(void)ctx;

	tbuf_append(resp, ERROR("unknown command"));

	return OPTIMEN_RET_OK;
}

static void close_dir(DIR **d)
{
	if (d != NULL && *d != NULL)
		(void)closedir(*d);
}

static char optimen_get_char_file_type(enum optimen_file_type type)
{
	switch (type) {
	case OPTIMEN_FILE_TYPE_REG:
		return 'F';
	case OPTIMEN_FILE_TYPE_DIR:
		return 'D';
	default:
		abort();
	}
}

static enum optimen_file_type optimen_detect_file_type(uint8_t d_type,
						       const char *full_path,
						       const char *file_name,
						       struct tbuf *resp)
{
	enum optimen_file_type type;

	if (d_type == DT_UNKNOWN) {
		struct stat st;
		char path[256] = {0};

		(void)snprintf(path, sizeof(path) - 1, "%s/%s", full_path, file_name);
		if (stat(path, &st) != 0) {
			tbuf_insert(resp, ERROR("can't detect file type"));
			log_e("can't stat file `%s': %s", path, strerror(errno));

			return OPTIMEN_RET_ERROR;
		}

		switch (st.st_mode & S_IFMT) {
		case S_IFREG:
			type = OPTIMEN_FILE_TYPE_REG;
			break;
		case S_IFDIR:
			type = OPTIMEN_FILE_TYPE_DIR;
			break;
		default:
			type = OPTIMEN_FILE_TYPE_OTHER;
		}
	} else {
		switch (d_type) {
		case DT_DIR:
			type = OPTIMEN_FILE_TYPE_DIR;
			break;
		case DT_REG:
			type = OPTIMEN_FILE_TYPE_REG;
			break;
		default:
			type = OPTIMEN_FILE_TYPE_OTHER;
		}
	}

	return type;
}

static enum optimen_ret optimen_process_command_ls(struct tbuf *resp,
						   const char *args,
						   struct optimen_ctx *ctx)
{
	if (ctx->f != NULL) {
		tbuf_insert(resp, ERROR("'ls' - is not allowed now"));

		return OPTIMEN_RET_ERROR;
	}

	if (*args == '\0') {
		tbuf_insert(resp, ERROR("'ls' - required argument"));

		return OPTIMEN_RET_ERROR;
	}

	DIR *d __attribute__((cleanup(close_dir))) = opendir(args);
	if (d == NULL) {
		log_e("can't open dir `%s': %s", args, strerror(errno));
        tbuf_insert(resp, ERROR("can't open specified directory"));

		return OPTIMEN_RET_ERROR;
	}

	tbuf_insert(resp, OK("list files:"));

	int ret = 0;
	struct dirent dent, *pdent = NULL;
	while ((ret = readdir_r(d, &dent, &pdent)) == 0) {
		enum optimen_file_type type;

		if (pdent == NULL) // all readed
			break;

		type = optimen_detect_file_type(dent.d_type, args, dent.d_name, resp);
		if (type != OPTIMEN_FILE_TYPE_OTHER) {
			char file_type = optimen_get_char_file_type(type);
			tbuf_printf(resp, "%c %s\r\n", file_type, dent.d_name);
		}
	}

	if (ret != 0) {
		log_e("readdir_r error: %s", strerror(errno));
		tbuf_insert(resp, ERROR("error occurred, while reading directory"));

		return OPTIMEN_RET_ERROR;
	}

	tbuf_append(resp, STR_SIZE(".\r\n"));

	return OPTIMEN_RET_OK;
}

static enum optimen_ret optimen_process_command_file_open(struct tbuf *resp,
							  const char *args,
							  struct optimen_ctx *ctx)
{
	if (ctx->f != NULL) {
		tbuf_insert(resp, ERROR("'file_open' - file is open already"));

		return OPTIMEN_RET_ERROR;
	}

	if (*args == '\0') {
		tbuf_insert(resp, ERROR("'file_open' - required argument"));

		return OPTIMEN_RET_ERROR;
	}

	ctx->current_offset = 0;

	ctx->f = fopen(args, "rb");
	if (ctx->f == NULL) {
		log_e("can't open file `%s': %s", args, strerror(errno));
		tbuf_insert(resp, ERROR("'file_open' - can't open file"));

		return OPTIMEN_RET_ERROR;
	}

	tbuf_insert(resp, OK(""));

	return OPTIMEN_RET_OK;
}

static enum optimen_ret optimen_process_command_file_read(struct tbuf *resp,
							  const char *args,
							  struct optimen_ctx *ctx)
{
	static __thread void *buf;
	static __thread size_t buf_size;

	if (ctx->f == NULL){
		tbuf_insert(resp, ERROR("'file_read' - file isn't open yet"));

		return OPTIMEN_RET_ERROR;
	}
	// offset
	const char *args_begin = args;

	while (*args && isspace(*args) == 0)
		args++;

	const char *string_offset = strndupa(args_begin, args - args_begin);

	if (*string_offset == '\0') {
		tbuf_insert(resp, ERROR("'file_read' - required offset argument"));

		return OPTIMEN_RET_ERROR;
	}
	// spaces
	while (*args && isspace(*args) != 0)
		args++;

	args_begin = args;
	while (*args)
		args++;
	// size
	const char *string_size = strndupa(args_begin, args - args_begin);

	if (*string_size == '\0') {
		tbuf_insert(resp, ERROR("'file_read' - required size argument"));

		return OPTIMEN_RET_ERROR;
	}

	char *end = NULL;
	off_t offset = strtoll(string_offset, &end, 10);
	if (*end != '\0') {
		log_e("can't convert offset from `%s'", string_offset);
		tbuf_insert(resp, ERROR("'file_read' - can't convert offset"));

		return OPTIMEN_RET_ERROR;
	}

	off_t size = strtoll(string_size, &end, 10);
	if (*end != '\0' && *end != '\r' && *end != '\n') {
		log_e("can't convert size from `%s'", string_size);
		tbuf_insert(resp, ERROR("'file_read' - can't convert size"));

		return OPTIMEN_RET_ERROR;
	}

	if (ctx->current_offset != offset && fseek(ctx->f, offset, SEEK_SET) != 0) {
		log_e("can't fseek to `%lld': %s", offset, strerror(errno));
		tbuf_insert(resp, ERROR("'file_read' - can't fseek"));

		return OPTIMEN_RET_ERROR;
	}

	if ((size_t)size > buf_size) {
		void *tmp = realloc(buf, size + 1);
		if (!tmp) {
			log_e("can't realloc: %s", strerror(errno));
			tbuf_insert(resp, ERROR("'file_read' - internal error"));

			return OPTIMEN_RET_ERROR;
		}

		buf = tmp;
		buf_size = size + 1;
	}

	size_t bytes = fread(buf, 1, size, ctx->f);
	if (bytes <= 0) {
		if (feof(ctx->f) == 0 && ferror(ctx->f) != 0) {
			log_e("error occured, while reading data: %s", strerror(errno));
			tbuf_insert(resp, ERROR("'file_read' - internal error"));

			return OPTIMEN_RET_ERROR;
		}
	}

	ctx->current_offset = offset + bytes;

	tbuf_reset(resp);
	tbuf_printf(resp, "OK %zu\r\n", bytes);
	tbuf_append(resp, buf, bytes);

	return OPTIMEN_RET_OK;
}

static enum optimen_ret optimen_process_command_file_close(struct tbuf *resp,
							   const char *args,
							   struct optimen_ctx *ctx)
{
	if (ctx->f == NULL) {
		tbuf_insert(resp, ERROR("'file_close' - file isn't open yet"));

		return OPTIMEN_RET_ERROR;
	}

	(void)args;

	if (fclose(ctx->f) != 0) {
		log_e("can't close file: %s", strerror(errno));
		tbuf_insert(resp, ERROR("can't close file"));

		return OPTIMEN_RET_ERROR;
	}

	ctx->f = NULL;
	ctx->current_offset = 0;

	tbuf_insert(resp, OK(""));

	return OPTIMEN_RET_OK;
}

static ssize_t optimen_read_command_cb(const char *data, size_t size,
				       struct libev_conn_ctx *ctx)
{
	static __thread struct tbuf resp = {
		.data = NULL,
		.size = 0,
		.alloc_size = 0,
	};

	const size_t command_size = size;
	const char *command_beg = data;
	const char *command_end = NULL;
	bool need_remove_new_line = false;

	for (size_t i = 0; i < size; ++i) {
		if (data[i] == '\r' || data[i] == '\n') {
			command_end = &data[i];
			need_remove_new_line = i < size && data[i] == '\r'
							&& data[i + 1] == '\n';

			break;
		}
	}

	if (command_end == NULL)
		// command readed not fully
		return 0;

	// skip spaces
	while (size > 0 && isspace(*data)) {
		data++;
		size--;
	}

	const char *cmd_beg = data;
	while (size > 0 && data < command_end && isspace(*data) == 0) {
		data++;
		size--;
	}
	char *command = strndupa(cmd_beg, data - cmd_beg);

	// skip spaces
	while (size > 0 && data < command_end && isspace(*data)) {
		data++;
		size--;
	}
	const char *args = strndupa(data, command_end - data);

	log_i("received command: `%s' with args: `%s'", command, args);

	enum optimen_command cmd = optimen_detect_command(command);

	tbuf_reset(&resp);
	(void)optimen_command_handlers[cmd](&resp, args, ctx->data);

	assert(resp.size != 0 && resp.data != NULL);
	libev_send(resp.data, resp.size, ctx);

	ptrdiff_t shrink_size = command_end - command_beg + 1;
	if (need_remove_new_line == true)
		shrink_size++;

	assert(shrink_size <= (ptrdiff_t)command_size);

	return shrink_size;
}

static void optimen_ctx_init(struct libev_conn_ctx *ctx)
{
	ctx->data = calloc(1, sizeof(struct optimen_ctx));
}

static void optimen_ctx_destroy(struct libev_conn_ctx *ctx)
{
	struct optimen_ctx *tmp = (struct optimen_ctx *)(ctx->data);
	if (tmp == NULL)
		return;

	if (tmp->f != NULL) {
		fclose(tmp->f);
		tmp->f = NULL;
	}

	free(ctx->data);
	ctx->data = NULL;
}

void optimen_initialize()
{
	libev_set_ctx_init_cb(optimen_ctx_init);
	libev_set_ctx_destroy_cb(optimen_ctx_destroy);
	libev_set_read_cb(optimen_read_command_cb);
}
