#ifndef __MOCK_H__
#define __MOCK_H__

#include <stdlib.h>

ssize_t send_to_optimen(const char *data, size_t size);
void recv_from_optimen(const char **data, size_t *size);

#endif
