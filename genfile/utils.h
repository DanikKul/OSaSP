//
// Created by dan on 30.4.23.
//

#include <stdint.h>

#ifndef LAB6_UTILS_H
#define LAB6_UTILS_H

const char PROJECT[] = "genfile";
const char GENFILES_PATH[] = "generated_files/";

struct index_s {
    double time_mark;
    uint64_t recno;
} index_record;

struct index_hdr_s {
    uint64_t records;

};

#endif //LAB6_UTILS_H
