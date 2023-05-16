//
// Created by Dan Kulakovich on 16.05.23.
//

#include "utilities.h"

// Quotes validation
// Valid examples: "asd", 'asd', ""asd"", ''asd''
int validateQuotes(char* string) {

    int double_quotes = 0;
    int unary_quotes = 0;

    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == '\"') double_quotes++;
        if (string[i] == '\'') unary_quotes++;
    }

    if (double_quotes == 0 && unary_quotes == 0) return 1;

    if (double_quotes % 2 == 0 && unary_quotes % 2 == 0 &&
        ((string[0] == '\'' && string[strlen(string) - 1] == '\'') ||
         (string[0] == '\"' && string[strlen(string) - 1] == '\"'))) {
        return 1;
    }

    return 0;
}

// Removing all quotes from string
// Removing examples: "asd" -> asd, 'asd' -> asd
char* removeQuotes(char* string) {

    char* output = (char*) malloc(sizeof(char) * strlen(string));
    int idx = 0;

    for (int i = 0; i < strlen(string); i++) {
        if (string[i] != '\'' && string[i] != '\"') {
            output[idx] = string[i];
            idx++;
        }
    }

    return output;
}

// That function return amount of directories in current path
int directoriesAmount(const char* path) {
    int amount = 0;
    DIR* dir = opendir(path);
    if (!dir) {
        return 0;
    }
    struct dirent* cur_dir;
    while(1) {
        cur_dir = readdir(dir);
        if (cur_dir == NULL){
            break;
        }
        amount++;
    }
    return amount;
}
