#include <stdio.h>
#include "config.h"

int main(int argc, char *argv[])
{
	if (config_initialize("etc/optimen.conf") != 0)
		return -1;

	return 0;
}
