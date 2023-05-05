CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE

.PHONY: clean

all: build

build:
	$(CC) $(CFLAGS) print_file.c -o print_file

clean:
	rm print_file