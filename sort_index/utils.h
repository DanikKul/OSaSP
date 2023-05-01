//
// Created by Dan Kulakovich on 30.04.23.
//

#ifndef SORT_INDEX_UTILS_H
#define SORT_INDEX_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

typedef struct {
    double time_mark;
    uint64_t recno;
} index_s;

typedef struct {
    uint64_t records;
    index_s idx[10000000];
} index_hdr_s;

typedef struct {
    char* addr;
    long memsize;
    long blocks;
    size_t no;
} args;

typedef struct {
    int isBusy;
    int block;
} blocks_map;

#endif //SORT_INDEX_UTILS_H
