//
// Created by dan on 19.4.23.
//
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/wait.h>

#include "buffer.h"

extern buff_t* msgs;
extern pid_t consumers[];
extern size_t consCount;

extern sem_t* waitForFreeSpace;
extern sem_t* waitForAnyItem;
extern sem_t* mutex;

int validateHash(msg_t* msg) {

    short oldhash = msg -> hash;
    msg -> hash = 0;
    short curhash = hash(msg);

    if (oldhash != curhash) {
        fprintf(stderr, "Consumer_%d: ERROR: Hashes doesn't match %d => %d\n", getpid(), oldhash, curhash);
        return -1;
    }

    msg -> hash = curhash;
    return 0;
}

void createConsumer() {
    if (consCount >= CONSUMERS) {
        fprintf(stderr, "Main_%d: \"Consumers\" limit exceeded (consCount >= 128)\n", getpid());
        return;
    }

    consumers[consCount] = fork();
    if (consumers[consCount] == -1) {
        fprintf(stderr, "Main_%d: Can't create \"Consumer\" (ERROR: fork())\n", getpid());
        return;
    }
    if (consumers[consCount] != 0) {
        fprintf(stdout, "Main_%d: Created \"Consumer\" with PID: %d\n", getpid(), consumers[consCount]);
        consCount++;
        return;
    }
    if (consumers[consCount] == 0) {
        fprintf(stdout, "\"Consumer_%d: Started...\"\n", getpid());
        srand(getpid());
        msg_t* msg = (msg_t*) malloc(sizeof(msg_t));

        for (;;) {
            sem_wait(waitForAnyItem);
            sem_wait(mutex);

            pop(msgs, msg);

            sem_post(mutex);
            sem_post(waitForFreeSpace);

            if (validateHash(msg) == -1) {
                fprintf(stderr, "Consumer_%d: Can't print message: Reason: Hashes doesn't match", getpid());
            } else {
                fprintf(stdout, "Consumer_%d: data: ", getpid());
                for (size_t i = 0; i < (size_t) msg -> size; i++) {
                    fprintf(stdout, "%hhx ", msg -> data[i]);
                }
                fprintf(stdout, "\nType: %hhx", msg -> type);
                fprintf(stdout, "\nSize: %d", msg -> size);
                fprintf(stdout, "\n");
            }

            sleep(5);
        }
    }
}

void removeConsumer() {
    if (consCount <= 0) {
        fprintf(stderr, "Main_%d: Can't remove \"Consumer\" (consCount <= 0)\n", getpid());
        return;
    }
    int status;
    consCount--;
    fprintf(stdout, "Main_%d: Removing \"Consumer\" with PID: %d\n", getpid(), consumers[consCount]);
    kill(consumers[consCount], SIGKILL);
    wait(&status);
    fprintf(stdout, "Main_%d: Process with PID: %d killed with exit status %d\n", getpid(), consumers[consCount], status);
}
