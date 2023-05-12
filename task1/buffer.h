//
// Created by dan on 19.4.23.
//
#pragma once

#include <sys/types.h>

#define PRODUCERS 128
#define CONSUMERS 128

typedef struct {
    char type;
    short hash;
    unsigned char size;
    char data[256];
} msg_t;

typedef struct {
    size_t put_count;
    size_t pop_count;

    size_t head;
    size_t tail;
    size_t size;
    size_t maxsize;
    msg_t messages[65556];
} buff_t;

short hash(const msg_t* msg);
void init(buff_t* buff, size_t maxsize);
int put(buff_t* buff, msg_t* msg);
int pop(buff_t* buff, msg_t* msg);
void increaseBuffer(buff_t* buff);
void decreaseBuffer(buff_t* buff);