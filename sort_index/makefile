CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE

.PHONY: clean

all: build

build:
	$(CC) $(CFLAGS) sort_index.c thread.c -o sort_index

clean:
	rm sort_index