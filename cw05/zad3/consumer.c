#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/file.h>

#include <unistd.h>
#include <fcntl.h>

#include <limits.h>

#include "vector.h"

#define MAX_NO_CHARS  100
#define MAX_PRODUCERS 5

typedef vector vec_char;            //vector<char*>
typedef vector vec_of_vec_char;     //vector<vector<char*>>

// void seek_ith_new_line(int fd, int i){  
//     lseek(fd, 0, SEEK_SET);
//     char c;
//     int acc = 0;
//     while(acc != i && read(fd, &c, 1) == 1 && c != '\n'){
//         acc++;
//     }
// }

void free_vec_char_content(vec_char *v) {
    while (v->size > 0) {
        free(vec_pop_back(v));
    }
}

int main(int argc, char** argv){
    if (argc != 4) {
        perror("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    //reading args
    FILE* fifo = fopen(argv[1], "r");
    int out = open(argv[2], O_WRONLY|O_CREAT, 0666);  //unfortunetely there is no flock function for FILE streams AFAIK
    flock(out, LOCK_SH);                //many processes write to the same file
    int len = atoi(argv[3]);

    ///vec initialization
    vec_char *message[MAX_PRODUCERS];
    for (int i = 0; i < MAX_PRODUCERS; i++){
        message[i] = malloc(sizeof(*message));
        vec_init(message[i]);
    }

    //from pipe to vector
    char buffer[MAX_NO_CHARS + 1];
    int no_line;
    while(fscanf(fifo, "%d|%s\n", &no_line, buffer) != EOF){
        vec_push_back(message[no_line], strdup(buffer));
        printf("%s\n", buffer);
    }

    //from vector to out file
    for (int i = 0; i < MAX_PRODUCERS; i++){
        for (int j = 0; j < message[i]->size; j++){
            write(out, message[i]->storage[j], len);
        }
        write(out, "\n", 1);
    }

    //freeing
    fclose(fifo);
    close(out);
    for (int i = 0; i < 5; i++){
        free_vec_char_content(message[i]);
    }
}
    
