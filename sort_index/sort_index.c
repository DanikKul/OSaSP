#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "sort_index.h"
#include "utils.h"

index_hdr_s* indices;

int openFile(char* path, char* filename) {

    // Generating path to file

    char* absolute_filename = (char*) malloc(250 * sizeof(char));
    for (size_t i = 0; i < strlen(path); i++) {
        absolute_filename[i] = path[i];
        if (i >= 5) {
            int k = 0;
            for (size_t j = i - 5; j < i; j++, k++) {
                if (PROJECT[k] != absolute_filename[j]) {
                    break;
                }
            }
            if (k == 5) {
                break;
            }
        }
    }
    strcat(absolute_filename, GENFILES_PATH);
    strcat(absolute_filename, filename);

    // Opening file

    int fd;
    if (!(fd = open(absolute_filename, O_RDWR))) {
        fprintf(stderr, "Can't open file\n");
        exit(-7);
    }
    char* ptr = mmap(0, sizeof(index_hdr_s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    indices = (index_hdr_s*) ptr;
    for (size_t i = 0; i < indices -> records; i++) {
        fprintf(stdout, "%llu %f\n", indices -> idx[i].recno, indices -> idx[i].time_mark);
    }
    fprintf(stdout, "\n\nSORT\n\n");
    qsort(indices -> idx, indices -> records, sizeof(index_s), compare);
    for (size_t i = 0; i < indices -> records; i++) {
        fprintf(stdout, "%llu %f\n", indices -> idx[i].recno, indices -> idx[i].time_mark);
    }
    if (msync(ptr, sizeof(index_hdr_s), MS_SYNC) < 0 ) {
        perror("msync failed with error:");
        return -1;
    } else fprintf(stdout, "Synced\n");
    munmap(ptr, sizeof(index_hdr_s));
    close(fd);
    return fd;
}

void printFile(int fd) {
    index_hdr_s header;
    read(fd, &header, sizeof(index_hdr_s));

    for (size_t i = 0; i < header.records; i++) {
        fprintf(stdout, "%llu %f\n", header.idx[i].recno, header.idx[i].time_mark);
    }
}

int main(int argc, char* argv[]) {

    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    limit.rlim_cur = limit.rlim_max;
    setrlimit(RLIMIT_STACK, &limit);

    // Check args

    if (argc < 4) {
        fprintf(stderr, "[ERROR]: Not enough arguments, please run program with memory size, blocks amount, threads amount and filename\n");
        return -1;
    }

    long memsize = strtol(argv[1], NULL, 10);
    if (memsize == 0) {
        fprintf(stderr, "[ERROR]: Memory size is zero or not a number\n");
        return -2;
    }
    if (memsize % getpagesize() != 0) {
        fprintf(stderr, "[ERROR]: Memory size is not multiple of %d\n", getpagesize());
        return -3;
    }
    long blocks = strtol(argv[2], NULL, 10);
    if (blocks == 0) {
        fprintf(stderr, "[ERROR]: Blocks amount is zero or not a number\n");
        return -2;
    }
    long tmp = blocks;
    while (tmp != 1) {
        if (tmp % 2 != 0) {
            fprintf(stderr, "[ERROR]: Blocks amount is not power of 2\n");
            return -4;
        }
        tmp /= 2;
    }
    long threads = strtol(argv[3], NULL, 10);
    if (threads == 0) {
        fprintf(stderr, "[ERROR]: Threads amount is zero or not a number\n");
        return -2;
    }
    if (threads < MIN_THREADS) {
        fprintf(stderr, "[ERROR]: Threads amount is less than amount of kernels\n");
        return -5;
    }
    if (threads > MAX_THREADS) {
        fprintf(stderr, "[ERROR]: Threads amount is greater than max amount of threads\n");
        return -5;
    }
    if (threads >= blocks) {
        fprintf(stderr, "[ERROR]: Threads amount is greater than blocks amount\n");
        return -6;
    }
    openFile(argv[0], argv[4]);
    return 0;
}