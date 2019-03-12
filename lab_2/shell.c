/*
Author: Hoang Ho
Simple Unix Shell Program
Main Shell Program

TODOs:
1. TAKE CARE OF PIPE (DONE)

2. TAKE CARE OF I/O REDIRECTION (DONE)

3. TAKE CARE OF BACKGROUND PROCESSES (DONE)

4. TAKE CARE OF PARALLEL COMMAND (DONE)

5. TAKE CARE OF BATCH MODE
*/

#include "shell.h"

int (*builtin_commands[10])(char **args) = {&cd, &clr, &dir, &env, &echo, &path, &help, &myPause, &quit};
char *builtins[10] = {"cd", "clr", "dir", "environ", "echo", "path", "help", "pause", "quit"};
char *PATH = "/bin";
int changedPath = 0;

int main(int argc, char **argv) {
    /*the isatty() function shall test whether an open file descriptor, is associated with a terminal device.*/
    if (argv[1] == NULL) {
        /* interactive mode */
        shell_prog();
    } else {
        /* batch mode */
        batch_shell(argv[1]);
    }
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
    putenv("PATH=/bin"); // set search path

    do {
        /* get the current directory */
        ptr = getcwd(buf, (size_t)MAXPATHLEN);
        printf("%s> ", ptr);
        command = read_command();
        argsList = parse_command(command);
        status = execute_command(argsList);
        /* Explicitly setting the disposition of SIGCHLD to SIG_IGN causes any child process that subsequently
        terminates to be immediately removed from the system instead of being converted into a zombie. */
        free(command);
        free(argsList);
        signal(SIGCHLD, SIG_IGN);
    } while (status);
    if (changedPath) {
        free(PATH);
    }
}

int batch_shell(char *filename) {
    /* argument: filename - batch file containing commands to run

    */
    FILE *fp;
    char *command = NULL;
    size_t bufsize = 0;
    ssize_t read;
    char **argsList;
    int status;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        return EXIT_FAILURE;
    }

    while ((read = getline(&command, &bufsize, fp)) != -1) {
        // printf("read command %s of length: %zu\n", command, read);
        argsList = parse_command(command);
        status = execute_command(argsList);
        signal(SIGCHLD, SIG_IGN);
    }
    free(command);
    free(argsList);
    if (changedPath) {
        free(PATH);
    }
    fclose(fp);
    return EXIT_SUCCESS;
}

