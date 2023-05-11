//
// Created by dan on 19.4.23.
//
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

#include "buffer.h"

void init(buff_t* buff, size_t maxsize) {
    buff -> maxsize = maxsize;
}

short hash(const msg_t* msg) {
    short hash = 123;

    for(size_t i = 0; i < sizeof(msg_t); i++) {
        hash = ((hash << 5) + hash) + ((short*) msg)[i];
    }

    return hash;
}

int put(buff_t* buff, msg_t* msg) {
    buff -> messages[buff -> tail] = *msg;
    ++buff -> tail;
    if(buff -> size < buff -> maxsize) {
        ++buff -> size;
    } else {
        if (buff -> head < buff -> maxsize - 1) {
            ++buff -> head;
        } else {
            buff -> head = 0;
        }
    }
    ++buff -> put_count;

    if(buff -> tail >= buff -> maxsize) {
        buff -> tail = 0;
    }

    return 0;
}

int pop(buff_t* buff, msg_t* msg) {
    if(buff -> size <= 0) {
        return -1;
    }

    *msg = buff -> messages[buff -> head];
    ++buff -> head;
    --buff -> size;
    ++buff -> pop_count;

    if(buff -> head >= buff -> maxsize) {
        buff -> head = 0;
    }

    return 0;
}

void increaseBuffer(buff_t* buff) {
    ++buff -> maxsize;
}

void decreaseBuffer(buff_t* buff) {

    if (buff -> size == buff -> maxsize) {
        fprintf(stderr, "Main_%d: Can't decrease buffer: Buffer is full\n", getpid());
        return;
    }

    if (buff -> maxsize == 1) {
        fprintf(stderr, "Main_%d: Can't decrease buffer size: Size can't be less than 1\n", getpid());
        return;
    }

    if (buff -> maxsize <= 0) {
        return;
    } else {
        --buff -> maxsize;
    }
}