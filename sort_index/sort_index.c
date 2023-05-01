#include "sort_index.h"
#include "thread.h"
#include "utils.h"

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
    long _threads = strtol(argv[3], NULL, 10);
    if (_threads == 0) {
        fprintf(stderr, "[ERROR]: Threads amount is zero or not a number\n");
        return -2;
    }
    if (_threads < MIN_THREADS) {
        fprintf(stderr, "[ERROR]: Threads amount is less than amount of kernels\n");
        return -5;
    }
    if (_threads > MAX_THREADS) {
        fprintf(stderr, "[ERROR]: Threads amount is greater than max amount of threads\n");
        return -5;
    }
    if (_threads >= blocks) {
        fprintf(stderr, "[ERROR]: Threads amount is greater than blocks amount\n");
        return -6;
    }
    createThreads(_threads, memsize, blocks, argv[0], argv[4]);
    joinThreads((int) _threads);
    return 0;
}