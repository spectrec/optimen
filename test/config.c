#include "unit.h"
#include "config.h"

static int check_config_initialized(void)
{
	const struct config *config = config_get_config();
	ck_assert(config != NULL);

	ck_assert_str_eq(config->root_dir, "/var/optimen");
	ck_assert_str_eq(config->pid_file, "/var/run/optimen.pid");
	ck_assert_str_eq(config->user, "optimen");

	ck_assert_int_eq(config->timeout_write, 30);
	ck_assert_int_eq(config->timeout_read, 30);
	ck_assert_int_eq(config->max_connections, 100);
	ck_assert_int_eq(config->listen_port, 12345);
	ck_assert_int_eq(config->log_level, 4);

	return 0;
}

static int check_config_empty(void)
{
	const struct config *config = config_get_config();
	ck_assert(config != NULL);

	ck_assert(config->root_dir == NULL);
	ck_assert(config->pid_file == NULL);
	ck_assert(config->user == NULL);

	ck_assert_int_eq(config->timeout_write, 0);
	ck_assert_int_eq(config->timeout_read, 0);
	ck_assert_int_eq(config->max_connections, 0);
	ck_assert_int_eq(config->listen_port, 0);
	ck_assert_int_eq(config->log_level, 0);

	return 0;
}

static int check_config_with_dups(void)
{
	const struct config *config = config_get_config();
	ck_assert(config != NULL);

	ck_assert_str_eq(config->root_dir, "/var/optimen0");
	ck_assert_str_eq(config->pid_file, "/var/run/optimen2.pid");
	ck_assert_str_eq(config->user, "optimen3");

	ck_assert_int_eq(config->timeout_write, 30);
	ck_assert_int_eq(config->timeout_read, 300);
	ck_assert_int_eq(config->max_connections, 400);
	ck_assert_int_eq(config->listen_port, 143);
	ck_assert_int_eq(config->log_level, 4);

	return 0;
}

#define PATH_TO_CONFIG "test/etc/optimen.conf"
START_TEST(test_config_init)
{
	int ret = config_initialize(PATH_TO_CONFIG, false);

	ck_assert_int_eq(ret, 0);
	ck_assert(check_config_initialized() == 0);

	config_deinitialize();
}
END_TEST

#undef PATH_TO_CONFIG
#define PATH_TO_CONFIG "test/etc/optimen.conf.dups"
START_TEST(test_config_with_duplicate_values)
{
	int ret = config_initialize(PATH_TO_CONFIG, false);

	ck_assert_int_eq(ret, 0);
	ck_assert(check_config_with_dups() == 0);

	config_deinitialize();
}
END_TEST

#undef PATH_TO_CONFIG
#define PATH_TO_CONFIG "test/etc/optimen.conf.empty.values"
START_TEST(test_config_with_empty_values)
{
	int ret = config_initialize(PATH_TO_CONFIG, false);

	ck_assert_int_eq(ret, -1);

	config_deinitialize();
}
END_TEST

#undef PATH_TO_CONFIG
#define PATH_TO_CONFIG "test/etc/optimen.conf.incorrect.values"
START_TEST(test_config_with_incorrect_values)
{
	int ret = config_initialize(PATH_TO_CONFIG, false);

	ck_assert_int_eq(ret, -1);

	config_deinitialize();
}
END_TEST

#undef PATH_TO_CONFIG
#define PATH_TO_CONFIG "test/etc/optimen.conf.noent"
START_TEST(test_config_noent)
{
	int ret = config_initialize(PATH_TO_CONFIG, false);

	ck_assert_int_eq(ret, -1);

	config_deinitialize();
}
END_TEST

#undef PATH_TO_CONFIG
#define PATH_TO_CONFIG "test/etc/optimen.conf.empty"
START_TEST(test_config_empty)
{
	int ret = config_initialize(PATH_TO_CONFIG, false);

	ck_assert_int_eq(ret, 0);
	ck_assert(check_config_empty() == 0);

	config_deinitialize();
}
END_TEST

#undef PATH_TO_CONFIG
#define PATH_TO_CONFIG "test/etc/optimen.conf.spaces.comments"
START_TEST(test_config_spaces_and_comments)
{
	int ret = config_initialize(PATH_TO_CONFIG, false);

	ck_assert_int_eq(ret, 0);
	ck_assert(check_config_initialized() == 0);

	config_deinitialize();
}
END_TEST

Suite *config_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Config");
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_config_init);
	tcase_add_test(tc_core, test_config_noent);
	tcase_add_test(tc_core, test_config_empty);
	tcase_add_test(tc_core, test_config_with_empty_values);
	tcase_add_test(tc_core, test_config_with_incorrect_values);
	tcase_add_test(tc_core, test_config_with_duplicate_values);
	tcase_add_test(tc_core, test_config_spaces_and_comments);

	suite_add_tcase(s, tc_core);

	return s;
}
