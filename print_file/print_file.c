#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>

const char PROJECT[] = "lab6";
const char GENFILES_PATH[] = "genfile/generated_files/";

typedef struct {
    double time_mark;
    uint64_t recno;
} index_s;

typedef struct {
    uint64_t records;
    index_s idx[10000000];
} index_hdr_s;

void generate(char* path, char* filename, size_t size) {
    int file;

    // Generating path to file

    char* absolute_filename = (char*) malloc(250 * sizeof(char));
    for (size_t i = 0; i < strlen(path); i++) {
        absolute_filename[i] = path[i];
        if (i >= strlen(PROJECT)) {
            int k = 0;
            for (size_t j = i - strlen(PROJECT); j < i; j++, k++) {
                if (PROJECT[k] != absolute_filename[j]) {
                    break;
                }
            }
            if (k == strlen(PROJECT)) {
                break;
            }
        }
    }
    strcat(absolute_filename, GENFILES_PATH);
    strcat(absolute_filename, filename);
    fprintf(stdout, "%s\n", absolute_filename);

    // Creating file

    if (!(file = open(absolute_filename, O_RDWR, S_IRWXO | S_IRWXU | S_IRWXG))) {
        fprintf(stderr, "Can't create/open file\n");
        exit(-4);
    }

    // Generating structures

    index_hdr_s header;

    // Reading from the file with specific sizeofs

    read(file, &header, sizeof(uint64_t) + size * sizeof(index_s));

    // Printing file to stdout

    fprintf(stdout, "RECORDS: %lu\n", header.records);
    for (size_t i = 0; i < header.records; i++) {
        fprintf(stdout, "RECNO: %lu\t\t\tTIME: %f\n", header.idx[i].recno, header.idx[i].time_mark);
    }

    // Closing file and freeing memory

    close(file);
    free(absolute_filename);
}

int main(int argc, char* argv[]) {

    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    limit.rlim_cur = limit.rlim_max;
    setrlimit(RLIMIT_STACK, &limit);

    // Check args

    if (argc < 2) {
        fprintf(stderr, "[ERROR]: Not enough arguments, please run program with file name and file size\n");
        return -1;
    }
    long filesize = strtol(argv[2], NULL, 10);
    if (filesize == 0) {
        fprintf(stderr, "[ERROR]: File size is zero or not a number\n");
        return -2;
    }
    if (filesize % 256 != 0) {
        fprintf(stderr, "[ERROR]: File size is not multiple of 256\n");
        return -3;
    }

    generate(argv[0], argv[1], filesize);
    fprintf(stdout, "Read successfully\n");
    return 0;
}
