CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic
.PHONY: clean
all: parent child
parent: parent.c makefile
	$(CC) $(CFLAGS) parent.c -o parent
	export CHILD_PATH=/home/dan/CLionProjects/lab2/child
	export HOSTNAME=ubuntu-machine
	export LC_COLLATE=C
child: child.c makefile
	$(CC) $(CFLAGS) child.c -o child
clean:
	rm parent child