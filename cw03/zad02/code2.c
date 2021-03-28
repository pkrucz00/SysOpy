#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>

clock_t start_time, end_time;
struct tms start_tms, end_tms;

void start_measurement(){
    start_time = times(&start_tms);
}

void end_and_print_measurement(char *name){
    end_time = times(&end_tms);
    int ticks_per_sec = sysconf(_SC_CLK_TCK);
    
    double real_time = (double)(end_time - start_time) / ticks_per_sec;
    double user_time = (double)(end_tms.tms_utime - start_tms.tms_utime) / ticks_per_sec;
    double sys_time = (double)(end_tms.tms_stime - start_tms.tms_stime) / ticks_per_sec;
    printf("%35s:\t%20f\t%20f\t%20f\n",
        name, real_time, user_time, sys_time);
}

void print_header(){
    printf("%35s:\t%20s\t%20s\t%20s\n", 
        "name", "real_time [s]", "user_time [s]", "sys_time [s]");
}


int main(int argc, char** argv){
    main_table* table = NULL;
    block* new_block = NULL;
    FILE* tmp = NULL;

    pid_t child_pd;
    start_measurement();
    for (int i = 1; i < argc-1; i++){
        child_pd = fork();
        if (child_pd == 0){
            table = create_main_table(1); //one table per pair of files
            new_block = define_sequence(argv[i]);
            tmp = merge(new_block);
            pin_new_block(table, new_block, tmp);
            
            delete_table(table);
            exit(EXIT_SUCCESS);
        }
    }

    while(wait(NULL) > 0);  //we wait untill all processes are consumed
    end_and_print_measurement(argv[argc-1]);
    return 0;
}