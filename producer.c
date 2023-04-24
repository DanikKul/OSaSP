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
extern pthread_t producers[];
extern size_t prodCount;

extern sem_t* waitForFreeSpace;
extern sem_t* waitForAnyItem;
extern sem_t* mutex;

void createMessage(msg_t* msg) {
    int size = rand() % 257;
    short type = rand() % 0x8;
    for (int i = 0; i < size; i++) {
        msg -> data[i] = rand() % 256;
    }
    if (size == 256) {
        msg -> size = 0;
    } else {
        msg -> size = size;
    }
    msg -> type = type;
    msg -> hash = 0;
    msg -> hash = hash(msg);
}

_Noreturn void* produce(void* param __attribute__((unused))) {
    fprintf(stdout, "\"Producer_%zu: Started...\"\n", (size_t) pthread_self());
    srand(pthread_self());
    msg_t* msg = (msg_t*) malloc(sizeof(msg_t));
    for (;;) {
        createMessage(msg);
        sem_wait(waitForFreeSpace);
        sem_wait(mutex);

        put(msgs, msg);

        sem_post(mutex);
        sem_post(waitForAnyItem);

        fprintf(stdout, "Producer_%zu: Putted in queue msg with:\nType: %d\nSize: %d\n\n", (size_t) pthread_self(), msg -> type, msg -> size);

        sleep(5);
    }
}

void createProducer() {
    if (prodCount >= PRODUCERS) {
        fprintf(stderr, "Main_%d: Producers limit exceeded (prodCount >= 128)\n", getpid());
        return;
    }
    int err = pthread_create(&producers[prodCount], NULL, produce, NULL);
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
    prodCount++;
}

void removeProducer() {
    if (prodCount <= 0) {
        fprintf(stderr, "Main_%d: Can't remove \"Producer\" (prodCount <= 0)\n", getpid());
        return;
    }

    prodCount--;
    fprintf(stdout, "Main_%d: Removing \"Producer\" with ID: %zu\n", getpid(), (size_t) producers[prodCount]);
    sem_wait(mutex);
    pthread_cancel(producers[prodCount]);
    sem_post(mutex);
    pthread_join(producers[prodCount], NULL);
    fprintf(stdout, "Main_%d: Thread with ID: %zu is cancelled\n", getpid(), (size_t) producers[prodCount]);
}