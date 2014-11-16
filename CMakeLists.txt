cmake_minimum_required(VERSION 2.8)

project(optimend)

include_directories(include)

execute_process(COMMAND getconf  LFS_FLAGS
	OUTPUT_VARIABLE LFS_FLAGS)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -ggdb3 -Wall -Wextra -Werror \
		   ${LFS_CFLAGS} -lev -lcheck -fprofile-arcs -ftest-coverage")

set(SRC_LIST src/main.c
	     src/config.c
	     src/log.c
	     src/optimen.c
	     src/libev.c
	     src/tbuf.c)

set(TEST_SRC_LIST test/config.c
		  test/tbuf.c
		  test/main.c
		  test/mock.c
		  test/optimen.c
		  src/optimen.c
		  src/config.c
		  src/tbuf.c)

add_executable(${PROJECT_NAME} ${SRC_LIST})

add_executable(${PROJECT_NAME}.test ${TEST_SRC_LIST})
