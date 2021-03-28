#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argv, char** argc) {
    if (argv != 2){
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }
    
    int n = atoi(argc[1]);
    pid_t child_p;
    for (int i = 0; i < n; i++){
        child_p = fork();
        if (child_p == 0) {
            printf("Hello there! I'm process number %d\n", (int) getpid());
            exit(EXIT_SUCCESS);
        }
        wait(NULL);
    }

    return 0;
}