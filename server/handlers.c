//
// Created by Dan Kulakovich on 16.05.23.
//

#define ROOT "CLionProjects"

#include "handlers.h"

// Handler of "echo" request
// It just sends symbols that were provided as argument in request
void handleEcho(char* request, args* arg) {
    fprintf(stdout, "THREAD_%zu: [INFO]: Handling request \"echo\"\n", (size_t) pthread_self());
    fprintf(stdout, "THREAD_%zu: [INFO]: String to echo is \"%s\"\n", (size_t) pthread_self(), request);
    char* string = (char*) malloc(500 * sizeof(char));
    memcpy(string, request, strlen(request));
    if (strlen(string) == 0) {
        fprintf(stdout, "THREAD_%zu: [INFO]: Empty string while executing \"echo\"\n", (size_t) pthread_self());
        send(arg -> client_socket, " ", strlen(" "), 0);
        return;
    }
    string[strlen(string)] = '\0';
    if (!validateQuotes(string)) {
        send(arg -> client_socket, "Invalid argument\0", strlen("Invalid argument\0"), 0);
    } else {
        string = removeQuotes(string);
        send(arg -> client_socket, string, strlen(string), 0);
    }
    free(string);
}

// Handler of "delay" request
// It stops thread's execution for n seconds which is specified in request arguments
void handleDelay(char* request, args* arg) {
    fprintf(stdout, "THREAD_%zu: [INFO]: Handling request \"delay\"\n", (size_t) pthread_self());
    if (strlen(request) == 0) {
        fprintf(stderr, "THREAD_%zu: [ERROR]: Empty value while executing \"delay\" or zero value\n", (size_t) pthread_self());
        send(arg -> client_socket, "Please enter a number more than zero, like that\ndelay 5",
             strlen("Please enter a number more than zero, like that\ndelay 5"), 0);
        return;
    }
    long secs = strtol(request, NULL, 10);
    if (secs == 0) {
        fprintf(stderr, "THREAD_%zu: [ERROR]: Can't parse value or it is equals zero\n", (size_t) pthread_self());
        send(arg -> client_socket, "Please enter a number more than zero, like that\ndelay 5",
             strlen("Please enter a number more than zero, like that\ndelay 5"), 0);
        return;
    } else if (secs < 0) {
        fprintf(stderr, "THREAD_%zu: [ERROR]: Value is less than zero\n", (size_t) pthread_self());
        send(arg -> client_socket, "Please enter a number more than zero, like that\ndelay 5",
             strlen("Please enter a number more than zero, like that\ndelay 5"), 0);
        return;
    }
    fprintf(stdout, "THREAD_%zu: [INFO]: Sleeping...\n", (size_t) pthread_self());
    sleep(secs);
    send(arg -> client_socket, " ", strlen(" "), 0);
    fprintf(stdout, "THREAD_%zu: [INFO]: Woke up\n", (size_t) pthread_self());
}

// Handler of "info" request
// Sends information about server
void handleInfo(args* arg) {
    fprintf(stdout, "THREAD_%zu: [INFO]: Handling request \"info\"\n", (size_t) pthread_self());
    fprintf(stdout, "THREAD_%zu: [INFO]: Sending information about server\n", (size_t) pthread_self());
    send(arg -> client_socket, "That server is just for fun :)\0", strlen("That server is just for fun :)\0"), 0);
}

// TODO: Допилить симлинки, а то фулл кринге
// Handler of "ls" request
// Sends a string containing all files in current directory of a server
void handleList(args* arg) {
    fprintf(stdout, "THREAD_%zu: [INFO]: Handling request \"ls\"\n", (size_t) pthread_self());
    fprintf(stdout, "THREAD_%zu: [INFO]: Listing files and directories\n", (size_t) pthread_self());
    char* output = (char*) malloc(500 * sizeof(char));
    DIR* dir = opendir(arg -> path);
    if (!dir) {
        fprintf(stderr, "THREAD_%zu: [ERROR]: Error at opening directory\n", (size_t) pthread_self());
        send(arg -> client_socket, "Internal server error\0", strlen("Internal server error\0"), 0);
        return;
    }
    struct dirent* cur_dir;
    while(1) {
        cur_dir = readdir(dir);
        if (cur_dir == NULL){
            break;
        }
        if (strcmp(cur_dir -> d_name, ".") == 0 || strcmp(cur_dir -> d_name, "..") == 0) continue;
        strcat(output, cur_dir -> d_name);
        if (cur_dir -> d_type == DT_DIR) {
            strcat(output, "/");
        }
        if (cur_dir -> d_type == DT_LNK) {
            char* tmp = (char*) malloc(500 * sizeof(char));
            strcat(tmp, arg -> path);
            if (tmp[strlen(tmp) - 1] != '/') {
                strcat(tmp, "/");
            }
            strcat(tmp, cur_dir -> d_name);
            (void) readlink(tmp, tmp, 500);
            struct stat* st = (struct stat*) malloc(sizeof(struct stat));\
            if (lstat(tmp, st) != 0) {
                fprintf(stdout, "THREAD_%zu: [WARNING]: lstat() error\n", (size_t) pthread_self());
            }
            if (S_ISLNK(st -> st_mode)) {
                strcat(output, " -->> ");
            } else {
                strcat(output, " --> ");
            }
            tmp = dirname(tmp, -2);
            strcat(output, tmp);
        }
        strcat(output, "\n");
    }
    output[strlen(output) - 1] = '\0';
    if (strlen(output) == 0) {
        send(arg -> client_socket, "server: folder is empty\0", strlen("server: folder is empty\0"), 0);
    } else {
        send(arg->client_socket, output, strlen(output), 0);
    }
}

