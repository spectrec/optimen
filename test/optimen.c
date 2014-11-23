#include "unit.h"

#include "tbuf.h"
#include "mock.h"
#include "optimen.h"

#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>

#define STR_SIZE(_str) ((_str)), sizeof((_str)) - 1

__attribute__((constructor))
static void initialize(void)
{
	optimen_initialize();
}

static void close_dir(DIR **d)
{
	if (*d != NULL)
		closedir(*d);
}

static bool can_detect_file_type(void)
{
	int ret;
	struct dirent dent, *pdent = NULL;

	DIR *d __attribute__((cleanup(close_dir))) = opendir(".");
	if (d == NULL)
		return false;

	ret = readdir_r(d, &dent, &pdent);
	if (ret != 0 || pdent == NULL)
		return false;

	if (dent.d_type != DT_DIR)
		return false;

	return true;
}

static int get_expected_ls_resp(const char *path, struct tbuf *resp)
{
	DIR *d __attribute__((cleanup(close_dir))) = opendir(path);
	if (d == NULL)
		return -1;

	tbuf_insert(resp, STR_SIZE("OK list files:\r\n"));

	int ret = 0;
	struct dirent dent, *pdent = NULL;
	while ((ret = readdir_r(d, &dent, &pdent)) == 0) {
		if (pdent == NULL) // all readed
			break;

		char type;
		switch (dent.d_type) {
		case DT_REG:
			type = 'F';
			break;
		case DT_DIR:
			type = 'D';
			break;
		default:
			continue;
		}

		tbuf_printf(resp, "%c %s\r\n", type, dent.d_name);
	}

	if (ret != 0)
		return -1;

	tbuf_append(resp, STR_SIZE(".\r\n"));

	return 0;
}

START_TEST(optimen_test_unknown_cmd)
{
	size_t size;
	const char *data;

	(void)send_to_optimen(STR_SIZE("some command with args\r\n"));
	recv_from_optimen(&data, &size);

	ck_assert(strncmp("ERROR unknown command\r\n", data, size) == 0);
}
END_TEST

START_TEST(optimen_test_read_parser)
{
	size_t size;
	const char *data;
	ssize_t shrink_size;

	// buffer should contains error response after this command
	shrink_size = send_to_optimen(STR_SIZE("some command with args\r\n"));

	send_to_optimen(STR_SIZE("not fully command"));
	recv_from_optimen(&data, &size);

	// buffer contains data from previous test
	ck_assert(strncmp("ERROR unknown command\r\n", data, size) == 0);
	ck_assert(shrink_size == sizeof("some command with args\r\n") - 1);

	// check command with spaces at the begining
	shrink_size = send_to_optimen(STR_SIZE("    spaces at the begining\r\n"));
	recv_from_optimen(&data, &size);
	ck_assert(strncmp("ERROR unknown command\r\n", data, size) == 0);
	ck_assert(shrink_size == sizeof("    spaces at the begining\r\n") - 1);

	// check empty command
	shrink_size = send_to_optimen(STR_SIZE("\r\n"));
	recv_from_optimen(&data, &size);
	ck_assert(strncmp("ERROR unknown command\r\n", data, size) == 0);
	ck_assert(shrink_size == sizeof("\r\n") - 1);

	// check command with `\r' only
	shrink_size = send_to_optimen(STR_SIZE("some command \rdata"));
	recv_from_optimen(&data, &size);
	ck_assert(strncmp("ERROR unknown command\r\n", data, size) == 0);
	ck_assert(shrink_size == sizeof("some command \r") - 1);

	// check command with data after `\r\n'
	shrink_size = send_to_optimen(STR_SIZE("some command \r\n some data\n"));
	recv_from_optimen(&data, &size);
	ck_assert(strncmp("ERROR unknown command\r\n", data, size) == 0);
	ck_assert(shrink_size == sizeof("some command \r\n") - 1);
}
END_TEST

START_TEST(optimen_test_ls_command)
{
	size_t size;
	const char *data;
	struct tbuf t __attribute__((cleanup(tbuf_delete))) = (struct tbuf) {
		.data = NULL,
		.size = 0,
		.alloc_size = 0
	};

	// test `ls' without args
	(void)send_to_optimen(STR_SIZE("ls\r\n"));
	recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'ls' - required argument\r\n", data, size) == 0);

	// once more
	(void)send_to_optimen(STR_SIZE("ls     \r\n"));
	recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'ls' - required argument\r\n", data, size) == 0);

	// test `ls' with non existing folder
	(void)send_to_optimen(STR_SIZE("ls  non_existing_path   \r\n"));
	recv_from_optimen(&data, &size);
	ck_assert(strncmp("ERROR can't open specified directory\r\n", data, size) == 0);

	// test `ls' with multiple args
	(void)send_to_optimen(STR_SIZE("ls  multiple args   \r\n"));
	recv_from_optimen(&data, &size);
	ck_assert(strncmp("ERROR can't open specified directory\r\n", data, size) == 0);

	// test `ls' on file
	(void)send_to_optimen(STR_SIZE("ls  test/mock.c\r\n"));
	recv_from_optimen(&data, &size);
	ck_assert(strncmp("ERROR can't open specified directory\r\n", data, size) == 0);

	if (can_detect_file_type() == false) {
		fprintf(stderr, "can't detect file types, skip some tests\n");
		return;
	}
	// test `ls' with empty directory
	(void)send_to_optimen(STR_SIZE("ls  test/empty_directory_for_ls\r\n"));
	recv_from_optimen(&data, &size);
	if (get_expected_ls_resp("test/empty_directory_for_ls", &t) != 0)
        ck_abort_msg("can't get expected ls for `test/empty_directory_for_ls'");
	ck_assert(strncmp(t.data, data, size) == 0);

	// test `ls' with non-empty directory
	(void)send_to_optimen(STR_SIZE("ls test\r\n"));
	recv_from_optimen(&data, &size);
	if (get_expected_ls_resp("test", &t) != 0)
		ck_abort_msg("can't get expected ls for `test'");
	ck_assert(strncmp(t.data, data, size) == 0);
}
END_TEST

