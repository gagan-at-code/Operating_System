/*
Author: Hoang Ho
Simple Unix Shell Program
Main Shell Program

TODOs:
1. TAKE CARE OF PIPE (DONE)

2. TAKE CARE OF I/O REDIRECTION (DONE)

3. TAKE CARE OF BACKGROUND PROCESSES (DONE)

4. TAKE CARE OF PARALLEL COMMAND

5. TAKE CARE OF BATCH MODE
*/

#include "shell.h"

int (*builtin_commands[10])(char **args) = {&cd, &clr, &dir, &env, &echo, &path, &help, &myPause, &quit};
char *builtins[10] = {"cd", "clr", "dir", "environ", "echo", "path", "help", "pause", "quit"};
char *PATH = "/bin";

int main() {
    shell_prog();
    return EXIT_SUCCESS;
}

void shell_prog() {
    /*
    start a shell program
    as long as the status isn't zero, which ONLY HAPPENS WHEN THE USER CALLS
    QUIT, we read the command from stdin, parse the command and execute it.
    */

    char *command;
    char **argsList;
    int status = 0;
    char *buf = (char *)malloc((size_t)MAXPATHLEN);
    char *ptr = NULL;

    do {
        /* get the current directory */
        ptr = getcwd(buf, (size_t)MAXPATHLEN);
        printf("%s> ", ptr);
        command = read_command();
        argsList = parse_command(command);
        status = execute_command(argsList);
        free(command);
        free(argsList);
        /* Explicitly setting the disposition of SIGCHLD to SIG_IGN causes any child process that subsequently
        terminates to be immediately removed from the system instead of being converted into a zombie. */
        signal(SIGCHLD, SIG_IGN);
    } while (status);
}

char *read_command(void) {
    /*
    read command from stdin
    returns: command from user
    */

    char *command = NULL;
    size_t bufsize = 0;

    /*
    getline reads an entire line from stdin, storing the address of the
    buffer containing the text into command. Since command is set to NULL
    and bufsize is set to 0 before the call, getline will allocate a buffer
    for storing the line
    */

    if (getline(&command, &bufsize, stdin) == -1) {
        fprintf(stderr, "error reading from stdin %s\n", strerror(errno));
        return NULL;
    }
    return command;
}

char **parse_command(char *command) {
    /*
    argument: command from user
    return: an array of strings of arguments
    */

    int bufsize = 64;
    int argsLen = bufsize;
    char **argsList = malloc(sizeof(char *) * argsLen);

    if (strcmp(command, "\n") == 0) {
        // if the user returns without any command, set the first
        // element in argsList to the empty string
        argsList[0] = "";
    } else {
        const char *delim = " \"\t\n";
        char *arg = strtok(command, delim); // tokenized the command
        int i = 0;
        while (arg) {
            argsList[i++] = arg;
            arg = strtok(NULL, delim);
            if (i > argsLen) {
                // if we have more arguments than slots in
                // argsList, extend argsList by reallocating it
                argsLen += bufsize;
                argsList = realloc(argsList, argsLen);
            }
        }
        argsList[i] = NULL;
    }

    return argsList;
}

int execute_command(char **argsList) {
    /*
    argument: an array of strings of arguments
    return: status of the execution for the command
    */

    int i = 0;
    int status;
    int n = get_len_array(argsList);
    if (strcmp(argsList[0], "") == 0) // if there is no command returned by user
    {
        return 1;
    }

    /* Check if have background process*/
    int bg = (strcmp(argsList[(n - 1)], "&") == 0);
    if (bg) {
        /* if background process, set the last element in the array (which is &) to NULL*/
        argsList[n - 1] = NULL;
    }

    int index = -1;
    int left = -1;
    int right = -1;
    if ((index = detect_pipe(argsList)) != -1) {
        /* create two arrays of arguments */
        char *args1[index];
        char *args2[(n - index)];

        int i = 0;
        for (; i < index; i++) {
            args1[i] = argsList[i];
        }
        args1[i++] = NULL;

        int j = 0;
        for (; argsList[i] != NULL; i++, j++) {
            args2[j] = argsList[i];
        }
        args2[j] = NULL;

        status = execute_pipe(args1, args2);
    } else if ((left = detect_left(argsList)) != -1 | (right = detect_right(argsList)) != -1) {
        status = redirect(left, right, argsList);
    } else if ((index = detect_parallel(argsList)) != -1) {
        /* create two arrays of arguments */
        char *args1[index];
        char *args2[(n - index)];

        int i = 0;
        for (; i < index; i++) {
            args1[i] = argsList[i];
        }
        args1[i++] = NULL;

        int j = 0;
        for (; argsList[i] != NULL; i++, j++) {
            args2[j] = argsList[i];
        }
        args2[j] = NULL;

        status = execute_parallel(args1, args2);
    } else {
        if ((index = detect_builtins(argsList[0])) != -1) {
            if (bg) {
                /* if background process, we fork() and call the builtin command in the child process */
                pid_t pid;

                if ((pid = fork()) == -1) {
                    fprintf(stderr, "fork error %s\n", strerror(errno));
                } else if (pid == 0) {
                    /* In the child process, we call the
                     * builtin command */
                    status = builtin_commands[index](argsList);
                }
            } else {
                status = builtin_commands[index](argsList);
            }
        } else {
            /* if user doesn't call builtin command, e.g., user calls external command,
            call execute_external_command handler */
            status = execute_external_command(argsList, bg);
        }
    }

    return status;
}

