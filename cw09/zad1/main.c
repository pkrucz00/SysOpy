#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <time.h>
#include <unistd.h>

#define NO_REINDEERS 9
#define NO_ELVES 10
#define ELVES_TRIGGER 3
#define MAX_DELIVERED_GIFTS 3


int no_idle_reindeers = 0;
int no_waiting_elves = 0;
int waiting_elves_ids[ELVES_TRIGGER];

int let_reindeers_go = 1;

pthread_mutex_t santa_sleeping_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_sleeping_condition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t reindeer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reindeer_wait_condition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t elf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elf_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t elf_wait_condition = PTHREAD_COND_INITIALIZER;



//helpers
int rand_range(int a, int b){
    return random()%(b-a+1) + a;
}

void clean_threads(pthread_t* santa_thread, pthread_t** reindeer_threads, pthread_t** elves_threads, int* reindeerIDs, int* elvesIDs){
    pthread_join(*santa_thread, NULL);
    for (int i = 0; i < NO_REINDEERS; i++){
        pthread_join(*reindeer_threads[i], NULL);
    }
    for (int i = 0; i < NO_ELVES; i++){
        pthread_join(*elves_threads[i], NULL);       
    }

    free(reindeerIDs);
    free(reindeer_threads);
    free(elvesIDs);
    free(elves_threads);

};

//main functions
void* santa(void* arg){
    srand(time(NULL));
    int delivered_gifts = 0;

    while (delivered_gifts < MAX_DELIVERED_GIFTS){
        //sleeping
        pthread_mutex_lock(&santa_sleeping_mutex);
        while (no_idle_reindeers < NO_REINDEERS && no_waiting_elves < ELVES_TRIGGER){
            printf("Mikolaj: spie\n");
            pthread_cond_wait(&santa_sleeping_condition, &santa_sleeping_mutex);
        }
        pthread_mutex_unlock(&santa_sleeping_mutex);

        printf("Mikolaj: budze sie\n");
        //sending reindeers
        pthread_mutex_lock(&reindeer_mutex);
        if (no_idle_reindeers == NO_REINDEERS){
            printf("Mikolaj: dostarczam zabawki\n");
            sleep(rand_range(2, 4));
            delivered_gifts += 1;
            no_idle_reindeers = 0;

            //notifying reindeers
            pthread_mutex_lock(&reindeer_wait_mutex);
            let_reindeers_go = 1;
            pthread_cond_broadcast(&reindeer_wait_condition);
            pthread_mutex_unlock(&reindeer_wait_mutex);
        }
        pthread_mutex_unlock(&reindeer_mutex);

        pthread_mutex_lock(&elf_mutex);
        if (no_waiting_elves == ELVES_TRIGGER){
            pthread_mutex_lock(&elf_wait_mutex);
            printf("Mikolaj: rozwiazuje problemy elfow ");
            for (int i = 0; i < ELVES_TRIGGER; i++){
                printf("%d ", waiting_elves_ids[i]);
                waiting_elves_ids[i] = -1;
            }
            printf("\n");

            sleep(rand_range(1,2));
            no_waiting_elves = 0;
            pthread_cond_broadcast(&elf_wait_condition);
            pthread_mutex_unlock(&elf_wait_mutex);
        }
        pthread_mutex_unlock(&elf_mutex);

        printf("Mikolaj: zasypiam\n");
    }
    exit(0);
}

void* reindeer(void *arg){
    int ID = *((int *) arg);
    srand(ID);

    while(1){
 
        
        pthread_mutex_lock(&reindeer_wait_mutex);
        while (!let_reindeers_go){
            pthread_cond_wait(&reindeer_wait_condition, &reindeer_wait_mutex);
        }
        pthread_mutex_unlock(&reindeer_wait_mutex);

        sleep(rand_range(5,10));

        pthread_mutex_lock(&reindeer_mutex);
        no_idle_reindeers += 1;
        printf("Renifer: czeka %d reniferow na Mikolaja, %d\n", no_idle_reindeers, ID);
        let_reindeers_go = 0;

        if (no_idle_reindeers == NO_REINDEERS){
            printf("Renifer: wybudzam Mikolaja, %d\n", ID);
            pthread_mutex_lock(&santa_sleeping_mutex);
            pthread_cond_broadcast(&santa_sleeping_condition);
            pthread_mutex_unlock(&santa_sleeping_mutex);
        }
        pthread_mutex_unlock(&reindeer_mutex);
        
        sleep(rand_range(2, 4));
    }
}

void* elf(void *arg){
    int ID = *((int *) arg);
    srand(ID);

    while(1){
        sleep(rand_range(2,5));

        pthread_mutex_lock(&elf_wait_mutex);
        while (no_waiting_elves == ELVES_TRIGGER){
            printf("Elf: czeka na powrót elfow, %d\n", ID);
            pthread_cond_wait(&elf_wait_condition, &elf_wait_mutex);
        }
        pthread_mutex_unlock(&elf_wait_mutex);

        pthread_mutex_lock(&elf_mutex);
        if (no_waiting_elves < ELVES_TRIGGER){
            waiting_elves_ids[no_waiting_elves] = ID;
            no_waiting_elves += 1;
            printf("Elf: czeka %d elfow na Mikolaja, %d\n", no_waiting_elves, ID);

            if (no_waiting_elves == ELVES_TRIGGER){
                printf("Elf: wybudzam Mikolaja, %d\n", ID);
                pthread_mutex_lock(&santa_sleeping_mutex);
                pthread_cond_broadcast(&santa_sleeping_condition);
                pthread_mutex_unlock(&santa_sleeping_mutex);
            }
        }
        pthread_mutex_unlock(&elf_mutex);
    }

}



int main(){
    
    //default value for waiting elves table
    for (int i = 0; i < ELVES_TRIGGER; i++){
        waiting_elves_ids[i] = -1;
    }

    //necessary allocs
    int* reindeerIDs = calloc(NO_REINDEERS, sizeof(int));
    pthread_t* reindeer_threads = calloc(NO_REINDEERS, sizeof(pthread_t));

    int* elvesIDs = calloc(NO_ELVES, sizeof(int));
    pthread_t* elves_threads = calloc(NO_ELVES, sizeof(pthread_t));

    //santa thread
    pthread_t santa_thread;
    if (pthread_create(&santa_thread, NULL, *santa, NULL) != 0){
        clean_threads(&santa_thread, &reindeer_threads, &elves_threads, reindeerIDs, elvesIDs);
        perror("Problem with creating santa thread\n");
        exit(EXIT_FAILURE);
    }

    //reindeers thread
    for (int i = 0; i < NO_REINDEERS; i++){
        reindeerIDs[i] = i;
        if (pthread_create(&reindeer_threads[i], NULL, &reindeer, &reindeerIDs[i]) != 0){
            clean_threads(&santa_thread, &reindeer_threads, &elves_threads, reindeerIDs, elvesIDs);
            perror("Problem with creating reindeer threads\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // starting elf threads

    for (int i = 0; i < NO_ELVES; i++){
        elvesIDs[i] = i;
        if (pthread_create(&elves_threads[i], NULL, &elf, &elvesIDs[i]) != 0){
            clean_threads(&santa_thread, &reindeer_threads, &elves_threads, reindeerIDs, elvesIDs);
            perror("Problem with creating elves threads\n");
            exit(EXIT_FAILURE);
        }
    }

    clean_threads(&santa_thread, &reindeer_threads, &elves_threads, reindeerIDs, elvesIDs);
    return 0;
}