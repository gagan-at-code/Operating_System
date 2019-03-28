/*
author: Hoang Ho
*/
#include <stdlib.h>
#include <stdio.h>

void start_log();
void log_statistics(int id, int server, int arrival_time, int pop_time, int finished_time,
                    int response_time, int process_time, int queue_len);
void calculate_statistics(int INIT_TIME, int FIN_TIME);

