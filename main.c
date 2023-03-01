#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


struct map {
    char c;
    int bool;
} flags[4];

struct types {
    char c;
    int bool;
} allowed_types[4];

void allow_types () {
    char* _flags = "lfd";
    for (int i = 0; i < 3; i++) {
        allowed_types[i].c = _flags[i];
        allowed_types[i].bool = 1;
    }
    if (flags[0].bool == flags[1].bool && flags[1].bool == flags[2].bool) return;
    else {
        for (int i = 0; i < 3; i++) {
            allowed_types[i].bool = 0;
        }
        for (int i = 0; i < 3; i++) {
            allowed_types[i].bool = flags[i].bool;
        }
    }
}

void search(const char* path) {
    static int depth = 0;
    struct dirent** list;
    int n;
    if (!flags[3].bool) {
        n = scandir(path, &list, 0, NULL);
    } else {
        n = scandir(path, &list, 0, alphasort);
    }
    if (n == -1 && depth == 0) {
        perror(path);
        return;
    }
    int start = 0;
    while(start < n) {
        if (strcmp(list[start] -> d_name, ".") != 0 && strcmp(list[start] -> d_name, "..") != 0) {
            if (list[start] -> d_type == DT_DIR && allowed_types[2].bool) {
                printf("%s/%s\n", path, list[start] -> d_name);
            } else if (list[start] -> d_type == DT_REG && allowed_types[1].bool) {
                printf("%s/%s\n", path, list[start] -> d_name);
            } else if (list[start] -> d_type == DT_LNK && allowed_types[0].bool) {
                printf("%s/%s\n", path, list[start] -> d_name);
            }
            if (list[start] -> d_type == DT_DIR) {
                char* _path = (char*) malloc(1024 * sizeof(char));
                strcpy(_path, path);
                strcat(_path, "/");
                strcat(_path, list[start] -> d_name);
                depth++;
                search(_path);
            }
        }
        start++;
    }
}

int count (const char *string, char c) {
    int x = 0;
    for (size_t i = 0; i < strlen(string); i++) if (string[i] == c) x++;
    if (x == 0) x = -1;
    return x;
}

int main(int argc, char* argv[]) {
    char* path = (char*) malloc(1024 * sizeof(char));
    char* _flags = "lfds";
    for (int i = 0; i < 4; i++) flags[i].c = _flags[i];
    if (argc <= 1 || argv[1][0] == '-') {
        if (getcwd(path, 1024) == NULL) {
            perror("getcwd() error");
            return 1;
        }
        if (argv[1][0] == '-') {
            for (int i = 1; i < argc; i++) {
                if (argv[i][0] != '-') {
                    strcpy(path, argv[i]);
                    break;
                }
            }
        }
    }
    else {
        if (getcwd(path, 1024) == NULL) {
            perror("getcwd() error");
            return 1;
        }
        strcat(path, "/");
        strcpy(path, argv[1]);
    }
    printf("%s\n", path);
    const char* error = "Possible flags are:\n"
                        "[-l] - Only symbol links\n"
                        "[-f] - Only directories\n"
                        "[-d] - Only files\n"
                        "[-s] - Sort output\n"
                        "And their combinations";
    for (int i = 1; i < argc; i++) {
        char* flag = argv[i];
        if (flag[0] == '-') {
            for (size_t j = 1; j < strlen(flag); j++) {
                if (count(_flags, flag[j]) != 1) {
                    printf("Error: Incorrect flags\n");
                    printf("%s", error);
                    exit(1);
                }
            }
            for (size_t j = 0; j < strlen(_flags); j++){
                if (count(flag, _flags[j]) == 1) {
                    flags[j].bool = 1;
                } else if (count(flag, _flags[j]) > 1) {
                    printf("Error: Incorrect flag \"%s\"\n", flag);
                    printf("%s", error);
                    exit(1);
                }
            }
        }
    }
    allow_types();
    search(path);
    return 0;
}