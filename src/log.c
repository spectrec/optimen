#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>

#include "log.h"
#include "config.h"

__attribute__((destructor))
static void close_log()
{
	closelog();
}

static int log_level2syslog(enum log_level level)
{
	switch (level) {
	case LOG_LEVEL_DEBUG:
		return LOG_DEBUG;
	case LOG_LEVEL_INFO:
		return LOG_INFO;
	case LOG_LEVEL_WARNING:
		return LOG_WARNING;
	case LOG_LEVEL_ERROR:
		return LOG_ERR;
	default:
		abort();
	}
}

void write_log(enum log_level level, const char *fmt, ...)
{
	const struct config *config = config_get_config();
	if (config->log_level < level)
		return;

	va_list ap;
	int syslog_level = log_level2syslog(level);

	va_start(ap, fmt);
	vsyslog(syslog_level, fmt, ap);
	va_end(ap);
}

void log_initialize(bool use_stderr, const char *program_name)
{
	int stderr_option = use_stderr == true ? LOG_PERROR : 0;
	openlog(program_name, LOG_PID | stderr_option, LOG_DAEMON);
}
