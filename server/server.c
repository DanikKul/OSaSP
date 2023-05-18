//
// Created by Dan Kulakovich on 16.05.23.
//

#include "handlers.h"

#define MAX_CLIENTS 3
#define BUFFER_SIZE 1024

pthread_t threads[MAX_CLIENTS];
int client_sockets[MAX_CLIENTS];

// This function redirects serving threads to a function according to client's request
void dispatch(char* request, args* arg) {
    fprintf(stdout, "THREAD_%zu: [INFO]: Dispatching request\n", (size_t) pthread_self());
    if (strncmp(request, "echo", 4) == 0) {
        handleEcho(request + 5, arg);
    } else if (strcmp(request, "exit") == 0) {
        handleExit(arg);
        client_sockets[arg -> thread_no] = 0;
        pthread_exit(NULL);
    } else if (strncmp(request, "delay", 5) == 0) {
        if (strncmp(request, "delay ", 6) == 0) {
            handleDelay(request + 6, arg);
        } else {
            send(arg->client_socket, "Please specify arguments, like that\ndelay 5",
                 strlen("Please specify arguments, like that\ndelay 5"), 0);
        }
    } else if (strcmp(request, "info") == 0) {
        handleInfo(arg);
    } else if (strcmp(request, "ls") == 0) {
        handleList(arg);
    } else if (strncmp(request, "cd", 2) == 0) {
        if (strncmp(request, "cd ", 3) == 0) {
            handleChangeDirectory(request + 3, arg);
        } else {
            send(arg->client_socket, "Please specify arguments, like that\ncd .",
                 strlen("Please specify arguments, like that\ncd ."), 0);
        }
    } else {
        fprintf(stdout, "THREAD_%zu: [INFO]: No such command found\n", (size_t) pthread_self());
        send(arg -> client_socket, "server: command not found\0", strlen("server: command not found\0"), 0);
    }
}

// Function that every thread executes while serving client
static void* serve(args* arg) {
    char* path = (char*) malloc(500 * sizeof(char));
    realpath(arg -> path, path);
    path = dirname(path, -1);
    send(arg -> client_socket, path, strlen(path), 0);
    char buffer[BUFFER_SIZE];
    fprintf(stdout, "THREAD_%zu: [INFO]: Start serving\n", (size_t) pthread_self());
    while (1) {
        // Read and print client message
        int bytes_read = (int) recv(arg -> client_socket, buffer, sizeof(buffer), 0);
        if (bytes_read == -1) {
            fprintf(stderr, "THREAD_%zu: [ERROR]: Can't read client data\n", (size_t) pthread_self());
            close(arg -> client_socket);
            client_sockets[arg -> thread_no] = 0;
            pthread_exit(NULL);
        } else if (bytes_read == 0) {
            fprintf(stdout, "THREAD_%zu: [INFO]: Client disconnected\n", (size_t) pthread_self());
            close(arg -> client_socket);
            client_sockets[arg -> thread_no] = 0;
            pthread_exit(NULL);
        }

        dispatch(buffer, arg);

        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));
    }
}

int main(int argc, char* argv[]) {
    long port = 8123;
    if (argc > 1) {
        port = strtol(argv[1], NULL, 10);
        if (port < 8000 || port > 8999) {
            fprintf(stderr, "MAIN_THREAD: [WARNING]: Specify a port as positive value (8000 <= port <= 8999)\n");
            fprintf(stderr, "MAIN_THREAD: [WARNING]: Setting default port (8123)\n");
            port = 8123;
        }
    }
    int server_fd, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    for (int i = 0; i < MAX_CLIENTS; i++) client_sockets[i] = 0;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        fprintf(stderr, "MAIN_THREAD: [ERROR]: Failed to create socket\n");
        exit(1);
    }

    // Set up server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "MAIN_THREAD: [ERROR]: Failed to bind socket\n");
        exit(1);
    }

    // Listen for new connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        fprintf(stderr, "MAIN_THREAD: [ERROR]: Failed to listen\n");
        exit(1);
    }

    fprintf(stdout, "MAIN_THREAD: [INFO]: Server listening on port %ld...\n", port);

    while (1) {

        // Accept new connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_address,
                                    &client_address_len)) < 0) {
            fprintf(stderr, "MAIN_THREAD: [ERROR]: Failed to accept connection\n");
            exit(1);
        } else {

            // Add client socket to the list
            int i;
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_socket;
                    break;
                }
            }
            if (i == MAX_CLIENTS) {
                send(client_socket, "error:full server", sizeof("error:full server"), 0);
                continue;
            }
            args *arg = (args *) malloc(sizeof(args));
            arg -> client_socket = client_socket;
            arg -> thread_no = i;
            arg -> address = "127.0.0.1";
            arg -> port = port;
            arg -> path = ".";
            pthread_create(&threads[i], NULL, (void *(*)(void *)) serve, arg);
        }
    }
}
