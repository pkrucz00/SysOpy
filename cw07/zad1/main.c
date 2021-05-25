#include "common.h"

pid_t cooks[MAX_COOKS_NUMBER];
pid_t deliverers[MAX_DELIVERERS_NUMBER];
int no_cooks;
int no_deliverers;
int sem_id;
int shm_id;


//handlers
void before_closing(){    
    for (int i = 0; i < no_cooks; i++){
        kill(cooks[i], SIGINT);
    }
    for (int i = 0; i < no_deliverers; i++){
        kill(deliverers[i], SIGINT);
    }

    if (semctl(sem_id, 0, IPC_RMID) == -1){
        perror("Error: problem with removing semaphores\n");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shm_id, IPC_RMID, NULL) == -1){
        perror("Error: problem with removing shared memory\n");
        exit(EXIT_FAILURE);
    }
}

void sigint_handler(int signal){
    exit(0);
}

//inits

void init_sems(){
    key_t sem_key = ftok(getenv(HOME_PATH), 0);
    sem_id = semget(sem_key, 5, IPC_CREAT|IPC_EXCL|0666);
    if (sem_id == -1){
        perror("Problem with getting semaphore.\n");
        exit(EXIT_FAILURE);
    }
    
    sem_init_value(sem_id, IS_OVEN_USED, 1);
    sem_init_value(sem_id, OVEN_FULL_COUNT, MAX_OVEN_SIZE);

    sem_init_value(sem_id, IS_TABLE_USED, 1);
    sem_init_value(sem_id, TABLE_EMPTY_COUNT, 0);
    sem_init_value(sem_id, TABLE_FULL_COUNT, MAX_TABLE_SIZE);
}

void init_shm(){
    key_t shm_key = ftok(getenv(HOME_PATH), 1);
    shm_id = shmget(shm_key, sizeof(oven_and_table), IPC_CREAT|0666);
    if (shm_id == -1){
        perror("Problem with getting shared memory %d\n");
        exit(EXIT_FAILURE);
    }
    
    oven_and_table *o_and_t = shmat(shm_id, NULL, 0);
    if (o_and_t == NULL){
        perror("Error: problem with getting shared memory\n");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < MAX_OVEN_SIZE; i++){
        o_and_t->oven[i] = -1;
    }
    for (int i = 0; i < MAX_TABLE_SIZE; i++){
        o_and_t->table[i] = -1;
    }

    o_and_t->oven_old_pizza_index = 0;
    o_and_t->oven_no_pizzas = 0;
    o_and_t -> table_cold_pizza_index = 0;
    o_and_t->table_no_pizzas = 0;
    shmdt(o_and_t);

}


//forks
void fork_cook(int i){
    pid_t cook_pid = fork();
    if (cook_pid == 0){
        execlp("./cook", "cook", NULL);   
    }
    cooks[i] = cook_pid;
}

void fork_deliverer(int i){
    pid_t deliverer_pid = fork();
    if (deliverer_pid == 0){
        execl("./deliverer", "deliverer", NULL);
    }
    deliverers[i] = deliverer_pid;
}


int main(int argc, char** argv){
    if (argc != 3){
        perror("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    no_cooks = safe_atoi(argv[1]);
    no_deliverers = safe_atoi(argv[2]);
    if (no_cooks > MAX_COOKS_NUMBER || no_deliverers > MAX_DELIVERERS_NUMBER){
        perror("Wrong arguments\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Error: problem with setting signal handler\n");
        exit(EXIT_FAILURE);
    };
    if (atexit(before_closing) == -1){
        perror("Error: problem with setting atexit\n");
        exit(EXIT_FAILURE);
    }

    init_sems();
    init_shm();

    for (int i = 0; i < no_cooks; i++){
        fork_cook(i);
    }
    for (int j = 0; j < no_deliverers; j++){
        fork_deliverer(j);
    }

    while(wait(NULL) != -1);


    return 0;
}