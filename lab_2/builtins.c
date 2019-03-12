/*
Author: Hoang Ho
Simple Unix Shell Program
Builtin functions

TODOs:

1. TAKE CARE OF HELP

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
    if (changedPath) {
        /* if we have changed PATH, free the calloc */
        free(PATH);
        changedPath = 0;
    }
    if (args[1] == NULL) {
        PATH = "";
        if (putenv("PATH=\"\"")) {
            fprintf(stderr, "failed setting path %s", strerror(errno));
        }
    } else {
        int totalLen = 6;
        int i = 1;
        while(args[i] != NULL) {
            totalLen += strlen(args[i]);
            i++;
        }

        char *p;
        p = PATH = calloc(totalLen, sizeof(char));
        memcpy(p, "PATH=", 6);
        p +=5;
        for (int j = 1; j < i; j++) {
            memcpy(p, args[j], strlen(args[j]));
            p += strlen(args[j]);
            if (j < i - 1) {
                memcpy(p, ":", 1);
                p += 1;
            }
        }
        *p = '\0';
        putenv(PATH);
        changedPath = 1;
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
