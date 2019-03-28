/*
author: Hoang Ho
Simulation File
*/

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "statistics.h"

/*Handling functions*/

int gen_next_event();
int cpu_handler(Node **job);
int disk_handler(Node **job);
void enqueue_disk(int arrival_time, int id);
int process_disk1(Node **job);
int process_disk2(Node **job);
void read_config();

int SEED;
int INIT_TIME;
int FIN_TIME;
int ARRIVE_MIN;
int ARRIVE_MAX;
float QUIT_PROB;
int CPU_MIN;
int CPU_MAX;
int DISK1_MIN;
int DISK1_MAX;
int DISK2_MIN;
int DISK2_MAX;

Queue *event_queue = NULL;
Queue *cpu_queue = NULL;
Queue *disk1_queue = NULL;
Queue *disk2_queue = NULL;

int time_cpu_free = 0;
int time_disk1_free = 0;
int time_disk2_free = 0;
int job_id = 0;

/*
SIMULATION
*/

int main(int argc, char *argv[])
{
    read_config(); // start the program by reading in the configurations
    srand(SEED); 

    /*
    Generate the first job and enqueue the job into the event_queue and cpu_queue
    */
    int next = rand() % (ARRIVE_MAX - ARRIVE_MIN);
    int current_time = next;
    job_id += 1;

    event_queue = newQueue();
    enqueue(event_queue, next, next, job_id, 0);
    cpu_queue = newQueue();
    enqueue(cpu_queue, next, next, job_id, 0);

    disk1_queue = newQueue();
    disk2_queue = newQueue();

    Node *temp = NULL;
    int finished_time = 0;

    start_log();

    while (!isEmpty(event_queue) && current_time <= FIN_TIME)
    {
        /*
        1. Pop an event out of the event_queue and generate the next event coming to the event queue
        2. Check what server the event is coming in, and use the corresponding handler
        3. If the current_time is smaller than the time the server finishes the event, set the current_time to 
        finished_time
        */
        printf("Current Time: %d\n", current_time);
        temp = pop(event_queue);
        next = gen_next_event(next);
        if (temp->server != 0)
        {
            finished_time = disk_handler(&temp);
        }
        else
        {
            finished_time = cpu_handler(&temp);
        }
        /* Need to update the current time */
        if (current_time < finished_time)
        {
            current_time = finished_time;
        }
        free(temp);
    }
    calculate_statistics(INIT_TIME, FIN_TIME); // After the simulation, calculate the necessary statistics
}

/*
Handler Functions
1. Generate the Next Event
2. CPU Handler
3. Enqueue to disk
4. Disk handler
5. Process disk 1
6. Process disk 2
*/

int gen_next_event(int last_arrival)
{
    job_id += 1;
    int next_arrival = last_arrival + ((rand() % (ARRIVE_MAX - ARRIVE_MIN)) + ARRIVE_MIN);
    enqueue(event_queue, next_arrival, time_cpu_free, job_id, 0);
    enqueue(cpu_queue, next_arrival, time_cpu_free, job_id, 0);
    return next_arrival;
}

int cpu_handler(Node **job)
{
    int current_time = (*job)->pop_time;
    if (current_time < time_cpu_free)
    {
        current_time = time_cpu_free;
        (*job)->pop_time = current_time;
    }
    int id = dequeue(cpu_queue);
    int time_handling = (rand() % (CPU_MAX - CPU_MIN)) + CPU_MIN + 1;
    time_cpu_free = current_time + time_handling; // update time_cpu_free

    int response_time = time_cpu_free - ((*job)->arrival_time);
    log_statistics((*job)->id, (*job)->server, (*job)->arrival_time, (*job)->pop_time, time_cpu_free,
                   response_time, time_handling, subSize(cpu_queue, time_cpu_free));

    if (cpu_queue->head)
    {
        cpu_queue->head->pop_time = time_cpu_free;
    }

    int diskNeeded = (rand() % 100) > (QUIT_PROB * 100);
    if (diskNeeded)
    {
        enqueue_disk(time_cpu_free, id);
    }
    return time_cpu_free;
}

