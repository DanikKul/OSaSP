//
// Created by dan on 19.4.23.
//
#include "buffer.h"

short hash(const msg_t* msg) {
    short hash = 123;

    for(size_t i = 0; i < sizeof(msg_t); i++) {
        hash = ((hash << 5) + hash) + ((short*) msg)[i];
    }

    return hash;
}

int put(buff_t* buff, msg_t* msg) {
    if(buff -> size == BUFFER_SIZE) {
        return -1;
    }

    buff -> messages[buff -> tail] = *msg;
    ++buff -> tail;
    ++buff -> size;
    ++buff -> put_count;

    if(buff -> tail == BUFFER_SIZE) {
        buff -> tail = 0;
    }

    return 0;
}

int pop(buff_t* buff, msg_t* msg) {
    if(buff -> size == 0) {
        return -1;
    }

    *msg = buff -> messages[buff -> head];
    ++buff -> head;
    --buff -> size;
    ++buff -> pop_count;

    if(buff -> head == BUFFER_SIZE) {
        buff -> head = 0;
    }

    return 0;
}
