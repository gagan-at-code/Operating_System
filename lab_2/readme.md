# **Hoang_Ho's Simple Shell Documentation**

## **1. General Ideas**

Example of a command line

```
> cd /home/User/hoangho
```

This command change the current working directory to "/home/User/hoang". A command line allows the user to interact with the computer. The shell takes in a string of characters (terminated by pressing the Enter or Return key) and the OS responds by executing the command the user writes and returns the output to the screen if any. The goal of the assignment is to create a simple shell program. Our shell will support basic builtin commands and external commands from the user. The shell will also allow for I/O redirection, background process, and shell pipe. 

Generally, Shell starts by printing a prompt and waiting for the command from the user. When a command is typed in, first the shell must read the command, then it creates an array of arguments, argv, in which, the first element is the  name of the command, and the rest are the arguments. Next the shell finds in the PATH the executable file corresponding with the command name and executes the command. After the execution succeeds, the shell prints the result (if any) and gets back a status variable. If the execution fails, the shell prints an error message and gets back a status variable. The status variable will determine the intention of the user to continue running the shell.

## **2. Core while loop**

At the core of the shell, there is a do-while loop as follow:

```C
int main() {
    start_shell();
    return EXIT_SUCCESS;
}

void start_shell() {
    char *command;
    char **args;
    int status;

    do {
        printf("> "); // print the prompt
        line = read_command(); // read the command 
        args = parse_command(); // parse the command into array of arguments
        status = execute_command(args); // execute the command with args
        puts("");
    } while(status);
}
```

However, we cannot execute every program in the do-while loop, because some erronous programs may cause everything to shutdown. Thus, we need to separate 2 kinds of commands: builtin commands and external commands. For builtin commands, we call functions to execute it directly our shell. For external commands, we create a directory (a PATH) that contains their executables and perform fork() and execvp() whenever we call them. By doing this, even though the external commands may contain fatal error, the process got shut down is the child process not our shell process. To read the command, we can make use of getline(). To parse the command, we can use strtok(). As for execute_command, we will first check if the command is a builtin, if it is, we execute the command directly, if not, we call a handler to fork() and exec() the external command.

## **3. Builtin Commands**

Builtin commands are commands that are built into the shell and can be called directly. Our shell supports the following builtin functions:

* cd - change directory. This command always take one argument and use chdir() system call with the argument to change directory
* clr - clear the screen. Use system("clear")
* dir - list the contents of directory. Make use of dirent.h
* environ - list all the environment strings. Use extern char **environ, which is an array of environment variables and print them to the screen
* echo - takes an argument (usually a file or a string) and display it. Multiple spaces/tabs may be reduced to a single space
* help - display user manual
* quit - quit the shell and return 0 to status
* path - the command takes 0 or more arguments, with each argument separated by whitespace from the others. Set the arguments to be the searched path of the shell

We will use an array of function pointers to keep track of our builtin functions as follow:

```C
int (*builtin_command[])(char **args) = {&cd, &clr, &dir, &environ, &echo, &help, &quit, &path};
char **builtins = {"cd", "clr", "dir", "environ", "echo", "help", "quit", "path"};
```

In execute_command() (in section 2), we will loop through builtins to see if any string match a builtin command string. If it is, execute the corresponding command from builtin_command, or else, call the external execution. 

**Side note on environment variables**: environment variables in the system that describe your environment. The most well-known environment variables is PATH, which contains the path to all folders that might contain executables.

## **4. External Commands**

External commands are commands that aren't builtin into the shell and have executables in our search path. For this kind of command, we need fork() and exec(). The idea is our program (the parent) will first fork() and create a child process which will exec() the external command. Once the child finishes executing, the parent (our program) will reap the child process by waitpid().

```C
int execute_external_command(char **args) {
    int status = 0;
    pid_t pid;
    pid = fork();
    char **argv = args[1:];

    if (pid == -1) {
        // if fork() fails, print error message
        fprintf(stderr, "fork error: %s\n", stderror(erno));
    } else if (pid == 0) {
        /* If in the child process, execute the process */
        if (execvp(args[0], argv) == -1) {
            // execvp returns -1 if there is an error. If no error, there will no return
            perror("Error executing");
        }
    } else {
        if (waitpid(pid, &status, 0) != pid) {
            // if waitpid fails, it will return -1. If it succeeds, it returns the pid of the child
            fprintf(stderr, "waitpid error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
}
```

When we call fork(), we create an exact copy of the current one. Then we call execvp, we replace the current running process with a new one. We will reap the child process by calling waitpid in the parent process, returns success if waitpid succeeds. By going through such process, we are guarranteed that no fatal error can kill the running shell process. Before start calling this execute_external_command, in execute_command() (in section 2), after checking if the command is builtin or not, we need to search through our search path to see if such external command exists. We only call execute_external_command if the external command exists in the search path.

