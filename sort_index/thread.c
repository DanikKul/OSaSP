#include "thread.h"

extern pthread_t threads[];
extern size_t countThreads;

_Noreturn static void* execute(args* arguments) {
    fprintf(stdout, "%zu thread: %ld %ld\n", arguments -> no, arguments -> blocks, arguments -> memsize);
}

void createThreads(long amount, char* addr, long memsize, long blocks) {
    for (int i = 0; i < amount; i++) {
        args* arguments = (args*) malloc(sizeof(args));
        arguments -> addr = addr;
        arguments -> memsize = memsize;
        arguments -> blocks = blocks;
        arguments -> no = countThreads;
        int err = pthread_create(&threads[countThreads], NULL, (void *(*)(void *)) execute, arguments);
        if (err == EAGAIN) {
            fprintf(stderr, "The system lacked the necessary resources to create a thread\n");
            return;
        } else if (err == EPERM) {
            fprintf(stderr, "The caller doesn't have appropriate permission to set the required scheduling parameters or policy\n");
            return;
        } else if (err == EINVAL) {
            fprintf(stderr, "The value specified by attr is invalid\n");
            return;
        }
        countThreads++;
    }
}

void joinThreads(int amount) {
    for (int i = 0; i < amount; i++) {
        pthread_join(threads[i], NULL);
    }
}