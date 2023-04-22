//
// Created by dan on 19.4.23.
//
#pragma once

#include <sys/types.h>

#define BUFFER_SIZE 128
#define PRODUCERS 128
#define CONSUMERS 128

typedef struct {
    char type;
    short hash;
    char size;
    char data[256];
} msg_t;

typedef struct {
    size_t put_count;           // счетчик помещенных сообщений
    size_t pop_count;           // счетчик извлеченных сообщений

    size_t head;                // указывает на индекс головы
    size_t tail;                // указывает на индекс хвоста
    size_t size;                // используется для упрощения проверки оставшегося размера
    msg_t messages[BUFFER_SIZE]; // буфер сообщений
} buff_t;

short hash(const msg_t* msg);

int put(buff_t* buff, msg_t* msg);
int pop(buff_t* buff, msg_t* msg);