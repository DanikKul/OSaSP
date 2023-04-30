CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE

.PHONY: clean

all: main

main: main makefile
	$(CC) $(CFLAGS) sort_index.c -o sort_index

clean:
	rm sort_index