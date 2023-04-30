cmake_minimum_required(VERSION 3.22)
project(genfile C)

set(CMAKE_C_STANDARD 11)

add_executable(genfile genfile.c genfile.h)
