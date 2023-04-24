#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

#include "buffer.h"
#include "producer.h"
#include "consumer.h"

buff_t *msgs;
size_t BUFFER_SIZE = 5;

pthread_t producers[PRODUCERS];
pthread_t consumers[CONSUMERS];
size_t prodCount = 0;
size_t consCount = 0;

sem_t* waitForFreeSpace;
sem_t* waitForAnyItem;
sem_t* mutex;
sem_t* service;

void printStats() {
    sem_wait(mutex);
    fprintf(stdout, "Main_%d: Buffer stats: puts: %zu, pops: %zu, count: %zu, size: %zu\n",getpid(), msgs -> put_count, msgs -> pop_count, msgs -> size, msgs -> maxsize);
    int sem1, sem2, sem3;
    sem_getvalue(waitForFreeSpace, &sem1);
    sem_getvalue(waitForAnyItem, &sem2);
    sem_getvalue(mutex, &sem3);
    fprintf(stdout, "Main_%d: Semaphores state: \'WaitForFreeSpace\': %d, \'WaitForAnyItem\': %d, \'Mutex\': %d\n", getpid(), sem1, sem2, sem3);
    fprintf(stdout, "Main_%d: \"Producers\": count: %zu\n", getpid(), prodCount);
    for (size_t i = 0; i < prodCount; i++) {
        fprintf(stdout, "Main_%d: Running \"Producer_%zu\"\n", getpid(), (size_t) producers[i]);
    }
    fprintf(stdout, "Main_%d: \"Consumers\": count: %zu\n", getpid(), consCount);
    for (size_t i = 0; i < consCount; i++) {
        fprintf(stdout, "Main_%d: Running \"Consumer_%zu\"\n", getpid(), (size_t) consumers[i]);
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
        sem_wait(mutex);
        pthread_cancel(producers[prodCount]);
        sem_post(mutex);
        pthread_join(producers[prodCount], NULL);
        fprintf(stdout, "Main_%d: \"Producer_%zu\" is cancelled\n", getpid(), (size_t) producers[prodCount]);
    }
    if (consCount == 0) {
        fprintf(stdout, "Main_%d: No running \"Consumers\" found\n", getpid());
    }
    while (consCount > 0) {
        consCount--;
        sem_wait(mutex);
        pthread_cancel(consumers[consCount]);
        sem_post(mutex);
        pthread_join(consumers[consCount], NULL);
        fprintf(stdout, "Main_%d: \"Consumer_%zu\" is cancelled\n", getpid(), (size_t) consumers[consCount]);
    }
    sem_destroy(waitForFreeSpace);
    sem_destroy(waitForAnyItem);
    sem_destroy(mutex);
    fprintf(stdout, "Main_%d: Deleting completed\n", getpid());
}

int main() {

    char* p = mmap(NULL, 4 * sizeof(sem_t) + sizeof(buff_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "Main_%d: Can't map bytes", getpid());
        exit(1);
    }

    waitForFreeSpace = (sem_t*) (p + 0);
    waitForAnyItem = (sem_t*) (p + 1 * sizeof(sem_t));
    mutex = (sem_t*) (p + 2 * sizeof(sem_t));
    service = (sem_t*) (p + 3 * sizeof(sem_t));
    msgs = (buff_t*) (p + 4 * sizeof(sem_t));

    init(msgs, BUFFER_SIZE);

    if (sem_init(service, 1, 1) + sem_init(waitForFreeSpace, 1, BUFFER_SIZE) + sem_init(waitForAnyItem, 1, 0) + sem_init(mutex, 1, 1) != 0) {
        fprintf(stderr, "Main_%d: Program failed at initializing semaphores\n", getpid());
        exit(1);
    }

    char choice;
    fprintf(stdout, "Menu:\n[p] - Create producer\n[c] - Create consumer\n[w] - Remove producer\n[e] - Remove consumer\n[q] - Quit\n\n");
    while (1) {
        fscanf(stdin, "%c", &choice);
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
        } else if (choice == '+') {
            sem_wait(mutex);
            increaseBuffer(msgs);
            sem_post(mutex);
        } else if (choice == '-') {
            sem_wait(mutex);
            decreaseBuffer(msgs);
            sem_post(mutex);
        } else if (choice == 'q') {
            removeOnExit();
            fprintf(stdout, "Quitting...\n");
            break;
        }
        fflush(stdin);
        rewind(stdin);
    }
    return 0;
}