## **5 Advanced Features**

### **a. I/O Redirection**

When created, a process has three default file identifiers: stdin, stdout, and stderr. The data that the shell receives will be direted from the keyboard to the stdin file descriptor. Similarly, the data received from stdout and stderr are mapped to the terminal display. With I/O redirection, we allow the user can redefine stdin and stdout whenever a command is enterred. If the user provides a filename argument to the command and precedes the filename with a "<" character, then the shell will substitute the designated file for stdin. Likewise, if the user precede a filename with the right angular brace character ">" character, then we redirect stdout to the file. 

To get I/O redirection working, we need to replace the contents of the file descriptor entry for stdin, currently the keyboard. The following is example code to redirect stdin. 

```C
int newStdin = open("main.cpp", O_RDONLY);
// the open function converts a filename to a file descriptor and returns the descriptor number
int saveStdin = dup(0);
// save file descriptor for stdin for later usage
close(0); // wipe out the contents of file descriptor table entry 0, which is the table entry for stdin
dup2(newstdin, 0); 
// dup2 system call creates a copy of the file descriptor newStdin using the file descriptor 0. 
close(newstdin);
// cleans things up so that only the stdin file descriptor that is linked to newstdin

/*
Perform execution by calling a builtin command or execute_external_command
*/

// Restore stdin
dup2(saveStdin, 0);
// creates a copy of the file descriptor saveStdin using file descriptor 0
close(saveStdin);
// file descriptor 0 is the only file descriptor that is linked to stdin
```

### **b. Background process**

In the normal paradigm, the parent process creates a child process, starts it executing the command, and then waits until the child process terminates. If "&" operator is used to terminate the command line, then the shell is expected to create a child process and start it executing on the designated command but **not have the parent wait** for the child to terminate. That is, the parent and the child will execute concurrently. While the child executes the command, the parents prints another prompt to stdout and waits for the user to enter another command line.

Try:

```
> ls &
```

This would let ls command to run in the background and let the user continue using the foreground process in the terminal by printing another prompt. To tackle this problem, we need to first fork() to create the child process, then call setpgid(0,0) and in the parent instead of calling waitpid(), we continue to execute and then afterward, everytime we print a prompt, we call waitpid with WNOHANG option to check if any process has stopped running and reap it.

### **c. Shell Pipe**

A pipe is a common IPC (InterProcess Communication) mechanism in Linux and other versions of UNIX. A process can send data by writing it into one end of the pipe and another can receive that data by reading the other end of the pipe. Example of a shell pipe is:

```
> ls | more
```

Here the output of ls will be the input of more. In other words, instead of writing the output of ls out in the screen, we write the output of ls into the more interface. Pseudocode for shell pipe is as follow:

```C
int execute_pipe(char **args) {
    int fd[2];
    pid_t pid;
    char buf[BUF_SIZE];
    
    if (pipe(fd) == -1) {
        /* 
        create a pipe. The array fd is used to return 2 file descriptors referring to the end of the pipe
        fd[0] referes to the read end of the file, and fd[1] refers to the write end of the file. Data written 
        to the write end of the pipe is buffered by the kernel until it is read from the read end of the pipe.

        pipe(fd) returns -1 on failure
         */
        fprintf(stderr, "pipe error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if ((pid=fork()) == -1) {
        fprintf(stderr, "fork error: %s\n", strerror(errno));
        print EXIT_FAILURE;
    } else if (pid == 0) {
        /* at the child process, close the write end and execute */
        if (close(fd[1]) == -1) {
            fprintf(stderr, "error closing the write end\n");
            return EXIT_FAILURE;
        }
        /* read input from the read end and execute */
    } else {
        if (close(fd[0]) == - 1) {
            fprintf(stderr, "error closing the read end\n");
            return EXIT_FAILURE;
        }
        /* execute and write outputs to the write end */
    }
    return EXIT_SUCCESS; 
}
```

## **6. Attacking Problems**

Writing a simple shell is not a small problem, so we need to tackle it step by step. 

* First step should be to build a shell that support only the builtin commands first. Design several test cases to make sure the shell succeed in supporting basic builtin commands. 
* The second step should be to put several executable files in our search path, design test cases for external commands and make sure the shell succeeds.
* The third step should be to support advanced features, and the easiest among the advanced features is I/O redirection. Design test cases for I/O redirection and makes sure the shell succeeds in I/O redirection with both builtin and external command
* The fourth step should be to support shell pipe and desgin test cases for shell pipe with both builtin and external commands
* The last step should be to support background process, which is the hardest advanced feature, and desgin test cases for background processes with both builtin and external commands


The Environment used for this assignment is Ubuntu 16.04 and GDB as debugger. 