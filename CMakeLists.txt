cmake_minimum_required(VERSION 3.22)
project(lab4 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab4 main.c buffer.c buffer.h producer.c consumer.c producer.h consumer.h)