void enqueue_disk(int arrival_time, int id)
{
    if (disk1_queue->size > disk2_queue->size)
    {
        enqueue(event_queue, arrival_time, time_disk2_free, id, 2);
        enqueue(disk2_queue, arrival_time, time_disk2_free, id, 2);
    }
    else
    {
        enqueue(event_queue, arrival_time, time_disk1_free, id, 1);
        enqueue(disk1_queue, arrival_time, time_disk1_free, id, 1);
    }
}

int disk_handler(Node **job)
{
    int finished_time;
    if ((*job)->server == 1)
    {
        int id = dequeue(disk1_queue);
        finished_time = process_disk1(job);
        time_disk1_free = finished_time;

        int response_time = time_disk1_free - ((*job)->arrival_time);
        int process_time = time_disk1_free - ((*job)->pop_time);
        log_statistics((*job)->id, (*job)->server, (*job)->arrival_time, (*job)->pop_time, time_disk1_free,
                       response_time, process_time, subSize(disk1_queue, time_disk1_free));

        if (disk1_queue->head)
        {
            disk1_queue->head->pop_time = time_disk1_free;
        }

        enqueue(event_queue, finished_time, time_cpu_free, id, 0);
        enqueue(cpu_queue, finished_time, time_cpu_free, id, 0);
    }
    else
    {
        int id = dequeue(disk2_queue);
        finished_time = process_disk2(job);
        time_disk2_free = finished_time;
        
        if (disk2_queue->head)
        {
            disk2_queue->head->pop_time = time_disk2_free;
        }

        int response_time = time_disk2_free - ((*job)->arrival_time);
        int process_time = time_disk2_free - ((*job)->pop_time);
        log_statistics((*job)->id, (*job)->server, (*job)->arrival_time, (*job)->pop_time, time_disk2_free,
                       response_time, process_time, subSize(disk2_queue, time_disk2_free));

        enqueue(event_queue, finished_time, time_cpu_free, id, 0);
        enqueue(cpu_queue, finished_time, time_cpu_free, id, 0);
    }
    return finished_time;
}

int process_disk1(Node **job)
{
    int current_time = (*job)->pop_time;
    if (current_time < time_disk1_free)
    {
        current_time = time_disk1_free;
        (*job)->pop_time = current_time;
    }
    int time_handling = (rand() % (DISK1_MAX - DISK1_MIN)) + DISK1_MIN + 1;
    return current_time + time_handling;
}

int process_disk2(Node **job)
{
    int current_time = (*job)->pop_time;
    if (current_time < time_disk2_free)
    {
        current_time = time_disk2_free;
        (*job)->pop_time = current_time;
    }
    int time_handling = (rand() % (DISK2_MAX - DISK2_MIN)) + DISK2_MIN + 1;
    return current_time + time_handling;
}

/*
Read, write and calculate statistics function
*/

void read_config()
{
    FILE *f;
    f = fopen("./config.txt", "r");

    //read file into array

    if (f == NULL)
    {
        printf("Error Reading File\n");
        exit(0);
    }
    fscanf(f, "%d\n", &SEED);
    fscanf(f, "%d\n", &INIT_TIME);
    fscanf(f, "%d\n", &FIN_TIME);
    fscanf(f, "%d\n", &ARRIVE_MIN);
    fscanf(f, "%d\n", &ARRIVE_MAX);
    fscanf(f, "%f\n", &QUIT_PROB);
    fscanf(f, "%d\n", &CPU_MIN);
    fscanf(f, "%d\n", &CPU_MAX);
    fscanf(f, "%d\n", &DISK1_MIN);
    fscanf(f, "%d\n", &DISK1_MAX);
    fscanf(f, "%d\n", &DISK2_MIN);
    fscanf(f, "%d\n", &DISK2_MAX);
    fclose(f);
}
