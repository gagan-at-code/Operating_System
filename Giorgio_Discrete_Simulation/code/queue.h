/*
author: Hoang Ho
Data Structures file
*/

typedef struct event_queue_node
{
    int arrival_time;
    int pop_time;
    int id;
    int server;
    struct event_queue_node *next;
} Node;

typedef struct priority_queue {
    int size;
    Node *head;
} Queue;


Node *newNode(int t, int pop, int id, int server);
Queue *newQueue();
int isEmpty(Queue *queue);
int dequeue(Queue *queue);
Node *pop(Queue *queue);
void enqueue(Queue *queue, int t, int pop, int id, int server);
int subSize(Queue *queue, int t);