int execute_external_command(char **argsList, int bg) {
    /*
    arguments: an array of arguments with the first element being an
    external command return: 1 both when the command fails to execute and
    when the command succeeds
    */

    pid_t pid;
    int status;

    if ((pid = fork()) == -1) {
        fprintf(stderr, "fork error %s\n", strerror(errno));
    } else if (pid == 0) {
        /* At the child process, we execute the command */
        execvp(argsList[0], argsList);
        fprintf(stderr, "failed executing command: %s\n", strerror(errno));
        exit(1);
    } else {
        /* At the parent process, don't wait if not background */
        if (!bg) {
            do {
                waitpid(pid, &status, 0);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    return 1;
}

/*
Advance features
*/

int execute_pipe(char **args1, char **args2) {
    /*
    arguments: 2 arrays of arguments
    Execute a pipe with the 2 arrays of arguments
    return: 1
    */

    int fd[2];
    int index;
    int status;
    pid_t pid;

    if (pipe(fd) == -1) {
        fprintf(stderr, "pipe error %s\n", strerror(errno));
    } else {
        /*
        There are two cases we need to take care of: builtin command and external command
        There are only 4 builtins that can be used in a pipe: dir, echo, environ, help;
        these commands only happen in the write-end of the pipe.
        Hence we just need to check if args1[0] is a builtin
        */

        if ((pid = fork()) == -1) {
            fprintf(stderr, "fork error %s\n", strerror(errno));
        } else if (pid == 0) {
            /*
            At the child process, we close the read-end, duplicate the write-end into STDOUT_FILENO
            Then we check if args1[0] is builtin, if it is, call the builtin and
            exit(0), else if it is external call execvp
            */

            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            if ((index = detect_builtins(args1[0])) != -1) {
                builtin_commands[index](args1);
                exit(0);
            } else {
                execvp(args1[0], args1);
                fprintf(stderr, "failed to execute command 1: %s\n", strerror(errno));
                exit(1);
            }
        } else {
            /*
            At the parent process, we have to fork again or else the shell process will be overwritten by execvp.
            At the child process of the second fork, close the write-end, duplicate the read-end into STDIN_FILENO,
            and execute.
            Finally, at the parent process of the second fork, close the read-end, close the write-end and
            reap the child process.
            */

            if ((pid = fork()) == -1) {
                fprintf(stderr, "fork error %s\n", strerror(errno));
            } else if (pid == 0) {
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                execvp(args2[0], args2);
                fprintf(stderr, "failed to execute command 2: %s\n", strerror(errno));
                exit(1);
            } else {
                close(fd[0]);
                close(fd[1]);
                waitpid(pid, &status, WUNTRACED);
            }
        }
    }

    return 1;
}

int redirect(int left, int right, char **argsList) {
    /*
    arguments:
    left is index of < (-1 if argsList doesn't have one), right: index of > (-1 if argsList doesn't have one),
    argsList is an array of arguments perform I/O redirection return: 1
    */

    pid_t pid;
    int status;

    if ((pid = fork()) == -1) {
        fprintf(stderr, "fork error %s \n", strerror(errno));
    } else if (pid == 0) {
        /* In the child process, we open the file(s), perform I/O redirection and exec */
        if (left != -1) {
            char *input = argsList[left + 1];
            int fd1;
            if ((fd1 = open(input, O_RDONLY, 00755)) == -1) {
                fprintf(stderr, "error openning file %s %s\n", input, strerror(errno));
            }
            dup2(fd1, STDIN_FILENO);
        }

        if (right != -1) {
            char *output = argsList[right + 1];
            int fd2;
            if ((fd2 = open(output, O_RDWR | O_CREAT | O_TRUNC, 00755)) == -1) {
                fprintf(stderr, "error openning file %s %s\n", output, strerror(errno));
            }
            dup2(fd2, STDOUT_FILENO);
        }

        /* extract command and arguments from argsList */
        int pivot;
        if (left == -1) { // no input just output
            pivot = right;
        } else if (right > left) {
            /* command arguments < some_file > some_other_file */
            pivot = left;
        } else if (right == -1) { // no output just input
            pivot = left;
        } else {
            /* command arguments > some_file < some_other_file */
            pivot = right;
        }

        char *args[pivot];
        int i = 0;
        for (; i < pivot; i++) {
            args[i] = argsList[i];
        }
        args[i] = NULL;

        int index;
        if ((index = detect_builtins(args[0])) != -1) {
            builtin_commands[index](args);
            exit(0);
        } else {
            execvp(args[0], args);
            fprintf(stderr, "error executing command %s", strerror(errno));
            exit(1);
        }
    } else {
        /* In the parent process, we wait and reap the child */
        waitpid(pid, &status, WUNTRACED);
    }

    return 1;
}

int execute_parallel(char **args1, char **args2) {
    /*
    arguments: 2 arrays of arguments
    return: 1
    */

    pid_t pid1;
    pid_t pid2;
    int status;
    int index;

    if ((pid1 = fork()) == -1) {
        fprintf(stderr, "fork error %s \n", strerror(errno));
    } else if (pid1 == 0) {
        if ((index = detect_builtins(args1[0])) != -1) {
            builtin_commands[index](args1);
            exit(0);
        } else {
            execvp(args1[0], args1);
            fprintf(stderr, "failed to execute command 1: %s\n", strerror(errno));
            exit(1);
        }
    } else {
        if ((index = detect_builtins(args2[0])) != -1) {
          builtin_commands[index](args2);
        } else {
            if ((pid2 = fork()) == -1) {
                fprintf(stderr, "fork error %s\n", strerror(errno));
            } else if (pid2 == 0) {
                execvp(args2[0], args2);
                fprintf(stderr, "failed to execute command 2: %s\n", strerror(errno));
                exit(1);
            } else {
                waitpid(pid2, &status, WUNTRACED);
            }
        }
    }

    return 1;
}