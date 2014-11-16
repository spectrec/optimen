#include "unit.h"

/*START_TEST(test_name)
{
}
END_TEST*/

Suite *tbuf_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("tbuf");
	tc_core = tcase_create("all");

	//tcase_add_test(tc_core, test_name);

	suite_add_tcase(s, tc_core);

	return s;
}
