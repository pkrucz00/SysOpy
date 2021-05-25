#include "common.h"

// Na później
//    int temp_shm_id = get_shared_memory();
//    oven_and_table *test = shmat(temp_shm_id, NULL, 0);
//     printf("%d\n", test->table_free_slot_index);
//     test->table_free_slot_index++;
//    shmdt(test);

int prepare_pizza(){
    int pizza_type = rand_range(0, 9);
    sleep(rand_range(1,2));
    print_stamp(getpid());
    printf(" Przygotowuje pizze: %d\n", pizza_type);
    fflush(stdout);

    return pizza_type;
}

void bake_pizza(int pizza_type, int sem_id, int shm_id){
    sem_wait(sem_id, OVEN_FULL_COUNT);
    sem_wait(sem_id, IS_OVEN_USED);

    oven_and_table* o_and_t = save_shm_attach(shm_id);
    int index = (o_and_t->oven_old_pizza_index + o_and_t->oven_no_pizzas) % MAX_OVEN_SIZE;
    o_and_t->oven[index] = pizza_type;

    o_and_t->oven_no_pizzas++;
    int no_pizzas_in_oven = o_and_t->oven_no_pizzas;
    shmdt(o_and_t);

    
    sem_signal(sem_id, IS_OVEN_USED);

    print_stamp(getpid());
    printf(" Dodalem pizze: %d. Liczba pizz w piecu: %d\n", pizza_type, no_pizzas_in_oven);
    fflush(stdout);
}

void put_pizza_on_table(int sem_id, int shm_id){
    //getting pizza out of oven

    sem_wait(sem_id, IS_OVEN_USED);
    oven_and_table* o_and_t = save_shm_attach(shm_id);
    
    int index = o_and_t->oven_old_pizza_index;
    int pizza_type = o_and_t->oven[index];
    o_and_t->oven[index] = -1;   //getting pizza out of the oven
    
    o_and_t->oven_no_pizzas--;
    o_and_t->oven_old_pizza_index = (index + 1) % MAX_OVEN_SIZE;
    int no_pizzas_in_oven = o_and_t->oven_no_pizzas;
    
    shmdt(o_and_t);
    
    sem_signal(sem_id, IS_OVEN_USED);
    sem_signal(sem_id, OVEN_FULL_COUNT);
    
    //putting pizza on the table
    sem_wait(sem_id, TABLE_FULL_COUNT);
    sem_wait(sem_id, IS_TABLE_USED);
    
    o_and_t = save_shm_attach(shm_id);
    
    int tab_index = (o_and_t->table_cold_pizza_index + o_and_t->table_no_pizzas)%MAX_TABLE_SIZE;

    o_and_t->table[tab_index] = pizza_type;
    o_and_t->table_no_pizzas++;
    int no_pizzas_on_table = o_and_t->table_no_pizzas;

    shmdt(o_and_t);
    sem_signal(sem_id, IS_TABLE_USED);
    sem_signal(sem_id, TABLE_EMPTY_COUNT);

    print_stamp(getpid());
    printf(" Wyjmuje pizza: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
         pizza_type, no_pizzas_in_oven, no_pizzas_on_table);
}

int main(){
    srand(getpid());
    int sem_id = get_semaphore();
    int shm_id = get_shared_memory();

    while(1){
        int pizza_type = prepare_pizza();
        bake_pizza(pizza_type, sem_id, shm_id);
        sleep(rand_range(1,2));
        put_pizza_on_table(sem_id, shm_id);

    }
}