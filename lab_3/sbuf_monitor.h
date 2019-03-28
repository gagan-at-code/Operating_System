#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>


#ifndef SBUF_MONITOR_H_
#define SBUF_MONITOR_H_

typedef struct {
    int *buf;
    int size;
    int front;
    int rear;
    sem_t mutex;
    pthread_cond_t empty_slot;
    pthread_cond_t full_slot;
} subf_monitor_t;



#endif