//
// Created by dan on 19.4.23.
//
#include <sys/wait.h>

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#include "buffer.h"

extern buff_t* msgs;
extern pid_t producers[];
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

void createProducer() {
    if (prodCount >= PRODUCERS) {
        fprintf(stderr, "Main_%d: Producers limit exceeded (prodCount >= 128)\n", getpid());
        return;
    }
    producers[prodCount] = fork();
    if (producers[prodCount] == -1) {
        fprintf(stderr, "Main_%d: Can't create \"Producer\" (ERROR: fork())\n", getpid());
        return;
    }
    if (producers[prodCount] != 0) {
        fprintf(stdout, "Main_%d: Created \"Producer\" with PID: %d\n", getpid(), producers[prodCount]);
        prodCount++;
        return;
    }
    if (producers[prodCount] == 0) {
        fprintf(stdout, "\"Producer_%d: Started...\"\n", getpid());
        srand(getpid());
        msg_t* msg = (msg_t*) malloc(sizeof(msg_t));

        for (;;) {
            createMessage(msg);
            sem_wait(waitForFreeSpace);
            sem_wait(mutex);

            put(msgs, msg);

            sem_post(mutex);
            sem_post(waitForAnyItem);

            fprintf(stdout, "Producer_%d: Putted in queue msg with:\nType: %d\nSize: %d\n\n", getpid(), msg -> type, msg -> size);

            sleep(5);
        }
    }
}

void removeProducer() {
    if (prodCount <= 0) {
        fprintf(stderr, "Main_%d: Can't remove Producer (prodCount <= 0)\n", getpid());
        return;
    }

    int status;
    prodCount--;
    fprintf(stdout, "Main_%d: Removing \"Producer\" with PID: %d\n", getpid(), producers[prodCount]);
    kill(producers[prodCount], SIGKILL);
    wait(&status);
    fprintf(stdout, "Main_%d: Process with PID: %d killed with exit status %d\n", getpid(), producers[prodCount], status);
}