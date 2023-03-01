cmake_minimum_required(VERSION 3.22)
project(task_manager C)

set(CMAKE_C_STANDARD 11)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
add_executable(task_manager main.c structures/structures.h graphics/graphics.h processes/processes.h main.c utils/utils.h)
target_link_libraries(task_manager ${CURSES_LIBRARIES})