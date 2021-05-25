#include "common.h"

int get_semaphore(){
    key_t sem_key = ftok(getenv(HOME_PATH), 0);
    int sem_id = semget(sem_key, 0, 0);
    if (sem_id == -1){
        perror("Problem with getting semaphore.\n");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

int get_shared_memory()
{
    key_t shm_key = ftok(getenv(HOME_PATH), 1);
    int shared_memory_id = shmget(shm_key, 0, 0);
    if (shared_memory_id == -1){
        perror("Problem with getting shared memory %d\n");
        exit(EXIT_FAILURE);
    }
    return shared_memory_id;
}


void sem_wait(int sem_id, int sem_num){
    struct sembuf buf = {sem_num, -1, 0};
    if (semop(sem_id, &buf, 1) == -1){
        perror("Error: problem with operating on semaphore (wait op)\n");
        exit(EXIT_FAILURE);
    }
}

void sem_signal(int sem_id, int sem_num){
    struct sembuf buf = {sem_num, 1, 0};
    if (semop(sem_id, &buf, 1) == -1){
        perror("Error: problem with operating on semaphore (signal op)\n");
        exit(EXIT_FAILURE);
    }
}


void sem_init_value(int sem_id, int sem_num, int value){
    union semun arg;
    arg.val = value;

    if (semctl(sem_id, sem_num, SETVAL, arg) == -1){
        perror("Error: problem with initialazing semaphore value\n");
        exit(EXIT_FAILURE);
    }
}

oven_and_table* save_shm_attach(int shm_id){
    oven_and_table *ptr = shmat(shm_id, NULL, 0);
    if (ptr == (void*) -1){
        perror("Error: problem with attaching the shared memory segment\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
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


void print_sems(int sem_id){
    for (int i = 0; i < 5; i++){
        int sem_val = semctl(sem_id, i, GETVAL);
        if (sem_val == -1){
            perror("Something's not yes\n");
            exit(EXIT_FAILURE);
        }
        printf("Wartosc semafora %d: %d\n", i, semctl(sem_id, i, GETVAL));
    }
}

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

    shmdt(test);
    fflush(stdout);
}
