#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
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

pthread_cond_t condWaitForFreeSpace = PTHREAD_COND_INITIALIZER;
pthread_cond_t condWaitForAnyItem = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexWaitForFreeSpace = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexWaitForAnyItem = PTHREAD_MUTEX_INITIALIZER;

void printStats() {
    fprintf(stdout, "Main_%d: Buffer stats: puts: %zu, pops: %zu, count: %zu, size: %zu\n", getpid(), msgs -> put_count, msgs -> pop_count, msgs -> size, msgs -> maxsize);
    fprintf(stdout, "Main_%d: \"Producers\": count: %zu\n", getpid(), prodCount);
    for (size_t i = 0; i < prodCount; i++) {
        fprintf(stdout, "Main_%d: Running \"Producer_%zu\"\n", getpid(), (size_t) producers[i]);
    }
    fprintf(stdout, "Main_%d: \"Consumers\": count: %zu\n", getpid(), consCount);
    for (size_t i = 0; i < consCount; i++) {
        fprintf(stdout, "Main_%d: Running \"Consumer_%zu\"\n", getpid(), (size_t) consumers[i]);
    }
    fprintf(stdout, "\n");
}

void removeOnExit() {
    if (prodCount == 0) {
        fprintf(stdout, "Main_%d: No running \"Producers\" found\n", getpid());
    }
    pthread_mutex_lock(&mutex);
    while (prodCount > 0) {
        prodCount--;
        pthread_cancel(producers[prodCount]);
        pthread_join(producers[prodCount], NULL);
        fprintf(stdout, "Main_%d: \"Producer_%zu\" is cancelled\n", getpid(), (size_t) producers[prodCount]);
    }
    pthread_mutex_unlock(&mutex);
    if (consCount == 0) {
        fprintf(stdout, "Main_%d: No running \"Consumers\" found\n", getpid());
    }
    pthread_mutex_lock(&mutex);
    while (consCount > 0) {
        consCount--;
        pthread_cancel(consumers[consCount]);
        pthread_join(consumers[consCount], NULL);
        fprintf(stdout, "Main_%d: \"Consumer_%zu\" is cancelled\n", getpid(), (size_t) consumers[consCount]);
    }
    pthread_mutex_unlock(&mutex);
    pthread_cond_destroy(&condWaitForAnyItem);
    pthread_cond_destroy(&condWaitForFreeSpace);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexWaitForAnyItem);
    pthread_mutex_destroy(&mutexWaitForFreeSpace);
    fprintf(stdout, "Main_%d: Deleting completed\n", getpid());
}

int main() {

    msgs = (buff_t*) malloc(sizeof(buff_t));

    init(msgs, BUFFER_SIZE);

    char choice;
    fprintf(stdout, "Menu:\n[p] - Create producer\n[c] - Create consumer\n[w] - Remove producer\n[e] - Remove consumer\n[s] - Print stats\n[+] - Increase buffer\n[-] - Decrease buffer\n[q] - Quit\n\n");
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
            pthread_mutex_lock(&mutex);
            increaseBuffer(msgs);
            pthread_cond_signal(&condWaitForFreeSpace);
            pthread_mutex_unlock(&mutex);
        } else if (choice == '-') {
            pthread_mutex_lock(&mutex);
            decreaseBuffer(msgs);
            pthread_mutex_unlock(&mutex);
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