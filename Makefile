CFLAGS += -std=gnu99 -Wall -Wextra
LDFLAGS += -lev -lm -pthread -lrt

DEBUG ?=
WERROR ?=

INCLUDE=include

ifeq (${DEBUG},1)
CFLAGS += -ggdb3
endif

ifeq (${WERROR},0)
CFLAGS +=
else
CFLAGS += -Werror
endif

CFLAGS += `getconf LFS_CFLAGS` -I${INCLUDE}

OPTIMEN_OBJS = src/config.o src/libev.o src/log.o src/main.o src/optimen.o src/tbuf.o
OPTIMEN_TEST_OBJS = src/config.o src/optimen.o src/tbuf.o test/config.o test/optimen.o test/tbuf.o test/main.o test/mock.o

optimen: ${OPTIMEN_OBJS}
	$(CC) -o $@ $^ ${LDFLAGS}

test: ${OPTIMEN_TEST_OBJS}
	$(CC) -o optimen.test $^ -lcheck $(LDFLAGS)
	@mkdir -p test/empty_directory_for_ls
	./optimen.test

clean:
	@find . -name '*.o' -delete
	@find . -name '*.gcno' -delete
	@rm -f optimen
	@rm -f optimen.test

install:
	install etc/optimen.conf /usr/local/etc/optimen.conf.dist
	install scripts/optimen-wrap -t /usr/local/bin/
	install scripts/optimen -t /etc/init.d/
	install optimen -t /usr/local/bin/

.PHONY: optimen test clean install