// Handler of "cd" request
// Changes directory on a server
void handleChangeDirectory(char* request, args* arg) {
    fprintf(stdout, "THREAD_%zu: [INFO]: Handling request \"cd\"\n", (size_t) pthread_self());
    fprintf(stdout, "THREAD_%zu: [INFO]: Changing directory...\n", (size_t) pthread_self());
    request = strdup(request);
    char* go, *newpath = (char*) malloc(500 * sizeof(char));
    strcat(newpath, arg -> path);
    if (newpath[strlen(newpath) - 1] != '/')
        strcat(newpath, "/");
    while ((go = strsep(&request, "/")) != NULL) {
        struct dirent **namelist;
        int n;
        n = scandir(newpath, &namelist, 0, alphasort);
        if (n < 0) {
            fprintf(stdout, "THREAD_%zu: [WARNING]: Can't open directory\n", (size_t) pthread_self());
            char* path;
            path = realpath(arg -> path, NULL);
            path = dirname(path, -1);
            send(arg -> client_socket, path, 1024 * sizeof(char), 0);
            return;
        } else {
            int flag = 0;
            while(n--) {
                if (strcmp(namelist[n] -> d_name, ROOT) == 0) {
                    newpath[strlen(newpath) - 1] = '\0';
                    newpath[strlen(newpath) - 2] = '\0';
                }
                if (strcmp(namelist[n] -> d_name, go) == 0) {
                    if (newpath[strlen(newpath) - 1] != '/')
                        strcat(newpath, "/");
                    strcat(newpath, go);
                    flag = 1;
                }
                free(namelist[n]);
            }
            free(namelist);
            if (!flag) {
                fprintf(stdout, "THREAD_%zu: [WARNING]: Can't open directory\n", (size_t) pthread_self());
                char* path;
                path = realpath(arg -> path, NULL);
                path = dirname(path, -1);
                send(arg -> client_socket, path, 1024 * sizeof(char), 0);
                return;
            }
        }
    }

    struct dirent **namelist;
    int n;
    n = scandir(newpath, &namelist, 0, alphasort);
    if (n < 0) {
        fprintf(stderr, "THREAD_%zu: [ERROR]: Error at opening directory\n", (size_t) pthread_self());
        char* path;
        path = realpath(arg -> path, NULL);
        path = dirname(path, -1);
        send(arg -> client_socket, path, 1024 * sizeof(char), 0);
        return;
    } else {
        while(n--) {
            if (strcmp(namelist[n] -> d_name, ROOT) == 0) {
                newpath[strlen(newpath) - 1] = '\0';
                newpath[strlen(newpath) - 2] = '\0';
            }
            free(namelist[n]);
        }
        free(namelist);
    }
    arg -> path = newpath;
    char* path;
    path = realpath(arg -> path, NULL);
    path = dirname(path, -1);
    send(arg -> client_socket, path, 1024 * sizeof(char), 0);
}

// Handler of "exit" request
// Sends "goodbye" to the client and terminates session
void handleExit(args* arg) {
    fprintf(stdout, "THREAD_%zu: [INFO]: Handling request \"exit\"\n", (size_t) pthread_self());
    fprintf(stdout, "THREAD_%zu: [INFO]: Terminating session...\n", (size_t) pthread_self());
    send(arg -> client_socket, "goodbye\0", strlen("goodbye\0"), 0);
    close(arg -> client_socket);
    fprintf(stdout, "THREAD_%zu: [INFO]: Terminated\n", (size_t) pthread_self());
    pthread_exit(NULL);
}