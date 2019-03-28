# Simple Unix Shell

## DESCRIPTION

This program is a simple command line shell that executes commands read from the standard input or from a file.

These commands can be built-in (listed below) or from environment path. 

It should work in the similar manner as any terminals. All commands are terminated with newline character.

## BUILT-IN COMMANDS

* cd \<directory\>: change the current directory to \<directory\>. If the directory specified does not exist report an error. If there is no argument, change to $HOME.
* clr: Clear the screen.
* dir \<directory\>: List the contents of a directory.
* environ: List all the environment strings.
* echo: Followed by a string and print that string onto the screen with a newline character.
* path: the path command takes 0 or more arguments and reset the search path environment variable
* help: Display the user manual.
* pause: Pause the operation of the shell until Enter is pressed
* quit: Quit the shell.

## Advanced Feature

### I/O Redirection

Output of commands can be redirected from stdout to a file with symbols > or >> (where >> append the output to what currently exists in the output file), and input of commands can be from a file with symbol <:
  
```
$ cmd > outfile
$ cmd < infile
$ cmd < infile > outfile
$ cmd > outfile < infile
```
In this case, the output of cmd is redirected to file, if file does not exist then the shell will create one.

### Pipe

Output of one command can be chained to the input of another command. The syntax is as follow:
```
$ cmd1 | cmd2  | cmd3
```

where output of cmd1 is the input of cmd2 and output of cmd2 is input of cmd3.

### BACKGROUND PROCESS

By default, when a program runs, the shell waits for it to finish before the user can perform any further action. However, in order to run a program in background, add '&' immediately after the command name, such as:
```
$ [command name]&
```

In this case, the process runs silently in background and the user can continue using the shell.

### PARALLEL PROCESS 

To run several commands parallely, use the ampersnad operator as follow:
```
$ cmd1 args1 & cmd2 args2 & cmd3 args3
```
In this case, instead of running cmd1 and then waiting for it to finish, the shell runs cmd1, cmd2, and cmd3 in parallel.

### BATCH-MODE  

Aside interactive mode, the shell also supports batch mode, which instead reads input from a batch file and executes from therein. Here is how to run the shell with a batch file named batch.txt:

```
$ ./shell batch.txt
```


### FILES
    shell.c     Main components of the shell
    util.c      Built-in functions for this shell
    shell.h     Header file with function declarations and constant definitions

## AUTHOR
        Hoang Ho
        Email: hoang.nk.ho@temple.edu

