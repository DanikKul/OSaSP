CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE

.PHONY: clean

all: build

build:
	$(CC) $(CFLAGS) competition.c -o competition

clean:
	rm competition