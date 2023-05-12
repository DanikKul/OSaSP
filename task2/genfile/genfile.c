//
// Created by dan on 12.5.23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

int openFile(char *path) {
    int fd;
    if ((fd = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU)) == -1) {
        fprintf(stderr, "[ERROR]: Can't create/generate file\n");
        exit(-1);
    }
    return fd;
}

void closeFile(int fd) {
    close(fd);
}

void generate(int fd, size_t size) {
    lseek(fd, 0, SEEK_SET);
    for (size_t i = 0; i < size; i++) {
        struct record_s* record = (struct record_s*) malloc(sizeof(struct record_s));
        size_t randomed = rand() % 1000;
        int j = 0;
        while (names[randomed][j]) {
            record -> name[j] = names[randomed][j];
            j++;
        }
        j = 0;
        while (addresses[randomed][j]) {
            record -> address[j] = addresses[randomed][j];
            j++;
        }
        record -> semester = (uint8_t) randomed % 8 + 1;
        write(fd, record, sizeof(struct record_s));
    }
}

void readFile(int fd, size_t size) {
    lseek(fd, 0, SEEK_SET);
    for (size_t i = 0; i < size; i++) {
        struct record_s* record = (struct record_s*) malloc(sizeof(struct record_s));
        read(fd, record, sizeof(struct record_s));
        fprintf(stdout, "NAME: %s\nADDRESS: %s\nSEMESTER: %u\n\n", record -> name, record -> address, record -> semester);
    }
}

int main(int argc, char *argv[]) {
    size_t size = 10;
    if (argc <= 1) {
        fprintf(stderr, "[ERROR]: No path to file provided\n");
        return -1;
    }
    char *path = (char *) malloc((strlen(argv[1]) + 1) * sizeof(char));
    if (argc > 2) {
        if (argv[2][0] == '-') {
            fprintf(stderr, "[ERROR]: Argument is negative\n");
            return -1;
        }
        size = strtol(argv[2], NULL, 10);
        if (size < 10) {
            fprintf(stderr, "[ERROR]: Can't parse argument or argument is less than 10\n");
            return -1;
        }
    }
    strcpy(path, argv[1]);
    srand(getpid());
    int fd = openFile(path);
    generate(fd, size);
    readFile(fd, size);
    fprintf(stdout, "Generated\n");
    closeFile(fd);
    return 0;
}
