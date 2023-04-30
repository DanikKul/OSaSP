//
// Created by Dan Kulakovich on 30.04.23.
//

#ifndef GENFILE_GENFILE_H
#define GENFILE_GENFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>

double getJulian(int r);
void generate(char* path, char* filename, size_t size);

#endif //GENFILE_GENFILE_H
