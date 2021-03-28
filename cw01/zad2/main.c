#include "library.h"
#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

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



int main(int argc, char **argv) {
    main_table* table = NULL;
    
    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];

        if (strcmp(arg, "create_table") == 0){
            if (table != NULL){     //if table already exists, overwrite it 
                delete_table(table);
            }
            int size = atoi(argv[++i]);
            table = create_main_table(size);   
        }
        else if (strcmp(arg, "merge_files") == 0){
            if (table == NULL)
                exit(EXIT_FAILURE);
            
            int size = atoi(argv[++i]); 
            for (int j = 0; j < size; j++){
                if (table -> current_size < table -> max_size){
                    block* new_block = define_sequence(argv[++i]);
                    FILE* tmp = merge(new_block);
                    pin_new_block(table, new_block, tmp);
                } else {
                    printf("Main table is too small. Please create bigger table\n");
                }
            }
        }
        else if (strcmp(arg, "remove_row") == 0){
            int block_ind = atoi(argv[++i]);
            int row_ind = atoi(argv[++i]);
            delete_line(table, block_ind, row_ind);
        }
        else if (strcmp(arg, "remove_block") == 0){
            int block_ind = atoi(argv[i++]);
            delete_block(table, block_ind);
        }
        else if (strcmp(arg, "remove_table") == 0){
            delete_table(table);
            table = NULL;   
        }
        else if (strcmp(arg, "print_merged_files") == 0){
            print_merged_files(table);
        }
        else if (strcmp(arg, "start_measurement") == 0){
            start_measurement();
        }
        else if (strcmp(arg, "end_measurement") == 0){
            char* measurement_name = argv[++i];
            end_and_print_measurement(measurement_name);
        }
        else if (strcmp(arg, "print_header") == 0){
            print_header();
        }
        else{
            printf("Unknown command %s\n", arg);
        }
    }

    if (table != NULL){   
        //delete whole table if the user forgot to
        delete_table(table);
    }
    return 0;
}
