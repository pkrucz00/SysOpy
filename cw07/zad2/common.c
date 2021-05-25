#include "common.h"

oven_and_table* save_shm_attach(int shm_fd){
    oven_and_table *ptr = mmap(NULL, sizeof(oven_and_table), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == (void*) -1){
        perror("Error: problem with attaching the shared memory segment\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void safe_shm_dettach(oven_and_table* ont){
    if (munmap(ont, sizeof(oven_and_table)) == -1){
        perror("Problem with closing shared memory\n");
        exit(EXIT_FAILURE);
    }
}

//helpers
int rand_range(int a, int b){
    return random()%(b-a+1) + a;
}

void print_stamp(pid_t pid){
    struct timeval currTime;
    if (gettimeofday(&currTime, NULL) == -1){
        perror("Error: problem with getting the time\n");
        exit(EXIT_FAILURE);
    }

    printf("(pid: %d; time: %ld [s] %ld [ms])", pid, currTime.tv_sec%1000, currTime.tv_usec/1000);
}


int safe_atoi(char* buff){
    char* endptr;
    int result = strtol(buff, &endptr, 10);
    if (*endptr){
        perror("Invalid argument\n");
        exit(EXIT_FAILURE);
    }
    return result;
}

//debug

void print_shm(int shm_id){
    oven_and_table *test = save_shm_attach(shm_id);

    printf("Oven: ");
    for (int i = 0; i < MAX_OVEN_SIZE; i++){
        printf("%2d ", test->oven[i]);
    }
    printf("\nOld index: %d\n", test->oven_old_pizza_index);
    printf("No. pizzas %d\n", test->oven_no_pizzas);
    printf("Table: ");
    for (int i = 0; i < MAX_TABLE_SIZE; i++){
        printf("%2d ", test-> table[i]);
    }
    printf("\nCold pizza: %d\n", test->table_cold_pizza_index);
    printf("No. pizzas %d\n", test->table_no_pizzas);
    safe_shm_dettach(test);
    fflush(stdout);
}
