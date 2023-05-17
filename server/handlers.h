//
// Created by Dan Kulakovich on 16.05.23.
//

#ifndef SERVER_HANDLERS_H
#define SERVER_HANDLERS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "utilities.h"

typedef struct {
    int client_socket;
    char* address;
    long port;
    char* path;
    size_t thread_no;
} args;

void handleEcho(char* string, args* arg);
void handleDelay(char* request, args* arg);
void handleInfo(args* arg);
void handleList(args* arg);
void handleChangeDirectory(char* path, args* arg);
void handleExit(args* arg);

#endif //SERVER_HANDLERS_H
