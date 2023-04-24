//
// Created by dan on 19.4.23.
//
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "buffer.h"

extern buff_t* msgs;
extern pthread_t consumers[];
extern size_t consCount;

extern sem_t* waitForFreeSpace;
extern sem_t* waitForAnyItem;
extern sem_t* mutex;

int validateHash(msg_t* msg) {

    msg -> hash = 0;
    short curhash = hash(msg);
    msg -> hash = curhash;
    short oldhash = msg -> hash;

    if (oldhash != curhash) {
        fprintf(stderr, "Consumer_%zu: ERROR: Hashes doesn't match %d => %d. Fixing that...\n", (size_t) pthread_self(), oldhash, curhash);
        return -1;
    }

    msg -> hash = curhash;
    return 0;
}

_Noreturn void* consume(void* param __attribute__((unused))) {
    fprintf(stdout, "\"Consumer_%zu: Started...\"\n", (size_t) pthread_self());
    srand(getpid());
    msg_t* msg = (msg_t*) malloc(sizeof(msg_t));

    while (1) {
        sem_wait(waitForAnyItem);
        sem_wait(mutex);

        pop(msgs, msg);

        sem_post(mutex);
        sem_post(waitForFreeSpace);

        if (validateHash(msg) == -1) {
            fprintf(stderr, "Consumer_%zu: Can't print message: Reason: Hashes doesn't match\n", (size_t) pthread_self());
        } else {
            fprintf(stdout, "Consumer_%zu: data: ", (size_t) pthread_self());
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

void createConsumer() {
    if (consCount >= CONSUMERS) {
        fprintf(stderr, "Main_%d: \"Consumers\" limit exceeded (consCount >= 128)\n", getpid());
        return;
    }
    int value, value1;
    sem_getvalue(waitForFreeSpace, &value);
    sem_getvalue(waitForAnyItem, &value1);
    fprintf(stdout, "FreeSpace: %d AnyItem: %d\n", value, value1);
    int err = pthread_create(&consumers[consCount], NULL, consume, NULL);
    if (err == EAGAIN) {
        fprintf(stderr, "Main_%d: The system lacked the necessary resources to create a thread\n", getpid());
        return;
    } else if (err == EPERM) {
        fprintf(stderr, "Main_%d: The caller doesn't have appropriate permission to set the required scheduling parameters or policy\n", getpid());
        return;
    } else if (err == EINVAL) {
        fprintf(stderr, "Main_%d: The value specified by attr is invalid\n", getpid());
        return;
    }
    consCount++;
}

void removeConsumer() {
    if (consCount <= 0) {
        fprintf(stderr, "Main_%d: Can't remove \"Consumer\" (consCount <= 0)\n", getpid());
        return;
    }
    consCount--;
    fprintf(stdout, "Main_%d: Removing \"Consumer\" with ID: %zu\n", getpid(), (size_t) consumers[consCount]);
    sem_wait(mutex);
    pthread_cancel(consumers[consCount]);
    sem_post(mutex);
    pthread_join(consumers[consCount], NULL);
    fprintf(stdout, "Main_%d: Thread with ID: %zu is cancelled\n", getpid(), (size_t) consumers[consCount]);
}
