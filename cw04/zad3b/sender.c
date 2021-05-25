#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>

#define SIG1(SIG_TYPE) (strcmp(SIG_TYPE, "sigrt") == 0 ? SIGRTMIN : SIGUSR1)
#define SIG2(SIG_TYPE) (strcmp(SIG_TYPE, "sigrt") == 0 ? SIGRTMIN + 1 : SIGUSR2)

int no_of_replies = 0;
int last_reply = 0;


void sigusr1_handler(int _, siginfo_t *__, void *___){
    no_of_replies += 1;
}

void sigusr1_sigqueue_handler(int _, siginfo_t *info, void *__){
    printf("Signal number %d has been recived\n", info->si_value.sival_int);
    no_of_replies += 1;
}

void sigusr2_handler(int _, siginfo_t *__, void *___){
    last_reply = 1;
}

int main(int argc, char **argv){
    if (argc != 4){
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = atoi(argv[1]);
    int no_of_signals = atoi(argv[2]);
    char* mode = strdup(argv[3]);

    struct sigaction sigusr1_act, sigusr2_act;

    sigusr1_act.sa_flags = sigusr2_act.sa_flags = SA_SIGINFO;
    sigusr1_act.sa_sigaction = ((strcmp(mode, "sigqueue") == 0) ? sigusr1_sigqueue_handler : sigusr1_handler);
    sigusr2_act.sa_sigaction = sigusr2_handler;

    sigaction(SIG1(mode), &sigusr1_act, NULL);
    sigaction(SIG2(mode), &sigusr2_act, NULL);

    sigset_t block_mask;
    sigfillset(&block_mask);
    sigdelset(&block_mask, SIG1(mode));
    sigdelset(&block_mask, SIG2(mode));
    sigprocmask(SIG_SETMASK, &block_mask, NULL);

    union sigval sval;    //for sigqueue

    for (int i = 0; i < no_of_signals; i++){
        if (strcmp(mode, "sigqueue") == 0){
            sval.sival_int = i;
            sigqueue(pid, SIG1(mode), sval);
        } else {
            kill(pid, SIG1(mode));
        }
        while (no_of_replies - 1 < i);
    }

    if (strcmp(mode, "sigqueue") == 0){ 
        sval.sival_int = (int) getpid();
        sigqueue(pid, SIG2(mode), sval);
    }else{
        kill(pid, SIG2(mode));
    }

    while (!last_reply);

    printf("Number of sent signals: %d\nNumber of recived signals: %d\n", no_of_signals, no_of_replies);
}