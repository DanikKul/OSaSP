CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE
.PHONY: clean
all: task_manager
task_manager: task_manager makefile
	$(CC) $(CFLAGS) main.c -o task_manager -lncurses
clean:
	rm task_manager