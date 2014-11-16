#include "unit.h"

#include <stdlib.h>

int main(void)
{
	int number_failed = 0;
	Suite *s;
	SRunner *sr;

	s = config_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
