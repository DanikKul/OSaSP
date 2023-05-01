//
// Created by Dan Kulakovich on 30.04.23.
//

#ifndef SORT_INDEX_THREAD_H
#define SORT_INDEX_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "utils.h"

void createThreads(long amount, long memsize, long blocks, char* path, char* filename);
void joinThreads(int amount);

#endif //SORT_INDEX_THREAD_H
