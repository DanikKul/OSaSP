//
// Created by dan on 2.3.23.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char **environ;

void getEnv1(char *envVar[], int len) {
    for (int i = 0; i < len; i++) {
        if (getenv(envVar[i]))
            fprintf(stdout, "%s=%s\n", envVar[i], getenv(envVar[i]));
    }
}

void getEnv2(char *envp[], char *envVar[], int len) {
    while (*envp != NULL) {
        int flag = 0;
        for (int j = 0; j < len; j++) {
            if (strncmp(envVar[j], *envp, strlen(envVar[j])) == 0) {
                fprintf(stdout, "%s=", envVar[j]);
                for (size_t i = 0; i < strlen(*envp); i++) {
                    if (flag) {
                        fprintf(stdout, "%c", (*envp)[i]);
                    }
                    if ((*envp)[i] == '=') {
                        flag = 1;
                    }
                }
                fprintf(stdout, "\n");
                break;
            }
        }
        envp++;
    }
}

void getEnv3(char *envVar[], int len) {
    while (*environ != NULL) {
        int flag = 0;
        for (int j = 0; j < len; j++) {
            if (strncmp(envVar[j], *environ, strlen(envVar[j])) == 0) {
                fprintf(stdout, "%s=", envVar[j]);
                for (size_t i = 0; i < strlen(*environ); i++) {
                    if (flag) {
                        fprintf(stdout, "%c", (*environ)[i]);
                    }
                    if ((*environ)[i] == '=') {
                        flag = 1;
                    }
                }
                fprintf(stdout, "\n");
                break;
            }
        }
        environ++;
    }
}

int main(int argc, char *argv[], char *envp[]) {
    fprintf(stdout, "\nExecuting child process with name %s, PID %d and PPID %d...\n\n", argv[0], getpid(), getppid());
    if (argc < 2) {
        exit(999);
    }
    FILE *f = NULL;
    if (!(f = fopen(argv[1], "r"))) {
        fprintf(stderr, "Can't open file\n");
        exit(998);
    }
    char **environment = (char**) malloc (100 * sizeof(char*));
    for (size_t i = 0; i < 100; i++) {
        environment[i] = (char*) malloc (500 * sizeof(char));
    }
    int idx = 0;
    while (!feof(f)) {
        fgets(environment[idx], 100, f);
        if (environment[idx][strlen(environment[idx]) - 1] == '\n')
            environment[idx][strlen(environment[idx]) - 1] = '\0';
        idx++;
    }
    if (!strcmp(argv[2], "+")) {
        fprintf(stdout, "\nCHILD ENVIRONMENT\n");
        getEnv1(environment, idx);
        fprintf(stdout, "END\n\n");
    } else if (!strcmp(argv[2], "*")) {
        fprintf(stdout, "\nCHILD ENVIRONMENT\n");
        getEnv2(envp, environment, idx);
        fprintf(stdout, "END\n\n");
    } else if (!strcmp(argv[2], "&")) {
        fprintf(stdout, "\nCHILD ENVIRONMENT\n");
        getEnv3(environment, idx);
        fprintf(stdout, "END\n\n");
    } else {
        fprintf(stderr, "Something went wrong\n");
    }
    fprintf(stdout, "Waiting 10 seconds entil exit\n");
    sleep(10);
    fprintf(stdout, "Child process exiting with status status code 123...\n");
    exit(123);
}