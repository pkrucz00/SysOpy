#include "common.h"

// Na później
//    int temp_shm_id = get_shared_memory();
//    oven_and_table *test = shmat(temp_shm_id, NULL, 0);
//     printf("%d\n", test->table_free_slot_index);
//     test->table_free_slot_index++;
//    shmdt(test);

int get_pizza_from_table(int sem_id, int shm_id){
    sem_wait(sem_id, TABLE_EMPTY_COUNT);
    sem_wait(sem_id, IS_TABLE_USED);

    oven_and_table* o_and_t = save_shm_attach(shm_id);
    int index = o_and_t->table_cold_pizza_index;
    
    o_and_t->table_cold_pizza_index = (index+1)%MAX_TABLE_SIZE;
    
    int pizza_type = o_and_t->table[index];
    o_and_t->table[index] = -1;

    o_and_t->table_no_pizzas--;
    int no_pizzas_on_table = o_and_t->table_no_pizzas;

    shmdt(o_and_t);
    sem_signal(sem_id, IS_TABLE_USED);
    sem_signal(sem_id, TABLE_FULL_COUNT);

    print_stamp(getpid());
    printf(" Pobieram pizze: %d. Liczba pizz na stole: %d\n", pizza_type, no_pizzas_on_table);

    return pizza_type;
}

void deliver_pizza(int pizza_type, int sem_id, int shm_id){
    print_stamp(getpid());
    printf(" Dostarczam pizze %d\n", pizza_type);
}

int main(){
    srand(getpid());
    int sem_id = get_semaphore();
    int shm_id = get_shared_memory();

    while(1){
        int pizza_type = get_pizza_from_table(sem_id, shm_id);
        sleep(rand_range(4, 5));
        deliver_pizza(pizza_type, sem_id, shm_id);
        sleep(rand_range(4, 5));
    }
}