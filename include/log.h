#ifndef __LOG_H__
#define __LOG_H__

#include <stdbool.h>

enum log_level {
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARNING = 2,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_DEBUG = 4,
};

void log_initialize(bool use_stderr, const char *progname);
void write_log(enum log_level level, const char *fmt, ...);

#define log(lev_, fmt_, ...) write_log(lev_, fmt_ " (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__)

#define log_e(fmt, ...) log(LOG_LEVEL_ERROR,   "E " fmt, ##__VA_ARGS__)
#define log_w(fmt, ...) log(LOG_LEVEL_WARNING, "W " fmt, ##__VA_ARGS__)
#define log_i(fmt, ...) log(LOG_LEVEL_INFO,    "I " fmt, ##__VA_ARGS__)
#define log_d(fmt, ...) log(LOG_LEVEL_DEBUG,   "D " fmt, ##__VA_ARGS__)

#endif
