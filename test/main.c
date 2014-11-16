#include "unit.h"

#include <stdlib.h>

int main(void)
{
	SRunner *sr;
	int number_failed = 0;

	sr = srunner_create(tbuf_suite());
	srunner_add_suite(sr, config_suite());
	srunner_add_suite(sr, optimen_suite());

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
