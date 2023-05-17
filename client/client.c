//
// Created by Dan Kulakovich on 16.05.23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int executeFile(const char* path, int client_fd, char* cur_path) {
    int fd;
    if ((fd = open(path, O_RDONLY, 0644)) == -1) {
        return -1;
    }
    char* buffer = (char*) malloc(200 * sizeof(char));
    read(fd, buffer, 20000);
    char commands[100][200];
    int idx = 0;
    int idx_c = 0;
    int counter = 0;
    while (buffer[idx] != '\0') {
        if (buffer[idx] == '\n') {
            counter++;
            idx_c = 0;
            idx++;
            continue;
        }
        commands[counter][idx_c] = buffer[idx];
        idx++, idx_c++;
    }

    char buffer_tmp[BUFFER_SIZE];

    for (int i = 0; i < counter + 1; i++) {

        // Send a message to the server
        fprintf(stdout, "%s> ", cur_path);
        if (strlen(commands[i]) == 1 && commands[i][0] == '\n') continue;
        fprintf(stdout, "%s\n", commands[i]);
        send(client_fd, commands[i], sizeof(commands[i]), 0);

        // Receive and print server response
        long b = recv(client_fd, buffer_tmp, BUFFER_SIZE * sizeof(char), 0);
        if (b == -1) {
            fprintf(stderr, "CLIENT: [ERROR]: Error when reading server data\n");
            close(client_fd);
            exit(1);
        } else if (b == 0) {
            fprintf(stdout, "CLIENT: [INFO]: Server disconnected\n");
            close(client_fd);
            exit(0);
        }
        if (strncmp(buffer_tmp, "goodbye", strlen("goodbye")) == 0) {
            fprintf(stdout, "%s\n", buffer_tmp);
            close(client_fd);
            exit(0);
        }
        if (strncmp("cd ", commands[i], 3) == 0) {
            memset(cur_path, 0, strlen(cur_path) * sizeof(char));
            strcpy(cur_path, buffer_tmp);
        } else {
            fprintf(stdout, "%s\n", buffer_tmp);
        }
        memset(buffer_tmp, 0, sizeof(buffer_tmp));
    }

    return fd;
}

void runClient(char* addr, long port) {
    char cur_path[BUFFER_SIZE];
    int client_fd;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        fprintf(stderr, "CLIENT: [ERROR]: Failed to create socket\n");
        exit(1);
    }

    // Set up server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, addr, &server_address.sin_addr) <= 0) {
        fprintf(stderr, "CLIENT: [ERROR]: Invalid address or address not supported\n");
        exit(1);
    }

    // Connect to server
    if (connect(client_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "CLIENT: [ERROR]: Failed to connect to server\n");
        exit(1);
    }

    // Setting path
    long bytes_received = recv(client_fd, cur_path, BUFFER_SIZE * sizeof(char), 0);
    if (bytes_received == -1) {
        fprintf(stderr, "CLIENT: [ERROR]: Error when reading server data\n");
        close(client_fd);
        return;
    } else if (bytes_received == 0) {
        fprintf(stdout, "CLIENT: [INFO]: Server disconnected\n");
        close(client_fd);
        return;
    }

    while (1) {

        // Send a message to the server
        char *message = (char*) malloc(300 * sizeof(char*));
        fprintf(stdout, "%s> ", cur_path);
        fgets(message, 300, stdin);
        if (strlen(message) == 1 && message[0] == '\n') continue;
        if (message[0] == '@') {
            message[strlen(message) - 1] = '\0';
            if (executeFile(message + sizeof(char), client_fd, cur_path) == -1) {
                fprintf(stderr, "CLIENT: [ERROR]: Can't execute file \"%s\"\n", message + sizeof(char));
            }
            continue;
        }
        message[strlen(message) - 1] = '\0';
        send(client_fd, message, strlen(message), 0);

        // Receive and print server response
        long b = recv(client_fd, buffer, BUFFER_SIZE * sizeof(char), 0);
        if (b == -1) {
            fprintf(stderr, "CLIENT: [ERROR]: Error when reading server data\n");
            close(client_fd);
            break;
        } else if (b == 0) {
            fprintf(stdout, "CLIENT: [INFO]: Server disconnected\n");
            close(client_fd);
            break;
        }
        if (strncmp(buffer, "goodbye", strlen("goodbye")) == 0) {
            fprintf(stdout, "%s\n", buffer);
            close(client_fd);
            break;
        }
        if (strncmp("cd ", message, 3) == 0) {
            memset(cur_path, 0, sizeof(cur_path));
            strcpy(cur_path, buffer);
        } else {
            fprintf(stdout, "%s\n", buffer);
        }
        memset(buffer, 0, sizeof(buffer));
    }
}

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        fprintf(stderr, "CLIENT: [ERROR]: Not specified address and port\n");
        exit(0);
    } else if (argc >= 4) {
        fprintf(stderr, "CLIENT: [ERROR]: Too many arguments specified\n");
        exit(0);
    }
    char* addr = argv[1];
    if (strcmp(addr, "localhost") == 0) {
        addr = "127.0.0.1";
    }
    struct sockaddr_in sa;
    if (!inet_pton(AF_INET, addr, &(sa.sin_addr))) {
        fprintf(stderr, "CLIENT: [ERROR]: Can't parse ip address\n");
    }
    long port = strtol(argv[2], NULL, 10);
    runClient(addr, port);
}
