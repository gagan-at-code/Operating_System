/*
Author: Hoang Ho
Simple Unix Shell Program
Builtin functions

TODOs:

1. TAKE CARE OF PATH
2. TAKE CARE OF HELP

*/

#include "shell.h"

extern char **environ;

int cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, " missing directory argument\n");
    } else if (chdir(args[1]) == -1) {
        fprintf(stderr, " failed changing directory %s\n", strerror(errno));
    }
    return 1;
}

int clr(char **args) {
    system("clear");
    return 1;
}

int dir(char **args) {
    DIR *dirp;
    struct dirent *ep;
    if (args[1] == NULL) {
        fprintf(stderr, " missing directory argument\n");
    } else if ((dirp = opendir(args[1])) == NULL) {
        fprintf(stderr, " error openning directory %s %s\n", args[1], strerror(errno));
    } else {
        while ((ep = readdir(dirp)) != NULL) {
            printf("%s \n", ep->d_name);
        }
    }
    return 1;
}

int env(char **args) {
    int i;
    for (i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }
    return 1;
}

int echo(char **args) {
    int i = 1;
    while (args[i] != NULL) {
        printf("%s ", args[i++]);
    }
    puts("");
    return 1;
}

int path(char **args) {

    if (args[1] == NULL) {
        PATH = "";
    } else {
    }
    return 1;
}

int help(char **args) { return 1; }

int myPause(char **args) {
    printf("Press Enter to continue . . . ");
    while (getchar() != '\n');
    return 1;
}

int quit(char **args) { return 0; }
