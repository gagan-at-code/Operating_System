CC=gcc
CFLAGS=-g -lrt -Wall -std=c99
LIBPATH=-pthread
all: server client

server: server.c wrappers.c wrappers.h queue_t.c queue_t.h hashTable.c hashTable.h
	$(CC) $(CFLAGS) $(LIBPATH) server.c queue_t.c hashTable.c wrappers.c -o server

client: client.c wrappers.c wrappers.h 
	$(CC) $(CFLAGS) $(LIBPATH) client.c wrappers.c -o client