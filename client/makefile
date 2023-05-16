CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE
.PHONY: clean
all: build
build:
	$(CC) $(CFLAGS) client.c -o client
clean:
	rm client