/*
author: Hoang Ho
Data Structures file
*/

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

Node *newNode(int t, int pop, int id, int server)
{
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->arrival_time = t;
    temp->pop_time = pop;
    temp->id = id;
    if (temp->arrival_time > pop)
    {
        temp->pop_time = t;
    }
    temp->server = server;
    temp->next = NULL;
    return temp;
}

Queue *newQueue()
{
    Queue *tmp = (Queue *)malloc(sizeof(Queue));
    tmp->size = 0;
    tmp->head = NULL;
    return tmp;
}

int isEmpty(Queue *queue)
{
    return queue->size == 0;
}

int dequeue(Queue *queue)
{
    if ((queue->head) != NULL)
    {
        queue->size -= 1;
        Node *temp = (queue->head);
        (queue->head) = temp->next;
        int id = temp->id;
        free(temp);
        return id;
    }
    return 0;
}

Node *pop(Queue *queue)
{
    if ((queue->head) != NULL)
    {
        queue->size -= 1;
        Node *temp = (queue->head);
        (queue->head) = temp->next;
        return temp;
    }
    return NULL;
}

void enqueue(Queue *queue, int t, int pop, int id, int server)
{
    if (isEmpty(queue))
    {
        queue->head = newNode(t, pop, id, server);
    }
    else
    {
        Node *tmp = newNode(t, pop, id, server);
        if ((queue->head->arrival_time) > (tmp->arrival_time))
        {
            tmp->next = queue->head;
            queue->head = tmp;
        }
        else
        {
            Node *start = queue->head;
            while (start->next != NULL && start->next->arrival_time <= (tmp->arrival_time))
            {
                start = start->next;
            }
            tmp->next = start->next;
            start->next = tmp;
        }
    }
    queue->size += 1;
}

int subSize(Queue *queue, int t)
{
    if (queue->size == 0 || queue->head->arrival_time > t)
    {
        return 0;
    }
    else
    {
        Node *start = queue->head;
        int count = 0;
        while (start != NULL && start->arrival_time <= t)
        {
            start = start->next;
            count += 1;
        }
        return count;
    }
}
