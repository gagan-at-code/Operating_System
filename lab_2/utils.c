/*
Author: Hoang Ho
Simple Shell Program

TODOs:

HELPER FUNCTIONS
*/

#include "shell.h"

int get_len_array(char **argsList) {
    int i = 0;
    while (argsList[i] != NULL) {
        i++;
    }
    return i;
}

int detect_builtins(char *command) {
    /*
    check if command is a builtin command
    argument: a string
    return : -1 if not builtin, and index of the builtin command if command is a builtin
    */

    int i = 0;
    while (builtins[i] != NULL) {
        if (strcmp(command, builtins[i]) == 0) {
            return i;
        }
        i++;
    }
    return -1;
}

int detect_pipe(char **argsList) {
    /*
    arguments: taking a list of arguments
    return: the index of | character in argsList if pipe is detected and 0 otherwise
    */

    int i = 0;
    while (strcmp(argsList[i], "|") != 0) {
        i++;
        if (argsList[i] == NULL) {
            return -1;
        }
    }
    return i;
}

int detect_left(char **argsList) {
    int i = 0;

    while (strcmp(argsList[i], "<") != 0) {
        i++;
        if (argsList[i] == NULL) {
            return -1;
        }
    }
    return i;
}

int detect_right(char **argsList) {
    int i = 0;

    while (strcmp(argsList[i], ">") != 0) {
        i++;
        if (argsList[i] == NULL) {
            return -1;
        }
    }
    return i;
}

int detect_parallel(char **argsList) {
    int n = get_len_array(argsList);
    for (int i = 0; i < n - 1; i++) {
        if (strcmp(argsList[i], "&") == 0) {
            return i;
        }
    }
    return -1;
}
