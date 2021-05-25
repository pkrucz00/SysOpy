#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>


#define PIPE_NAME "pipe"
#define MAX_PRODUCERS 5
#define MAX_NO_CHARS 100

#define PRODUCER "./producer"
#define CONSUMER "./consumer"
#define OUT_FILE "output"


char* itoa(int value) {
    static char buff[6];
    sprintf(buff, "%i", value);
    return strdup(buff);
}

int single_test(int no_producers, int no_consumers, int no_chars) {
    //running the problem
    char input_path[7];
    char* no_chars_str = itoa(no_chars);

    for (int i = 0; i < no_producers; i++){
        sprintf(input_path, "input%d", i);
        char* i_str = itoa(i);
        if (fork() == 0) {
            execl(PRODUCER, PRODUCER, PIPE_NAME, i_str, input_path, no_chars_str, NULL);
        }
        free(i_str);
    }

    for (int i = 0; i < no_consumers; i++){
        if (fork() == 0) {
            execl(CONSUMER, CONSUMER, PIPE_NAME, OUT_FILE, no_chars_str, NULL);
        }
    }

    free(no_chars_str);
    while(wait(NULL) != -1);

    //testing output
    FILE* out = fopen(OUT_FILE, "r");    
    char buff;
    int i = 0;
    
    while(fread(&buff, 1, 1, out) > 0){
        if (ferror(out) != 0){
            perror("Cannot create open out file\n");
            exit(EXIT_FAILURE);
        }
        char c = 'A' + i;
        if (buff == '\n'){
            i++;
        } else if (buff != c) {
            return false;   //test unsuccessful
        }
    }

    fclose(out);
    return true;  //test succesful

}

int main(){   
    if (mkfifo(PIPE_NAME, 0666) < 0){
        perror("Cannot create named pipe\n");
        exit(EXIT_FAILURE);
    }

    char input_path[7];

    for (int i = 0; i < MAX_PRODUCERS; i++) {
        sprintf(input_path, "input%d", i);

        FILE* file = fopen(input_path, "w");
        for (int j = 0; j < 5*MAX_NO_CHARS; j++){
            char c = 'A' + i;
            fwrite(&c, 1, 1, file);
        }
    }

    int no_of_lines[] = {5, 25, MAX_NO_CHARS};
    int no_of_producers[] = {1, 5, 1, 5};
    int no_of_consumers[] = {1, 1, 5, 5};

    for (int i = 0; i < 3; i++){
        printf("N = %d\n", no_of_lines[i]);
        for (int j = 0; j < 4; j++){
            printf("Producents = %d\nConsumers = %d\n", no_of_producers[j], no_of_consumers[j]);
            if (single_test(no_of_producers[j], no_of_consumers[j], no_of_lines[i])){
                printf("Success\n\n");
            } else {
                printf("Failure\n\n");
            }
        }
    }

    system("rm pipe");
}