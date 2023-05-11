//
// Created by dan on 19.4.23.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "buffer.h"

extern buff_t* msgs;
extern pthread_t consumers[];
extern size_t consCount;

extern pthread_cond_t condWaitForFreeSpace;
extern pthread_cond_t condWaitForAnyItem;
extern pthread_mutex_t mutex;
extern pthread_mutex_t mutexWaitForFreeSpace;
extern pthread_mutex_t mutexWaitForAnyItem;

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

void cleaner_consumer(void* isCaptured) {
    int* capt = (int*) isCaptured;

    if (!capt[1] && capt[0]) {
        pthread_cond_signal(&condWaitForFreeSpace);
        pthread_mutex_unlock(&mutexWaitForAnyItem);
    }

    if (capt[1]) {
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&condWaitForFreeSpace);
        pthread_mutex_unlock(&mutexWaitForAnyItem);
    }
}

_Noreturn void* consume(void* param __attribute__((unused))) {
    fprintf(stdout, "\"Consumer_%zu: Started...\"\n", (size_t) pthread_self());
    srand(getpid());
    msg_t* msg = (msg_t*) malloc(sizeof(msg_t));

    for (;;) {
        int isCaptured[] = {0, 0};
        pthread_cleanup_push(cleaner_consumer, &isCaptured);

        pthread_mutex_lock(&mutexWaitForAnyItem);

        isCaptured[0] = 1;

        while (msgs -> size <= 0)
            pthread_cond_wait(&condWaitForAnyItem, &mutexWaitForAnyItem);

        pthread_mutex_lock(&mutex);

        isCaptured[1] = 1;

        pop(msgs, msg);

        pthread_mutex_unlock(&mutex);

        isCaptured[1] = 0;

        pthread_cond_signal(&condWaitForFreeSpace);
        pthread_mutex_unlock(&mutexWaitForAnyItem);

        isCaptured[0] = 0;

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
        pthread_cleanup_pop(0);
    }
}

void createConsumer() {
    if (consCount >= CONSUMERS) {
        fprintf(stderr, "Main_%d: \"Consumers\" limit exceeded (consCount >= 128)\n", getpid());
        return;
    }
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
    pthread_cancel(consumers[consCount]);
    pthread_join(consumers[consCount], NULL);
    fprintf(stdout, "Main_%d: Thread with ID: %zu is cancelled\n", getpid(), (size_t) consumers[consCount]);
}
