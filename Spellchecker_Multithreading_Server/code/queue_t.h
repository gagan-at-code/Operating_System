#include <fcntl.h> 
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef SBUF_MONITOR_H_
#define SBUF_MONITOR_H_

typedef struct {
    int *buf;
    int n;
    int front;
    int rear;
    pthread_mutex_t mutex;
    sem_t *slots;
    sem_t *items;
} queue_t;

#endif

void queue_init(queue_t *sp, int n);
void queue_deinit(queue_t *sp);
void enqueue(queue_t *sp, int item);
int dequeue(queue_t *sp);
void destroy(queue_t *queue);

