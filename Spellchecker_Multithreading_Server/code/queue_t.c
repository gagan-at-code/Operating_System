#include "queue_t.h"
#include "wrappers.h"

/* Create an empty, bounded, shared FIFO buffer with n slots */
void queue_init(queue_t *queue, int n) {
    queue->buf = calloc(n, sizeof(int));
    queue->n = n;
    queue->front = queue->rear = 0;
    
    CHECK(pthread_mutex_init(&queue->mutex, NULL) == 0, "pthread_mutex_init");
    
    queue->slots = sem_open("slots", O_CREAT, 0644, n);
    CHECK(sem_unlink("slots") == 0, "sem_unlink");
    CHECK(queue->slots != SEM_FAILED, "sem_open");
    
    queue->items = sem_open("items", O_CREAT, 0644, 0);
    CHECK(sem_unlink("items") == 0, "sem_unlink");
    CHECK(queue->items != SEM_FAILED, "sem_open");
}

/* Clean up buffer sp */
void queue_deinit(queue_t *queue) {
    free(queue->buf);
}

/* Insert item onto the rear of shared buffer sp */
void enqueue(queue_t *queue, int item) {
    CHECK(sem_wait(queue->slots)==0, "sem_wait");      /* Wait for available slots */
    CHECK(pthread_mutex_lock(&queue->mutex) == 0, "pthread_mutex_lock"); /* Lock the buffer */
    queue->buf[(++queue->rear)%(queue->n)] = item;   /* Insert the item */
    CHECK(pthread_mutex_unlock(&queue->mutex) == 0, "pthread_mutex_unlock");     /* Unlock the buffer */
    CHECK(sem_post(queue->items) == 0, "sem_post");      /* Announce available items here */
}


/* Remove and return the first item from the buffer */
int dequeue(queue_t *queue) {
    int item;
    CHECK(sem_wait(queue->items) == 0, "sem_wait");
    CHECK(pthread_mutex_lock(&queue->mutex) == 0, "pthread_mutex_lock");
    item = queue->buf[(++queue->front)%(queue->n)];
    CHECK(pthread_mutex_unlock(&queue->mutex) == 0, "pthread_mutex_unlock");
    CHECK(sem_post(queue->slots) == 0, "sem_post");
    return item;
}

void destroy(queue_t *queue)
{
    CHECK(pthread_mutex_destroy(&queue->mutex) == 0, "pthread_mutex_destroy");
    CHECK(sem_close(queue->items) == 0, "sem_close");
    CHECK(sem_close(queue->slots) == 0, "sem_close");
}