#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include <sys/types.h>

#include "log.h"

#define DEFAULT_CONFIG_PATH "/usr/local/etc/optimen.conf"

struct config {
	char *user;
	char *pid_file;

	int16_t listen_port;

	int32_t timeout_read;
	int32_t timeout_write;

	uint32_t max_connections;

	enum log_level log_level;
};

int config_initialize(const char *path);
void config_deinitialize();

const struct config *config_get_config();

#endif
