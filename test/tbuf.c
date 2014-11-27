#include "unit.h"
#include "tbuf.h"
#include <string.h>

START_TEST(test_tbuf_append)
{
	struct tbuf t;
	tbuf_init(&t);
	ck_assert_int_eq(t.size, 0);

	tbuf_append(&t, "a", 1);
	ck_assert_int_eq(t.size, 1);
	ck_assert(t.data[0] == 'a');

	tbuf_delete(&t);
	ck_assert_int_eq(t.size, 0);
	ck_assert_int_eq(t.alloc_size, 0);

	tbuf_append(&t, "aaa", 3);
	ck_assert_int_eq(t.size, 3);
	ck_assert(strncmp(t.data, "aaa", t.size) == 0);

	tbuf_reset(&t);
	ck_assert_int_eq(t.size, 0);

	tbuf_delete(&t);
	ck_assert_int_eq(t.alloc_size, 0);
}
END_TEST

START_TEST(test_tbuf_shrink)
{
	struct tbuf t;
	tbuf_init(&t);
	ck_assert_int_eq(t.size, 0);

	tbuf_append(&t, "abcabcabc", 9);
	tbuf_shrink(&t, 7);
	ck_assert_int_eq(t.size, 2);
	ck_assert(strncmp(t.data, "bc", t.size) == 0);

	tbuf_shrink(&t, 2);
	ck_assert_int_eq(t.size, 0);

	tbuf_delete(&t);
	ck_assert_int_eq(t.alloc_size, 0);
}
END_TEST

START_TEST(test_tbuf_insert)
{
	struct tbuf t;
	tbuf_init(&t);
	ck_assert_int_eq(t.size, 0);

	tbuf_append(&t, "abc", 3);
	tbuf_insert(&t, "cb", 2);
	ck_assert_int_eq(t.size, 2);
	ck_assert(strncmp(t.data, "cb", t.size) == 0);

	tbuf_delete(&t);
	ck_assert_int_eq(t.alloc_size, 0);
}
END_TEST

START_TEST(test_tbuf_printf)
{
	struct tbuf t;
        tbuf_init(&t);
        ck_assert_int_eq(t.size, 0);

	tbuf_printf(&t, "%d%c%s", 10, 'a', "abc");
	ck_assert_int_eq(t.size, 6);
	ck_assert(strncmp(t.data, "10aabc", t.size) == 0);

	tbuf_delete(&t);
        ck_assert_int_eq(t.alloc_size, 0);
}
END_TEST

START_TEST(test_tbuf_append_vs_printf)
{
	struct tbuf t1, t2;
        tbuf_init(&t1);
	tbuf_init(&t2);

	tbuf_printf(&t1, "%d\r\n%.*s", 10, 3,  "a\0bcde");
	tbuf_printf(&t2, "%d\r\n", 10);
	tbuf_append(&t2, "a\0bcde", 3);

	ck_assert(memcmp(t1.data, "10\r\na\0b", 6) == 0);
	ck_assert(memcmp(t2.data, "10\r\na\0b", 6) == 0);

	tbuf_delete(&t1);
	tbuf_delete(&t2);
}
END_TEST


Suite *tbuf_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("tbuf");
	tc_core = tcase_create("all");

	tcase_add_test(tc_core, test_tbuf_append);
	tcase_add_test(tc_core, test_tbuf_shrink);
	tcase_add_test(tc_core, test_tbuf_insert);
	tcase_add_test(tc_core, test_tbuf_printf);
	tcase_add_test(tc_core, test_tbuf_append_vs_printf);

	suite_add_tcase(s, tc_core);

	return s;
}
