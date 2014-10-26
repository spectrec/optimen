#ifndef __OPTIMEN_H__
#define __OPTIMEN_H__

#include <sys/types.h>

enum optimen_ret {
	OPTIMEN_RET_OK = 0,
	OPTIMEN_RET_ERROR = 1,
};

struct optimen_ctx {
	char some_data;
};

void optimen_initialize();

#endif
