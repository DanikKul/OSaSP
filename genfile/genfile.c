#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils.h"

// Julian date generator

double getJulian() {
    time_t now;
    now = time(NULL);
    struct tm* local = localtime(&now);
    random();
    int year = local -> tm_year, month = local -> tm_mon,
    day = local -> tm_mday, hour = local -> tm_hour,
    min = local -> tm_min, sec = local -> tm_sec;
    year += 1900;
    month++;
    int a = (int) ((14 - month) / 12);
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    double JDN = day + (int) ((153 * m + 2) / 5) + 365 * y + (int) (y / 4) - (int) (y / 100) + (int) (y / 400) - 32045;
    return JDN + (double)(hour - 12) / 24 + (double) min / 1440 + (double) sec / 86400 - 2400000.5;
}

void generate(char* path, char* filename, size_t size) {
    FILE *file;

    // Generating path to file

    char* absolute_filename = (char*) malloc(250 * sizeof(char));
    for (size_t i = 0; i < strlen(path); i++) {
        absolute_filename[i] = path[i];
        if (i >= 7) {
            int k = 0;
            for (size_t j = i - 7; j < i; j++, k++) {
                if (PROJECT[k] != absolute_filename[j]) {
                    break;
                }
            }
            if (k == 7) {
                break;
            }
        }
    }
    strcat(absolute_filename, GENFILES_PATH);
    strcat(absolute_filename, filename);

    // Creating file

    if (!(file = fopen(absolute_filename, "w+"))) {
        fprintf(stderr, "Can't create/open file\n");
        exit(-4);
    }



    fclose(file);
    free(absolute_filename);
}

int main(int argc, char* argv[]) {
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
    return 0;
}