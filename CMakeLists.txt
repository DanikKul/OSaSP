cmake_minimum_required(VERSION 3.22)
project(lab5 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab5 main.c buffer.c buffer.h producer.c consumer.c producer.h consumer.h)
