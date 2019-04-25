#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <semaphore.h>
#include <assert.h>
#include <arpa/inet.h>

#define BACKLOG 20

#define CHECK(expr, msg)                        \
    do {                                        \
        if (!(expr)) {                          \
            perror(msg);                        \
            exit(EXIT_FAILURE);                 \
        }                                       \
    } while (0)

int open_listenfd(int port);
int open_clientfd(int port);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
void Pthread_detach(pthread_t tid);
void Close(int fd);
