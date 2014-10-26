#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/file.h>
#include <sys/types.h>

#include "log.h"
#include "libev.h"
#include "config.h"
#include "optimen.h"

static int __pid_file_fd = -1;

static void usage(const char *program)
{
	fprintf(stderr, "Usage: %s OPTIONS\n"
			"  -c [PAHT]      set path to config file (default: `%s')\n"
			"  -x             don't detach from terminal\n"
			"  -h             print this message and exit\n",
			program, DEFAULT_CONFIG_PATH);

	exit(0);
}

static int create_and_lock_pid_file(const char *path)
{
	__pid_file_fd = open(path, O_WRONLY | O_CREAT);
	if (__pid_file_fd == -1) {
		fprintf(stderr, "can't open file `%s': %s", path, strerror(errno));
		return -1;
	}

	char buf_pid[10] = {0};
	(void)snprintf(buf_pid, sizeof(buf_pid) - 1, "%d\n", getpid());
	if (write(__pid_file_fd, buf_pid, strlen(buf_pid)) < 0) {
		fprintf(stderr, "can't write to pid file: %s", strerror(errno));
		return -1;
	}

	if (flock(__pid_file_fd, LOCK_EX | LOCK_NB) != 0) {
		fprintf(stderr, "can't lock pid file: %s", strerror(errno));
		return -1;
	}

	return 0;
}

static int change_user_and_root(const char *user, const char *root)
{
	struct passwd *pw;

	errno = 0;
	if ((pw = getpwnam(user)) == NULL) {
		if (errno == 0)
			fprintf(stderr, "no such user: `%s'", user);
		else
			fprintf(stderr, "can't getpwnam: %s", strerror(errno));

		return -1;
	}

	if (chroot(root) != 0) {
		fprintf(stderr, "can't chroot to `%s': %s", root, strerror(errno));
		return -1;
	}

	gid_t egid = pw->pw_gid;
	if (setegid(egid) != 0) {
		fprintf(stderr, "can't change gid to `%d': %s", egid, strerror(errno));
		return -1;
	}

	uid_t euid = pw->pw_uid;
	if (seteuid(euid) != 0) {
		fprintf(stderr, "can't change uid to `%d': %s", euid, strerror(errno));
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	const struct config *config;
	const char *path_to_config = NULL;
	bool dont_daemonize = false;
	bool write_log_to_stderr = false;

	int opt;
	while ((opt = getopt(argc, argv, "c:xh")) != -1) {
		switch (opt) {
		case 'c':
			path_to_config = strdup(optarg);
			break;
		case 'x':
			dont_daemonize = true;
			write_log_to_stderr = true;
			break;

		case 'h':
		default:
			usage(basename(argv[0]));
		}
	}

	if (config_initialize(path_to_config) != 0)
		return -1;

	if (dont_daemonize != true)
		if (daemon(0, 1) != 0) {
			fprintf(stderr, "daemonize error: %s", strerror(errno));
			return -1;
		}

	config = config_get_config();
	if (create_and_lock_pid_file(config->pid_file) != 0)
		return -1;

	if (change_user_and_root(config->user, config->root_dir) != 0)
		return -1;

	log_initialize(write_log_to_stderr, basename(argv[0]));

	if (libev_initialize() != 0)
		return -1;

	optimen_initialize();

	libev_loop();

	return close(__pid_file_fd);
}
