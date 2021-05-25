#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>

#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <wait.h>


#define MAX_DELIVERERS_NUMBER 30
#define MAX_COOKS_NUMBER 30

#define HOME_PATH "HOME"
#define SHARED_MEMORY "/SHARED_MEMORY"

#define MAX_OVEN_SIZE 10
#define MAX_TABLE_SIZE 10

#define NO_SEMAPHORES 5

#define IS_OVEN_USED 0      
#define OVEN_FULL_COUNT 1   //stops process if the oven is full
#define IS_TABLE_USED 2     
#define TABLE_EMPTY_COUNT 3 //stops process if the table is empty
#define TABLE_FULL_COUNT 4  //stops process if the table is full



typedef struct oven_and_table{
    int oven[MAX_OVEN_SIZE];
    int table[MAX_TABLE_SIZE];

    int oven_old_pizza_index;       //pizza thats been in the oven the longest
    int oven_no_pizzas;

    int table_cold_pizza_index;     //pizzza taken by the deliverer
    int table_no_pizzas;
} oven_and_table;

oven_and_table* save_shm_attach(int shm_fd);
void safe_shm_dettach(oven_and_table* ont);

//helpers
int rand_range(int a, int b);
void print_stamp(pid_t pid);
int safe_atoi(char* buff);

//debug
void print_shm(int shm_id);

#endif //COMMON_H