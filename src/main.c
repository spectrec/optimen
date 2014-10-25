#include <stdio.h>
#include <libgen.h>

#include "log.h"
#include "config.h"

int main(int argc, char *argv[])
{
	if (config_initialize("etc/optimen.conf") != 0)
		return -1;

	log_initialize(true, basename(argv[0]));

	return 0;
}
