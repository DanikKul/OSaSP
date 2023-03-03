#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ;

int compareStrings(const char *s1, const char *s2) {
    return strcmp(s1, s2);
}

void sort(char* envp[], int len) {
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            if (compareStrings(envp[i], envp[j]) < 0) {
                char *buff = envp[i];
                envp[i] = envp[j];
                envp[j] = buff;
            }
        }
    }
}

// SHELL, HOME, //HOSTNAME\\, LOGNAME, LANG, TERM, USER, //LC_COLLATE\\, PATH

char** getMinimalEnvironment (const char *path) {
    char **environment = (char**) malloc (50 * sizeof (char*));
    char **minimalTokens = (char**) malloc(50 * sizeof(char*));
    for (int i = 0; i < 50; i++) {
        environment[i] = (char*) malloc (500 * sizeof(char*));
        minimalTokens[i] = (char*) malloc(500 * sizeof(char*));
    }
    FILE *f = NULL;
    if (!(f = fopen(path, "r"))) {
        return NULL;
    }
    int token_idx = 0;
    while (!feof(f)) {
        fgets(minimalTokens[token_idx], 100, f);
        if (minimalTokens[token_idx][strlen(minimalTokens[token_idx]) - 1] == '\n')
            minimalTokens[token_idx][strlen(minimalTokens[token_idx]) - 1] = '\0';
        token_idx++;
    }
    int len = 0;
    int idx = 0;
    while (environ[len] != NULL) len++;
    for (int k = 0; k < token_idx; k++) {
        for (int i = 0; i < len; i++) {
            if (strncmp(minimalTokens[k], environ[i], strlen(minimalTokens[k])) == 0) {
                strcpy(environment[idx], environ[i]);
                idx++;
                break;
            }
        }
    }
    fclose(f);
    return environment;
}

char **getEnvNames(const char *path, int *len) {
    FILE *f = NULL;
    if (!(f = fopen(path, "r"))) {
        return NULL;
    }
    char **names = (char**) malloc(50 * sizeof(char*));
    int token_idx = 0;
    while (!feof(f)) {
        names[token_idx] = (char*) malloc(500 * sizeof(char));
        fgets(names[token_idx], 100, f);
        if (names[token_idx][strlen(names[token_idx]) - 1] == '\n')
            names[token_idx][strlen(names[token_idx]) - 1] = '\0';
        token_idx++;
    }
    (*len) = token_idx;
    fclose(f);
    return names;
}

_Noreturn int start1(long number, char *pathToEnv, char** minEnv) {
    fprintf(stdout, "\nSTARTING NEW PROCESS WITH OPTION '+'\n\n");
    char *name = (char*) malloc (50 * sizeof(char));
    char *args[] = {name, pathToEnv, "+", (char*)0};
    int len;
    char **names = getEnvNames(pathToEnv, &len);
    printf("\nINNER ENVIRONMENT\n");
    for (int i = 0; i < len; i++) {
        fprintf(stdout, "%s=%s\n", names[i], getenv(names[i]));
    }
    printf("END\n");
    printf("Current PID: %d\n", getpid());
    sprintf(name, "child_%lu", number);
    fprintf(stdout, "Waiting 5 seconds until \"child\" will be executed\n");
    sleep(5);
    execve(getenv("CHILD_PATH"), args, minEnv);
    exit(123);
}

_Noreturn int start2(long number, char *envp[], int len, char *pathToEnv, char **minEnv) {
    fprintf(stdout, "\nSTARTING NEW PROCESS WITH OPTION '*'\n\n");
    char *name = (char*) malloc (500 * sizeof(char));
    char *path = (char*) malloc (50 * sizeof(char));
    char *args[] = {name, pathToEnv, "*", (char*)0};

    for (int i = 0; i < len; i++) {
        if (strncmp("CHILD_PATH", envp[i], strlen("CHILD_PATH")) == 0) {
            for (size_t j = 0; j < strlen(envp[i]) - 11; j++) {
                path[j] = envp[i][j + 11];
            }
            break;
        }
    }
    int lenx;
    char **names = getEnvNames(pathToEnv, &lenx);
    printf("\nINNER ENVIRONMENT\n");
    for (int i = 0; i < lenx; i++) {
        int flag = 0;
        for (int j = 0; j < lenx; j++) {
            if (strncmp(names[j], minEnv[i], strlen(names[j])) == 0) {
                fprintf(stdout, "%s=", names[j]);
                for (size_t k = 0; k < strlen(minEnv[i]); k++) {
                    if (flag) {
                        fprintf(stdout, "%c", minEnv[i][k]);
                    }
                    if (minEnv[i][k] == '=') {
                        flag = 1;
                    }
                }
                fprintf(stdout, "\n");
                break;
            }
        }
    }
    printf("END\n");
    sprintf(name, "child_%lu", number);
    printf("Current PID: %d\n", getpid());
    fprintf(stdout, "Waiting 5 seconds until \"child\" will be executed\n");
    sleep(5);
    execve(path, args, minEnv);
    exit(123);
}

