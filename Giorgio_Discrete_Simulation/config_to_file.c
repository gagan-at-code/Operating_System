/*
author: Hoang Ho
configuration settings 
*/

#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *f = fopen("/Users/hoangho/Documents/document/CS/CIS3207/labs/lab_1/config.txt", "w+");
    if (f == NULL)
    {
        printf("%s", "Error opening file!\n");
        exit(1);
    }
    
    int SEED = 0;
    int INIT_TIME = 0;
    int FIN_TIME = 100; 
    int ARRIVE_MIN = 5;
    int ARRIVE_MAX = 15;
    float QUIT_PROB = 0.4;
    int CPU_MIN = 2;
    int CPU_MAX = 5;
    int DISK1_MIN = 6;
    int DISK1_MAX = 10;
    int DISK2_MIN = 5;
    int DISK2_MAX = 10;

    fprintf(f, "%d\n", SEED);
    fprintf(f, "%d\n", INIT_TIME);
    fprintf(f, "%d\n", FIN_TIME);
    fprintf(f, "%d\n", ARRIVE_MIN);
    fprintf(f, "%d\n", ARRIVE_MAX);
    fprintf(f, "%0.1f\n", QUIT_PROB);
    fprintf(f, "%d\n", CPU_MIN);
    fprintf(f, "%d\n", CPU_MAX);
    fprintf(f, "%d\n", DISK1_MIN);
    fprintf(f, "%d\n", DISK1_MAX);
    fprintf(f, "%d\n", DISK2_MIN);
    fprintf(f, "%d\n", DISK2_MAX);
    fclose(f);
}
