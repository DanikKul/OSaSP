//
// Created by Dan Kulakovich on 30.04.23.
//

#ifndef SORT_INDEX_UTILS_H
#define SORT_INDEX_UTILS_H

#include <stdint.h>
#include <stddef.h>

const char PROJECT[] = "OSaSP";
const char GENFILES_PATH[] = "genfile/generated_files/";

typedef struct {
    double time_mark;
    uint64_t recno;
} index_s;

typedef struct {
    uint64_t records;
    index_s idx[1000000];
} index_hdr_s;

int compare(const void *a, const void *b) {

    index_s *indexA = (index_s *)a;
    index_s *indexB = (index_s *)b;

    double cmp = indexB -> time_mark - indexA -> time_mark;

    return cmp > 0 ? 1 : cmp < 0 ? -1 : 0;
}

#endif //SORT_INDEX_UTILS_H
