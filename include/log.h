#ifndef __LOG_H__
#define __LOG_H__

#include <stdbool.h>

enum log_level {
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARNING = 2,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_DEBUG = 4,
};

void log_initialize(bool use_stdout, enum log_level log_level);
void write_log(int level, const char *fmt, ...);

#endif
