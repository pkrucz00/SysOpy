#include "common.h"

const char* sem_names[NO_SEMAPHORES] = {"/IS_OVEN_USED", "/OVEN_FULL_COUNT","/IS_TABLE_USED", "/TABLE_EMPTY_COUNT", "/TABLE_FULL_COUNT"};
sem_t* sems[NO_SEMAPHORES];
int shm_fd;

//inits
sem_t* get_semaphore(int i){
    sem_t* sem = sem_open(sem_names[i], O_RDWR);
    if (sem == SEM_FAILED){
        perror("Error: problem with opening the semaphore\n");
        exit(EXIT_FAILURE);
    }
    return sem;
}

//cook behavior

int prepare_pizza(){
    int pizza_type = rand_range(0, 9);
    sleep(rand_range(1,2));
    print_stamp(getpid());
    printf(" Przygotowuje pizze: %d\n", pizza_type);
    fflush(stdout);

    return pizza_type;
}

void bake_pizza(int pizza_type){
    sem_wait(sems[OVEN_FULL_COUNT]);
    sem_wait(sems[IS_OVEN_USED]);
    oven_and_table* o_and_t = save_shm_attach(shm_fd);
    
    int index = (o_and_t->oven_old_pizza_index + o_and_t->oven_no_pizzas) % MAX_OVEN_SIZE;
    o_and_t->oven[index] = pizza_type;

    o_and_t->oven_no_pizzas++;
    int no_pizzas_in_oven = o_and_t->oven_no_pizzas;
    
    safe_shm_dettach(o_and_t);
    sem_post(sems[IS_OVEN_USED]);

    print_stamp(getpid());
    printf(" Dodalem pizze: %d. Liczba pizz w piecu: %d\n", pizza_type, no_pizzas_in_oven);
    fflush(stdout);
}

void put_pizza_on_table(){
    //getting pizza out of oven

    sem_wait(sems[IS_OVEN_USED]);
    oven_and_table* o_and_t = save_shm_attach(shm_fd);
    
    int index = o_and_t->oven_old_pizza_index;
    int pizza_type = o_and_t->oven[index];
    o_and_t->oven[index] = -1;   //getting pizza out of the oven
    
    o_and_t->oven_no_pizzas--;
    o_and_t->oven_old_pizza_index = (index + 1) % MAX_OVEN_SIZE;
    int no_pizzas_in_oven = o_and_t->oven_no_pizzas;
    
    safe_shm_dettach(o_and_t);
    if (pizza_type == -1){
        perror("ERROR: Wrong type of pizza was taken\n");
        exit(EXIT_FAILURE);
    }
    
    sem_post(sems[IS_OVEN_USED]);
    sem_post(sems[OVEN_FULL_COUNT]);
    
    //putting pizza on the table
    sem_wait(sems[TABLE_FULL_COUNT]);
    sem_wait(sems[IS_TABLE_USED]);
    
    o_and_t = save_shm_attach(shm_fd);
    
    int tab_index = (o_and_t->table_cold_pizza_index + o_and_t->table_no_pizzas)%MAX_TABLE_SIZE;

    o_and_t->table[tab_index] = pizza_type;
    o_and_t->table_no_pizzas++;
    int no_pizzas_on_table = o_and_t->table_no_pizzas;

    safe_shm_dettach(o_and_t);
    sem_post(sems[IS_TABLE_USED]);
    sem_post(sems[TABLE_EMPTY_COUNT]);

    print_stamp(getpid());
    printf(" Wyjmuje pizza: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
         pizza_type, no_pizzas_in_oven, no_pizzas_on_table);
}

int main(){
    srand(getpid());
    for (int i = 0; i < NO_SEMAPHORES; i++){
        sems[i] = get_semaphore(i);
    }
    if ( (shm_fd = shm_open(SHARED_MEMORY, O_RDWR, 0666)) == -1){
        perror("Error: Problem with opening shared memory\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        int pizza_type = prepare_pizza();
        bake_pizza(pizza_type);
        sleep(rand_range(1,2));
        put_pizza_on_table();

    }
}