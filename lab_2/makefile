# Author: Hoang Ho

CC=gcc
CFLAGS=-g -Wall -std=c99

all: clean shell

shell: shell.c builtins.c utils.c shell.h
	$(CC) $(CFLAGS) shell.c builtins.c utils.c -o shell

clean:
	$(RM) -rf shell *.dSYM
