#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L
#include "wrappers.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>

#define NUM_WORKERS 20
#define DEFAULT_PORT "5555"
char *words[8] = {"hello\n", "worldes\n", "my\n", "name\n", "is\n", "allen\n", "salut\n"};
void *connection_handler(void *port);

int main(int argc, char **argv) {
    int portNum = argc > 1 ? atoi(argv[1]) : atoi(DEFAULT_PORT);
    pthread_t client_thread;
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_create(&client_thread, NULL, connection_handler, (void *)&portNum);
    }
    pthread_exit(NULL);
    return 0;
}

void *connection_handler(void *port) {
    struct timespec tstart, tend;
    tstart.tv_sec = 0;
    tstart.tv_nsec = 0;
    tend.tv_sec = 0;
    tend.tv_nsec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tstart);
    int portNum = *(int *)port;
    int clientSocket = open_clientfd(portNum);
    /*
    in MacOS
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    */
    pid_t tid = syscall(__NR_gettid);

    srand(time(NULL));
    char *hello = words[tid % 7];
    char buffer[1024];

    if (write(clientSocket, hello, strlen(hello)) < 0) {
        perror("write");
    }

    if (read(clientSocket, buffer, 1024) < 0) {
        perror("receive failure");
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("In time %.5f second, thread %d gets message %s",
           ((double)tend.tv_sec + 1.0e-9 * tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec), tid,
           buffer);
    close(clientSocket);
    return 0;
}
