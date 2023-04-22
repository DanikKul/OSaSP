CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -D_DEFAULT_SOURCE
.PHONY: clean
all: main
main: main makefile
	$(CC) $(CFLAGS) main.c consumer.c producer.c buffer.c -o main
clean:
	rm main