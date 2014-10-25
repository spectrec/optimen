cmake_minimum_required(VERSION 2.8)

project(optimend)

include_directories(include)
set(CMAKE_CFLAGS "${CMAKE_CFLAGS} -std=gnu99 -Wall -Wextra")

set(SRC_LIST src/main.c)
add_executable(${PROJECT_NAME} ${SRC_LIST})
