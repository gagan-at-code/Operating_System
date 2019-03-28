/*
author: Hoang Ho
Statistics File
*/

#include "statistics.h"

void start_log()
{
    FILE *f;
    f = fopen("./log_statistics.txt", "w+");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "%-15s %-15s %-15s %-15s %-15s %-15s %-15s %-15s\n",
            "Job_ID", "Server", "Arrival", "Pop_time", "Finished", "Response_Time",
            "Process_Time", "Len_Queue");
    fclose(f);
}

void log_statistics(int id, int server, int arrival_time, int pop_time, int finished_time,
                    int response_time, int process_time, int queue_len)
{
    FILE *f;
    f = fopen("./log_statistics.txt", "a+");

    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    fprintf(f, "%-15d %-15d %-15d %-15d %-15d %-15d %-15d %-15d\n",
            id, server, arrival_time, pop_time, finished_time, response_time,
            process_time, queue_len);
    fclose(f);
}

void calculate_statistics(int INIT_TIME, int FIN_TIME)
{
    FILE *f;
    f = fopen("./log_statistics.txt", "a+");

    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(EXIT_FAILURE);
    }

    fseek(f, 0, SEEK_SET);
    fscanf(f, "%*s %*s %*s %*s %*s %*s %*s %*s");

    int server;
    int jobs_cpu = 0;
    int jobs_disk1 = 0;
    int jobs_disk2 = 0;
    int finished;

    int max_cpu_queue = 0;
    int wcpu_queue = 0;
    int stIntCPU = 0;
    int temp_len_cpu = 0;
    int prev_len_cpu = 0;
    int cpu_utilization = 0;
    int cpu_response = 0;
    int max_cpu_response = 0;
    int cpu_holder1 = 0;
    int cpu_holder2 = 0;

    int max_disk1_queue = 0;
    int wdisk1_queue = 0;
    int stIntDisk1 = 0;
    int temp_len_disk1 = 0;
    int prev_len_disk1 = 0;
    int disk1_utilization = 0;
    int disk1_response = 0;
    int max_disk1_response = 0;
    int disk1_holder1 = 0;
    int disk1_holder2 = 0;

    int max_disk2_queue = 0;
    int wdisk2_queue = 0;
    int stIntDisk2 = 0;
    int temp_len_disk2 = 0;
    int prev_len_disk2 = 0;
    int disk2_utilization = 0;
    int disk2_response = 0;
    int max_disk2_response = 0;
    int disk2_holder1 = 0;
    int disk2_holder2 = 0;

    while (fscanf(f, "%*d %d %*d %*d", &server) == 1)
    {
        if (server == 0)
        {
            fscanf(f, "%d %d %d %d", &finished, &cpu_holder1, &cpu_holder2, &temp_len_cpu);

            if (finished > FIN_TIME)
            {
                cpu_holder1 -= (finished - FIN_TIME);
                cpu_holder2 -= (finished - FIN_TIME);
                finished -= (finished - FIN_TIME);
            }

            jobs_cpu += 1;
            cpu_response += cpu_holder1;
            cpu_utilization += cpu_holder2;
            if (cpu_holder1 > max_cpu_response)
            {
                max_cpu_response = cpu_holder1;
            }
            if (prev_len_cpu != temp_len_cpu)
            {
                if (stIntCPU == 0)
                {
                    stIntCPU = finished;
                }
                else
                {
                    wcpu_queue += (finished - stIntCPU) * prev_len_cpu;
                    stIntCPU = finished;
                }
            }
            prev_len_cpu = temp_len_cpu;
            if (temp_len_cpu > max_cpu_queue)
            {
                max_cpu_queue = temp_len_cpu;
            }
        }
        else if (server == 1)
        {
            fscanf(f, "%d %d %d %d", &finished, &disk1_holder1, &disk1_holder2, &temp_len_disk1);

            if (finished > FIN_TIME)
            {
                disk1_holder1 -= (finished - FIN_TIME);
                disk2_holder2 -= (finished - FIN_TIME);
            }

            jobs_disk1 += 1;
            disk1_response += disk1_holder1;
            disk1_utilization += disk1_holder2;
            if (disk1_holder1 > max_disk1_response)
            {
                max_disk1_response = disk1_holder1;
            }
            if (prev_len_disk1 != temp_len_disk1)
            {
                if (stIntDisk1 == 0)
                {
                    stIntDisk1 = finished;
                }
                else
                {
                    wdisk1_queue += (finished - stIntDisk1) * prev_len_cpu;
                    stIntDisk1 = finished;
                }
            }
            prev_len_disk1 = temp_len_disk1;
            if (temp_len_disk1 > max_disk1_queue)
            {
                max_disk1_queue = temp_len_disk1;
            }
        }
        else
        {
            fscanf(f, "%d %d %d %d", &finished, &disk2_holder1, &disk2_holder2, &temp_len_disk2);

            if (finished > FIN_TIME)
            {
                disk2_holder1 -= (finished - FIN_TIME);
                disk2_holder2 -= (finished - FIN_TIME);
            }

            jobs_disk2 += 1;
            disk2_response += disk2_holder1;
            disk2_utilization += disk2_holder2;
            if (disk2_holder1 > max_disk2_response)
            {
                max_disk2_response = disk2_holder1;
            }
            if (prev_len_disk2 != temp_len_disk2)
            {
                if (stIntDisk2 == 0)
                {
                    stIntDisk2 = finished;
                }
                else
                {
                    wdisk2_queue += (finished - stIntDisk2) * prev_len_cpu;
                    stIntDisk2 = finished;
                }
            }
            prev_len_disk2 = temp_len_disk2;
            if (temp_len_disk2 > max_disk2_queue)
            {
                max_disk2_queue = temp_len_disk2;
            }
        }
    }
    wcpu_queue += (FIN_TIME - stIntCPU) * prev_len_cpu;
    wdisk1_queue +=  (FIN_TIME - stIntDisk1) * prev_len_disk1;
    wdisk2_queue += (FIN_TIME - stIntDisk2) * prev_len_disk2;

    fprintf(f, "\n");
    fprintf(f, "CPU utilization: %.2f\n", (1.00 * cpu_utilization) / (FIN_TIME - INIT_TIME));
    fprintf(f, "Average CPU response: %2.f\n", ((1.00 * cpu_response) / jobs_cpu));
    fprintf(f, "Max CPU response: %d\n", max_cpu_response);
    fprintf(f, "Max CPU queue length: %d\n", max_cpu_queue);
    fprintf(f, "Weighted Average CPU queue length: %.2f\n", (1.00 * wcpu_queue) / (FIN_TIME - INIT_TIME));
    fprintf(f, "Jobs finished in CPU per one unit of time: %.2f\n", (1.00 * jobs_cpu) / (FIN_TIME - INIT_TIME));

    fprintf(f, "\n");
    fprintf(f, "Disk 1 utilization: %.2f\n", (1.00 * disk1_utilization) / (FIN_TIME - INIT_TIME));
    fprintf(f, "Average Disk 1 response: %.2f\n", ((1.00 * disk1_response) / jobs_disk1));
    fprintf(f, "Max Disk 1 response: %d\n", max_disk1_response);
    fprintf(f, "Max Disk 1 queue: %d\n", max_disk1_queue);
    fprintf(f, "Weighted Average Disk 1 queue length: %.2f\n", (1.00 * wdisk1_queue) / (FIN_TIME - INIT_TIME));
    fprintf(f, "Jobs finished in disk 1 per one unit of time: %.2f\n", (1.00 * jobs_disk1) / (FIN_TIME - INIT_TIME));

    fprintf(f, "\n");
    fprintf(f, "Disk 2 utilization: %.2f\n", (1.00 * disk2_utilization) / (FIN_TIME - INIT_TIME));
    fprintf(f, "Average Disk 2 response: %.2f\n", ((1.00 * disk2_response) / jobs_disk2));
    fprintf(f, "Max Disk 2 response: %d\n", max_disk2_response);
    fprintf(f, "Max Disk 2 queue: %d\n", max_disk2_queue);
    fprintf(f, "Weighted Average Disk 2 queue length: %.2f\n", (1.00 * wdisk2_queue) / (FIN_TIME - INIT_TIME));
    fprintf(f, "Jobs finished in disk 2 per one unit of time: %.2f\n", (1.00 * jobs_disk2) / (FIN_TIME - INIT_TIME));

    fclose(f);
    exit(EXIT_SUCCESS);
}
