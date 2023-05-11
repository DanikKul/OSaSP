CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE
.PHONY: clean
all: build
build:
	$(CC) $(CFLAGS) main.c consumer.c producer.c buffer.c -o msg
clean:
	rm msg