cmake_minimum_required(VERSION 3.22)
project(sort_index C)

set(CMAKE_C_STANDARD 11)

add_executable(sort_index sort_index.c sort_index.h utils.h thread.c thread.h)
