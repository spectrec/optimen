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

void close_file(FILE **f)
{
	if (*f != NULL)
		fclose(*f);
}

int add_to_config(const char *key, const char *value)
{
	assert(key != NULL && value != NULL);

	if (strcmp(key, "user") == 0) {
		__config->user = strdup(value);
	} else if (strcmp(key, "pid_file") == 0) {
		__config->pid_file = strdup(value);
	} else if (strcmp(key, "listen_port") == 0) {
		__config->listen_port = atoi(value);
	} else if (strcmp(key, "timeout_read") == 0) {
		__config->timeout_read = atoi(value);
	} else if (strcmp(key, "timeout_write") == 0) {
		__config->timeout_write = atoi(value);
	} else if (strcmp(key, "max_connections") == 0) {
		__config->max_connections = atoi(value);
	} else if (strcmp(key, "log_level") == 0) {
		__config->log_level = atoi(value);
	} else {
		fprintf(stderr, "unknown key: `%s' with value: `%s'\n", key, value);

		return -1;
	}

	fprintf(stderr, "config: parameter `%s' set to `%s'\n", key, value);

	return 0;
}

#define MAX_CONFIG_LINE_LEN 256
int parse_config_line(FILE *f)
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
			     strndupa(value_beg, value_end - value_beg));
}

int config_initialize(const char *path)
{
	assert(__config == NULL);

	if (path == NULL) {
		fprintf(stderr, "using default config path: `%s'\n", DEFAULT_CONFIG_PATH);
		path = DEFAULT_CONFIG_PATH;
	}

	FILE *f __attribute__((cleanup(close_file))) = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "can't open file: `%s': %s", path, strerror(errno));

		return -1;
	}

	__config = (struct config *)calloc(1, sizeof(struct config));
	while (feof(f) == false && ferror(f) == false) {
		if (parse_config_line(f) != 0) {
			config_deinitialize();

			return -1;
		}
	}

	return 0;
}

void config_deinitialize()
{
	assert(__config != NULL);

	free(__config->user);
	free(__config->pid_file);

	free(__config);
}

const struct config *config_get_config()
{
	return __config;
}