_Noreturn int start3(long number, char *pathToEnv, char **minEnv) {
    fprintf(stdout, "\nSTARTING NEW PROCESS WITH OPTION '&'\n\n");
    char *name = (char*) malloc (500 * sizeof(char));
    char *path = (char*) malloc (50 * sizeof(char));
    char *args[] = {name, pathToEnv, "&", (char*)0};
    int len = 0;
    while (environ[len] != NULL) len++;
    for (int i = 0; i < len; i++) {
        if (strncmp("CHILD_PATH", environ[i], strlen("CHILD_PATH")) == 0) {
            for (size_t j = 0; j < strlen(environ[i]) - 11; j++) {
                path[j] = environ[i][j + 11];
            }
            break;
        }
    }
    int lenx;
    char **names = getEnvNames(pathToEnv, &lenx);
    printf("\nINNER ENVIRONMENT\n");
    for (int i = 0; i < lenx; i++) {
        int flag = 0;
        for (int j = 0; j < lenx; j++) {
            if (strncmp(names[j], minEnv[i], strlen(names[j])) == 0) {
                fprintf(stdout, "%s=", names[j]);
                for (size_t k = 0; k < strlen(minEnv[i]); k++) {
                    if (flag) {
                        fprintf(stdout, "%c", minEnv[i][k]);
                    }
                    if (minEnv[i][k] == '=') {
                        flag = 1;
                    }
                }
                fprintf(stdout, "\n");
                break;
            }
        }
    }
    printf("END\n");
    printf("Current PID: %d\n", getpid());
    sprintf(name, "child_%lu", number);
    fprintf(stdout, "Waiting 5 seconds until \"child\" will be executed\n");
    sleep(5);
    execve(path, args, minEnv);
    exit(123);
}

void printEnv(char *env[], int len) {
    printf("\nENVIRONMENT:\n");
    for (int i = 0; i < len; i++) printf("%s\n", env[i]);
    printf("END\n\n");
}

int main(int argc, char *argv[], char *env[]) {

    // Get and check environment variable "CHILD_PATH"

    char *path = getenv("CHILD_PATH");
    int status;
    long number = 0;
    if (!path || !fopen(path, "r") || argc < 2) return 1;

    // Get path to text file with variable's names from command line args and calculate it length

    char *pathToEnv = argv[1];
    int idx = 0;
    while (env[idx] != NULL) idx++;

    // Sort and print environment

    sort(env, idx);
    printEnv(env, idx);

    // Main loop for processing users input

    char ch;
    while (1) {
        ch = getc(stdin);
        if (ch == '+') {         //// Option where environment variables are taken with "getenv()"
            pid_t pid = fork();
            if (pid == -1) {
                fprintf(stdout, "Can't create new process\n");
            } else if (pid == 0) {
                fprintf(stdout, "New process created\n");
                start1(number, pathToEnv, getMinimalEnvironment(pathToEnv));
            }
            wait(&status);
            number++;
        } else if (ch == '*') {   //// Option where environment variables are taken from 3rd parameter of main
            pid_t pid = fork();
            if (pid == -1) {
                fprintf(stdout, "Can't create new process\n");
            } else if (pid == 0) {
                fprintf(stdout, "New process created\n");
                start2(number, env, idx, pathToEnv, getMinimalEnvironment(pathToEnv));
            }
            wait(&status);
            number++;
        } else if (ch == '&') {   //// Option where environment variables are taken from extern char **environ
            pid_t pid = fork();
            if (pid == -1) {
                fprintf(stdout, "Can't create new process\n");
            } else if (pid == 0) {
                fprintf(stdout, "New process created\n");
                start3(number, pathToEnv, getMinimalEnvironment(pathToEnv));
            }
            wait(&status);
            number++;
        } else if (ch == 'q'){
            printf("exit\n");
            break;
        }
        getchar();
    }
    return 0;
}