#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"

static struct config *__config;
static const char *__path_to_config;

__attribute__((destructor))
static void config_destroy()
{
	if (__config != NULL)
		config_deinitialize();
}

static void close_file(FILE **f)
{
	if (*f != NULL)
		fclose(*f);
}

#define CONFIG_SAFE_ADD_STRING(_field, _value) do {	\
	if ((_field) != NULL) {				\
		free(_field);				\
		(_field) = NULL;			\
	}						\
	_field = strdup(_value);			\
} while (0)

#define CONFIG_CHECK_ASSIGN_INT(_field, _value) ({	\
	int32_t ret = 0;				\
	char *endptr = NULL;				\
							\
	_field = strtol(_value, &endptr, 10);		\
	if (errno == ERANGE || *endptr != '\0')		\
		ret = -1;				\
							\
	ret;						\
})

#define CONFIG_CHECK_RET_ERROR(_field, _value) do {		\
	if (CONFIG_CHECK_ASSIGN_INT(_field, _value) != 0) {	\
		fprintf(stderr, "config: invalid value `%s'\n",	\
				_value);			\
		return -1;					\
	}							\
} while (0)

static int add_to_config(const char *key, const char *value, bool verbose)
{
	assert(key != NULL && value != NULL);

	if (strcmp(key, "user") == 0) {
		CONFIG_SAFE_ADD_STRING(__config->user, value);
	} else if (strcmp(key, "pid_file") == 0) {
		CONFIG_SAFE_ADD_STRING(__config->pid_file, value);
	} else if (strcmp(key, "root_directory") == 0) {
		CONFIG_SAFE_ADD_STRING(__config->root_dir, value);
	} else if (strcmp(key, "listen_port") == 0) {
		CONFIG_CHECK_RET_ERROR(__config->listen_port, value);
	} else if (strcmp(key, "timeout_read") == 0) {
		CONFIG_CHECK_RET_ERROR(__config->timeout_read, value);
		__config->timeout_read /= 1000.0f;
	} else if (strcmp(key, "timeout_write") == 0) {
		CONFIG_CHECK_RET_ERROR(__config->timeout_write, value);
		__config->timeout_write /= 1000.0f;
	} else if (strcmp(key, "max_connections") == 0) {
		CONFIG_CHECK_RET_ERROR(__config->max_connections, value);
	} else if (strcmp(key, "log_level") == 0) {
		CONFIG_CHECK_RET_ERROR(__config->log_level, value);
	} else {
		fprintf(stderr, "unknown key: `%s' with value: `%s'\n", key, value);

		return -1;
	}

	if (verbose == true)
		fprintf(stderr, "config: parameter `%s' set to `%s'\n", key, value);

	return 0;
}

#define MAX_CONFIG_LINE_LEN 256
static int parse_config_line(FILE *f, bool verbose)
{
	char buf[MAX_CONFIG_LINE_LEN + 1] = {0};
	if (fgets(buf, sizeof(buf) - 1, f) == NULL) {
		if (ferror(f) == true) {
			fprintf(stderr, "fgets error: `%s'", strerror(errno));

			return -1;
		}

		return 0; // EOF
	}

	char *pnt;
	for (pnt = buf; *pnt != '\0' && isspace(*pnt); ++pnt); // skip spaces

	if (*pnt == '\0') // empty line
		return 0;
	if (*pnt == '#') // comment
		return 0;

	assert(isspace(*pnt) == 0);

	// grep `key'
	char *key_beg = pnt, *key_end = NULL;
	for (char *c = key_beg + 1; *c != '\0'; ++c) {
		if (isspace(*c) != 0) {
			key_end = c;
			break;
		}
	}
	if (key_end == NULL) {
		fprintf(stderr, "invalid config file, check line: `%s'\n", buf);

		return -1;
	}

	for (pnt = key_end + 1; *pnt != '\0' && isspace(*pnt); ++pnt); // skip spaces
	if (*pnt == '\0') {
		fprintf(stderr, "invalid config file, check line: `%s'\n", buf);

		return -1;
	}

	// grep `value'
	char *value_beg = pnt, *value_end = NULL, *c;
	for (c = value_beg + 1; *c != '\0'; ++c) {
		if (isspace(*c)) {
			value_end = c;
			break;
		}
	}
	if (value_end == NULL)
		value_end = c;

	return add_to_config(strndupa(key_beg, key_end - key_beg),
			     strndupa(value_beg, value_end - value_beg),
			     verbose);
}

int config_initialize(const char *path, bool verbose)
{
	assert(__config == NULL);

	if (path == NULL) {
		path = __path_to_config;
		if (__path_to_config == NULL) {
			fprintf(stderr, "using default config path: `%s'\n", DEFAULT_CONFIG_PATH);
			path = DEFAULT_CONFIG_PATH;
		}
	}

	FILE *f __attribute__((cleanup(close_file))) = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "can't open file: `%s': %s", path, strerror(errno));

		return -1;
	}

	__config = (struct config *)calloc(1, sizeof(struct config));
	while (feof(f) == false && ferror(f) == false) {
		if (parse_config_line(f, verbose) != 0) {
			config_deinitialize();

			return -1;
		}
	}

	__path_to_config = path;

	return 0;
}

void config_deinitialize()
{
	if (__config == NULL)
		return;

	free(__config->user);
	free(__config->pid_file);
	free(__config->root_dir);

	free(__config);
	__config = NULL;
}

const struct config *config_get_config()
{
	assert(__config != NULL);

	return __config;
}
