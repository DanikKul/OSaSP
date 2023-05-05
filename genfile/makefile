CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE

.PHONY: clean

all: build

build:
	$(CC) $(CFLAGS) genfile.c -o genfile

clean:
	rm genfile