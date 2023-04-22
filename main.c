#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#include "buffer.h"
#include "producer.h"
#include "consumer.h"

buff_t *msgs;

pid_t producers[PRODUCERS];
pid_t consumers[CONSUMERS];
size_t prodCount = 0;
size_t consCount = 0;

sem_t* waitForFreeSpace;
sem_t* waitForAnyItem;
sem_t* mutex;

void printStats() {
    sem_wait(mutex);
    fprintf(stdout, "Main_%d: Buffer stats: puts: %zu, pops: %zu, size: %zu\n",getpid(), msgs -> put_count, msgs -> pop_count, msgs -> size);
    fprintf(stdout, "Main_%d: \"Producers\": count: %zu\n", getpid(), prodCount);
    for (size_t i = 0; i < prodCount; i++) {
        fprintf(stdout, "Main_%d: Running \"Producer_%d\"\n", getpid(), producers[i]);
    }
    fprintf(stdout, "Main_%d: \"Consumers\": count: %zu\n", getpid(), consCount);
    for (size_t i = 0; i < consCount; i++) {
        fprintf(stdout, "Main_%d: Running \"Consumer_%d\"\n", getpid(), consumers[i]);
    }
    sem_post(mutex);
    fprintf(stdout, "\n");
}

void removeOnExit() {
    if (prodCount == 0) {
        fprintf(stdout, "Main_%d: No running \"Producers\" found\n", getpid());
    }
    while (prodCount > 0) {
        prodCount--;
        kill(producers[prodCount], SIGKILL);
        int status;
        wait(&status);
        fprintf(stdout, "Main_%d: \"Producer_%d\" killed with exit code %d\n", getpid(), producers[prodCount], status);
    }
    if (consCount == 0) {
        fprintf(stdout, "Main_%d: No running \"Consumers\" found\n", getpid());
    }
    while (consCount > 0) {
        consCount--;
        kill(consumers[consCount], SIGKILL);
        int status;
        wait(&status);
        fprintf(stdout, "Main_%d: \"Consumer_%d\" killed with exit code %d\n", getpid(), consumers[consCount], status);
    }
    sem_destroy(waitForFreeSpace);
    sem_destroy(waitForAnyItem);
    sem_destroy(mutex);
    fprintf(stdout, "Main_%d: Deleting completed\n", getpid());
}

int main() {

    char* p = mmap(NULL, 3 * sizeof(sem_t) + sizeof(buff_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "Main_%d: Can't map bytes", getpid());
        exit(1);
    }

    waitForFreeSpace = (sem_t*) (p + 0);
    waitForAnyItem = (sem_t*) (p + 1 * sizeof(sem_t));
    mutex = (sem_t*) (p + 2 * sizeof(sem_t));
    msgs = (buff_t*) (p + 3 * sizeof(sem_t));

    if (sem_init(waitForFreeSpace, 1, BUFFER_SIZE) + sem_init(waitForAnyItem, 1, 0) + sem_init(mutex, 1, 1) != 0) {
        fprintf(stderr, "Main_%d: Program failed at initializing semaphores\n", getpid());
        exit(1);
    }

    char choice;
    fprintf(stdout, "Menu:\n[p] - Create producer\n[c] - Create consumer\n[w] - Remove producer\n[e] - Remove consumer\n[q] - Quit\n\n");
    while (1) {
        choice = getc(stdin);
        if (choice == 'p') {
            createProducer();
        } else if (choice == 'c') {
            createConsumer();
        } else if (choice == 'w') {
            removeProducer();
        } else if (choice == 'e') {
            removeConsumer();
        } else if (choice == 's') {
            printStats();
        } else if (choice == 'q') {
            removeOnExit();
            fprintf(stdout, "Quitting...\n");
            break;
        } else {
            fprintf(stdout, "\n\nUnknown option... Try these:\n[p] - Create producer\n[c] - Create consumer\n[w] - Remove producer\n[e] - Remove consumer\n[q] - Quit\n\n");
        }
        fprintf(stdout, "Option: %c\n", choice);
        getchar();
    }
    return 0;
}