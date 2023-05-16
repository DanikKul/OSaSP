//
// Created by Dan Kulakovich on 16.05.23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void runClient(char* addr, long port) {
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

    while (1) {

        // Send a message to the server
        char *message = (char*) malloc(300 * sizeof(char*));
        fprintf(stdout, "> ");
        fgets(message, 300, stdin);
        if (strlen(message) == 1 && message[0] == '\n') continue;
        message[strlen(message) - 1] = '\0';
        send(client_fd, message, strlen(message), 0);

        // Receive and print server response
        long bytes_received = recv(client_fd, buffer, BUFFER_SIZE * sizeof(char), 0);
        if (bytes_received == -1) {
            fprintf(stderr, "CLIENT: [ERROR]: Error when reading client data\n");
            close(client_fd);
            break;
        } else if (bytes_received == 0) {
            fprintf(stdout, "CLIENT: [INFO]: Server disconnected\n");
            close(client_fd);
            break;
        }
        fprintf(stdout, "%s\n", buffer);
        if (strncmp(buffer, "goodbye", strlen("goodbye")) == 0) {
            close(client_fd);
            break;
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
