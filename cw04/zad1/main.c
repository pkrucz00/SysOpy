#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void handle(int sigint){
    printf("I have recived a signal. Have a good day!\n");
}

int main(int argc, char ** argv) {
    if (argc != 3){
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    sigset_t signal_mask;
    
    if (strcmp(argv[1], "ignore") == 0){
        signal(SIGUSR1, SIG_IGN);
    } else if (strcmp(argv[1], "handler") == 0){
        signal(SIGUSR1, handle);
    } else if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0){
        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    }

    raise(SIGUSR1);

    //checking pending signal 
    if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0){
        sigpending(&signal_mask);
        printf("Signal pending: %d\n", sigismember(&signal_mask, SIGUSR1));
    }

    if (strcmp(argv[2], "fork") == 0){
        pid_t child_pd = fork();

        if (child_pd == 0){
            if (strcmp(argv[1], "pending") != 0) {
                raise(SIGUSR1);
            }
            if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
                sigpending(&signal_mask);
                printf("Signal pending: %d\n", sigismember(&signal_mask, SIGUSR1));
            }           
            else if(strcmp(argv[1], "ignore") == 0) {
                struct sigaction act;
                sigaction(SIGUSR1, NULL, &act);
                printf("Signal ignored %d\n", act.sa_handler == SIG_IGN);
            }
        }
    } else if (strcmp(argv[2], "exec") == 0){
        execl("./exec", "./exec", argv[1], NULL);
    }
}