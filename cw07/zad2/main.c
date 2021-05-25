#include "common.h"

pid_t cooks[MAX_COOKS_NUMBER];
pid_t deliverers[MAX_DELIVERERS_NUMBER];
int no_cooks;
int no_deliverers;
int shm_fd;

const char* sem_names[NO_SEMAPHORES] = {"/IS_OVEN_USED", "/OVEN_FULL_COUNT","/IS_TABLE_USED", "/TABLE_EMPTY_COUNT", "/TABLE_FULL_COUNT"};


//handlers
void before_closing(){    
    for (int i = 0; i < no_cooks; i++){
        kill(cooks[i], SIGINT);
    }
    for (int i = 0; i < no_deliverers; i++){
        kill(deliverers[i], SIGINT);
    }

    for (int i = 0; i < NO_SEMAPHORES; i++){
        if (sem_unlink(sem_names[i]) == -1){
            perror("Error: problem with removing semaphores\n");
            exit(EXIT_FAILURE);
        }
    }
    if (shm_unlink(SHARED_MEMORY) == -1){
        perror("Error: problem with removing shared memory\n");
        exit(EXIT_FAILURE);
    }
}

void sigint_handler(int signal){
    exit(0);
}

//inits

void init_sems(){
    int init_values[NO_SEMAPHORES] = {1, MAX_OVEN_SIZE, 1, 0, MAX_TABLE_SIZE};
    
    for (int i = 0; i < NO_SEMAPHORES; i++){
        sem_t* sem = sem_open(sem_names[i], O_RDWR|O_CREAT|O_EXCL, 0666, init_values[i]);
        if (sem == SEM_FAILED){
            perror("Error: problem with creating the semaphore\n");
            exit(EXIT_FAILURE);
        }
        sem_close(sem);
    }
    
}

void init_shm(){
    shm_fd = shm_open(SHARED_MEMORY, O_RDWR|O_CREAT|O_EXCL, 0666);
    if (shm_fd == -1){
        perror("Problem with getting shared memory\n");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, sizeof(oven_and_table)) == -1){
        perror("Error: problem with truncating shared memory");
        exit(EXIT_FAILURE);
    }
    
    oven_and_table *o_and_t = save_shm_attach(shm_fd);
    
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

    safe_shm_dettach(o_and_t);
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