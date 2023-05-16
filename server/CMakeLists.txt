cmake_minimum_required(VERSION 3.22)
project(server C)

set(CMAKE_C_STANDARD 11)

add_executable(server server.c handlers.h handlers.c utilities.c)