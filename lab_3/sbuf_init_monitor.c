#include "sbuf_monitor.h"

void sbuf_init(subf_monitor_t *sp, int n) {
    sp->buf = calloc(n, sizeof(int));
    sp->size = n;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
}