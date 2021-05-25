#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <wait.h>


#define MAX_DELIVERERS_NUMBER 30
#define MAX_COOKS_NUMBER 30

#define HOME_PATH "HOME"

#define MAX_OVEN_SIZE 10
#define MAX_TABLE_SIZE 10

#define IS_OVEN_USED 0      
#define OVEN_FULL_COUNT 1   //stops process if the oven is full
#define IS_TABLE_USED 2     
#define TABLE_EMPTY_COUNT 3 //stops process if the table is empty
#define TABLE_FULL_COUNT 4  //stops process if the table is full

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

typedef struct oven_and_table{
    int oven[MAX_OVEN_SIZE];
    int table[MAX_TABLE_SIZE];

    int oven_old_pizza_index;       //pizza thats been in the oven the longest
    int oven_no_pizzas;

    int table_cold_pizza_index;     //pizzza taken by the deliverer
    int table_no_pizzas;
} oven_and_table;

int get_semaphore();
int get_shared_memory();

void sem_wait(int sem_id, int sem_num);
void sem_signal(int sem_id, int sem_num);
void sem_init_value(int sem_id, int sem_num, int value);

oven_and_table* save_shm_attach(int shm_id);

//helpers
int rand_range(int a, int b);
void print_stamp(pid_t pid);
int safe_atoi(char* buff);

//do usuniecia
void print_sems(int sem_id);
void print_shm(int shm_id);

#endif //COMMON_H