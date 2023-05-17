CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE
.PHONY: clean
all: build
build:
	$(CC) $(CFLAGS) server.c utilities.c handlers.c -o server
clean:
	rm server