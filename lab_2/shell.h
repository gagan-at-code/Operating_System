/*
Author: Hoang Ho
Simple Unix Shell Program
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

// builtin commands
int cd(char **args);
int clr(char **args);
int dir(char **args);
int env(char **args);
int echo(char **args);
int path(char **args);
int help(char **args);
int myPause(char **args);
int quit(char **args);

// handler functions
void shell_prog();
char *read_command(void);
char **parse_command(char *command);
int execute_command(char **argsList);
int execute_external_command(char **argsList, int bg);
int execute_pipe(char **argsList, int *indices, int n_pipes, int n_args);
int redirect(int left, int right, char **argsList);
int execute_parallel(char **args1, char **args2);


// helper functions
int detect_builtins(char *command);
int detect_pipe(char **argsList);
int detect_left(char **argsList);
int detect_right(char **argsList);
int detect_background(char **argsList);
int detect_parallel(char **argsList);
int get_len_array(char **argsList);


extern char *PATH;
extern char *builtins[10];


