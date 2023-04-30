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
#include "utils.h"

void createThreads(long amount, char* addr, long memsize, long blocks);
void joinThreads();

#endif //SORT_INDEX_THREAD_H
