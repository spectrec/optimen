#include "unit.h"

#include "mock.h"
#include "optimen.h"

#define STR_SIZE(_str) ((_str)), sizeof((_str))

__attribute__((constructor))
void initialize(void)
{
	optimen_initialize();
}

START_TEST(optimen_test_unknown_cmd)
{
	size_t size;
	const char *data;

	send_to_optimen(STR_SIZE("some command with args\r\n"));
	recv_from_optimen(&data, &size);

	ck_assert(strncmp("ERROR unknown command\r\n", data, size) == 0);
}
END_TEST

Suite *optimen_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("optimen");
	tc = tcase_create("api");

	tcase_add_test(tc, optimen_test_unknown_cmd);

	suite_add_tcase(s, tc);

	return s;
}