char *read_command(void) {
    /*
    read command from stdin
    returns: command from user
    */

    char *command = NULL;
    size_t bufsize = 0;

    /*
    getline reads an entire line from stdin, storing the address of the buffer containing the text into command.
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
        /* if the user returns without any command, set the first element in argsList to the empty string */
        argsList[0] = "";
    } else {
        const char *delim = " \"\t\n";
        char *arg = strtok(command, delim); // tokenized the command
        int i = 0;
        while (arg) {
            argsList[i++] = arg;
            arg = strtok(NULL, delim);
            if (i > argsLen) {
                /* if we have more arguments than slots in argsList, extend argsList by reallocating it */
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
    int status = 1;
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
        /* get the indices of the pipe character in argsList */
        int indices[n];
        indices[0] = index;
        int j = 1;
        for (int i = index + 1; i < n; i++) {
            if (strcmp(argsList[i], "|") == 0) {
                indices[j++] = i;
            }
        }
        status = execute_pipe(argsList, indices, j, n);
    } else if ((left = detect_left(argsList)) != -1 | (right = detect_right(argsList)) != -1) {
        /* If have I/O redirection, call handler function */
        status = redirect(left, right, argsList);
    } else if ((index = detect_parallel(argsList)) != -1) {
        /* If have parallel execution, get the indices of the pipe character in argsList */
        int indices[n];
        indices[0] = index;
        int j = 1;
        for (int i = index + 1; i < n - 1; i++) {
            if (strcmp(argsList[i], "&") == 0) {
                indices[j++] = i;
            }
        }
        status = execute_parallel(argsList, indices, (j + 1), n);
    } else {
        /* If usual execution */
        if ((index = detect_builtins(argsList[0])) != -1) {
            if (bg) {
                /* if background process, we fork() and call the builtin command in the child process */
                pid_t pid;

                if ((pid = fork()) == -1) {
                    fprintf(stderr, "fork error %s\n", strerror(errno));
                } else if (pid == 0) {
                    /* In the child process, we call the builtin command */
                    builtin_commands[index](argsList);
                    exit(0);
                }
            } else {
                status = builtin_commands[index](argsList);
            }
        } else {
            /* if user doesn't call builtin command, e.g., user calls external command,
            call execute_external_command handler */
            execute_external_command(argsList, bg);
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

int execute_pipe(char **argsList, int *indices, int n_pipes, int n_args) {
    /*
    arguments:
    argsList is an array of arguments from user
    indices is an array containing indices of pipe in argsList
    n_pipes is the number of pipe
    n_args is the number of arguments
    execute n pipe(s)
    return: 1
    */

    int next;
    int status;
    int index;
    pid_t pid;
    pid_t lPID[(n_pipes + 1)];
    int newfd[2];
    int oldfd[2];
    /* even indices are the read-ends and odd indices are the write-ends */
    int start = 0; // starting indices of a command in argsList
    int n_cmds = n_pipes + 1;

    for (int i = 0; i < n_cmds; i++) {
        /* create a new pipe */
        if (pipe(newfd) == -1) {
            fprintf(stderr, "pipe error %s", strerror(errno));
            return 1;
        }

        /* get the next array of arguments */
        char *args[n_args];
        if (i < n_pipes) {
            /* Not the last command */
            next = indices[i]; // get next appearance of |
            int k = 0;
            for (; k < (next - start); k++) {
                args[k] = argsList[k + start];
            }
            args[k] = NULL;
            start = next + 1; // update the starting point for the next command
        } else {
            /* the last command */
            int k = 0;
            for (; k < (n_args - start); k++) {
                args[k] = argsList[k + start];
            }
            args[k] = NULL;
        }

        /* fork */
        if ((pid = fork()) == -1) {
            fprintf(stderr, "fork error %s", strerror(errno));
        } else if (pid == 0) {
            /* in the child, perform redirection */
            if (i != 0) {
                /* if not the first command, get input from the previous command */
                close(oldfd[1]);
                dup2(oldfd[0], STDIN_FILENO);
                close(oldfd[0]);
            }

            if (i < n_pipes) {
                /* if not the last command, output to the next command */
                close(newfd[0]);
                dup2(newfd[1], STDOUT_FILENO);
                close(newfd[1]);
            }

            if (i == 0 && (index = detect_builtins(args[0])) != -1) {
                builtin_commands[index](args);
                exit(0);
            } else {
                execvp(args[0], args);
                fprintf(stderr, "failed to execute command %d %s", (i + 1), strerror(errno));
                exit(1);
            }
        } else {
            /* In the parent */
            if (i != 0) {
                /* if there is a previouse command, close oldfd */
                close(oldfd[0]);
                close(oldfd[1]);
            }
            if (i < n_pipes) {
                /* if there is a next command */
                oldfd[0] = newfd[0];
                oldfd[1] = newfd[1];
            }
            lPID[i] = pid;
        }
    }

    close(newfd[0]);
    close(newfd[1]);

    for (int j = 0; j < n_pipes + 1; j++) {
        waitpid(lPID[j], &status, WUNTRACED);
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

    /* create a new array to store command and arguments */
    char *args[pivot];
    int i = 0;
    for (; i < pivot; i++) {
        args[i] = argsList[i];
    }
    args[i] = NULL;

    if ((pid = fork()) == -1) {
        fprintf(stderr, "fork error %s \n", strerror(errno));
    } else if (pid == 0) {
        /* In the child process, we open the file(s), perform I/O redirection and exec */
        if (left != -1) {
            /* open file for input */
            char *input = argsList[left + 1];
            int fd1;
            if ((fd1 = open(input, O_RDONLY, 00755)) == -1) {
                fprintf(stderr, "error openning file %s %s\n", input, strerror(errno));
                exit(1);
            }
            dup2(fd1, STDIN_FILENO);
        }

        if (right != -1) {
            /* open file for output */
            char *output = argsList[right + 1];
            int fd2;

            if (strcmp(argsList[right], ">>") == 0) {
                if ((fd2 = open(output, O_RDWR | O_CREAT | O_APPEND, 00755)) == -1) {
                    fprintf(stderr, "error openning file %s %s\n", output, strerror(errno));
                    exit(1);
                }
            } else {
                if ((fd2 = open(output, O_RDWR | O_CREAT | O_TRUNC, 00755)) == -1) {
                    fprintf(stderr, "error openning file %s %s\n", output, strerror(errno));
                    exit(1);
                }
            }
            dup2(fd2, STDOUT_FILENO);
        }

        /* call builtin command if the command is builtin or else execvp */
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

int execute_parallel(char **argsList, int *indices, int n_procs, int n_args) {
    /*
     arguments:
     argsList is an array of arguments from user
     indices is an array containing indices of pipe in argsList
     n_procs is the number of parallel processes
     n_args is the number of arguments
     execute n parallel processes
     return: 1
     */

    int next = 0;
    int status;
    int index;
    pid_t pid;
    /* even indices are the read-ends and odd indices are the write-ends */
    int start = 0; // starting indices of a command in argsList

    for (int i = 0; i < n_procs; i++) {
        /* get the next array of arguments */
        char *args[n_args];
        if (i < (n_procs - 1)) {
            /* Not the last command */
            next = indices[i]; // get next appearance of &
            int k = 0;
            for (; k <= (next - start); k++) {
                args[k] = argsList[k + start];
            }
            args[k] = NULL;
            start = next + 1; // update the starting point for the next command
        } else {
            /* the last command */
            int k = 0;
            for (; k < (n_args - start); k++) {
                args[k] = argsList[k + start];
            }
            args[k] = NULL;
        }

        execute_command(args);
    }
    return 1;
}