START_TEST(optimen_test_file_open_command)
{
    size_t size;
    const char *data;

    (void)send_to_optimen(STR_SIZE("file_open\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'file_open' - required argument\r\n", data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_open ../ololo.txt\r\n"));
    recv_from_optimen(&data, &size);    
    ck_assert(strncmp("ERROR 'file_open' - can't open file\r\n", data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_open test/etc/file_read_test.data\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK \r\n", data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_open test/etc/file_read_test.data\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'file_open' - file is open already\r\n", data, size) == 0);
}
END_TEST

START_TEST(optimen_test_file_close_command)
{
    size_t size;
    const char *data;

    (void)send_to_optimen(STR_SIZE("file_close\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'file_close' - file isn't open yet\r\n", data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_open test/etc/file_read_test.data\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK \r\n", data , size) == 0);

    (void)send_to_optimen(STR_SIZE("file_close\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK \r\n", data, size) == 0);
}
END_TEST

START_TEST(optimen_test_file_read_command)
{
    size_t size;
    const char *data;

    (void)send_to_optimen(STR_SIZE("file_open test/etc/file_read_test.data\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK \r\n", data , size) == 0);

    (void)send_to_optimen(STR_SIZE("file_read\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'file_read' - required offset argument\r\n", data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_read 0\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'file_read' - required size argument\r\n", data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_read s 1024\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'file_read' - can't convert offset\r\n", data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_read 0 s\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'file_read' - can't convert size\r\n", data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_read -10 1024\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("ERROR 'file_read' - can't fseek\r\n", data, size) == 0);

    // very long offset
    memset((void *)data, 0, size);
    (void)send_to_optimen(STR_SIZE("file_read 3000 16\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK 0\r\n", data, size) == 0);

    FILE *f = fopen("test/etc/file_read_test.data", "rb");
    ck_assert(f != NULL);

    void *f_data = NULL;
    f_data = realloc(f_data, 64);
    size_t f_bytes = fread(f_data, 1, 16, f);
    ck_assert(f_bytes != 0);

    (void)send_to_optimen(STR_SIZE("file_read 0 16\r\n"));
    recv_from_optimen(&data, &size);
    char buf[32] = {0};
    sprintf(buf, "OK 16\r\n%.*s", 16, (char *)f_data);
    ck_assert(strncmp(buf, data, size) == 0);

    // one more block
    (void)send_to_optimen(STR_SIZE("file_read 16 16\r\n"));
    recv_from_optimen(&data, &size);
    fseek(f, 16, SEEK_SET);
    f_bytes = fread(f_data, 1, 16, f);
    ck_assert(f_bytes != 0);
    memset(&buf, 0, sizeof(buf));
    sprintf(buf, "OK 16\r\n%.*s", 16, (char *)f_data);
    ck_assert(strncmp(buf, data, size) == 0);

    (void)send_to_optimen(STR_SIZE("file_close\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK \r\n", data, size) == 0);

    fclose(f);

    free(f_data);
}
END_TEST

START_TEST(optimen_test_file_read_empty_file_command)
{
    size_t size;
    const char *data;

    (void)send_to_optimen(STR_SIZE("file_open test/etc/optimen.conf.empty\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK \r\n", data , size) == 0);

    (void)send_to_optimen(STR_SIZE("file_read 0 16\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK 0\r\n", data , size) == 0);

    (void)send_to_optimen(STR_SIZE("file_close\r\n"));
    recv_from_optimen(&data, &size);
    ck_assert(strncmp("OK \r\n", data, size) == 0);
}
END_TEST

Suite *optimen_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("optimen");
	tc = tcase_create("api");

	tcase_add_test(tc, optimen_test_unknown_cmd);
	tcase_add_test(tc, optimen_test_read_parser);
	tcase_add_test(tc, optimen_test_ls_command);
    tcase_add_test(tc, optimen_test_file_open_command);
    tcase_add_test(tc, optimen_test_file_close_command);
    tcase_add_test(tc, optimen_test_file_read_command);
    tcase_add_test(tc, optimen_test_file_read_empty_file_command);

	suite_add_tcase(s, tc);

	return s;
}
