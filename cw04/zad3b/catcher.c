#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>

#define SIG1(SIG_TYPE) (strcmp(SIG_TYPE, "sigrt") == 0 ? SIGRTMIN : SIGUSR1)
#define SIG2(SIG_TYPE) (strcmp(SIG_TYPE, "sigrt") == 0 ? SIGRTMIN + 1 : SIGUSR2)

pid_t sender_pid = 0;
int no_of_recived_sigusr1;
int end = 0;

char* mode;

void sigusr1_handler(int _, siginfo_t *info, void *__){
    no_of_recived_sigusr1++;
    
    sender_pid = info->si_pid;
    
    if (strcmp(mode, "sigqueue")){
        union sigval sval;
        sval.sival_int = no_of_recived_sigusr1;
        sigqueue(sender_pid, SIG1(mode), sval);
    } else {
        kill(sender_pid, SIG1(mode));
    }
}

void sigusr2_handler(int _, siginfo_t *info, void *__) {
    sender_pid = info->si_pid;
    if (strcmp(mode, "sigqueue")){
        union sigval sval;
        sval.sival_int = no_of_recived_sigusr1;
        sigqueue(sender_pid, SIG2(mode), sval);
    } else {
        kill(sender_pid, SIG2(mode));
    }
    end = 1;
}

int main(int argc, char** argv){
    if (argc != 2){
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    mode = strdup(argv[1]);

    struct sigaction sigusr1_act, sigusr2_act;
    sigusr1_act.sa_flags = sigusr2_act.sa_flags = SA_SIGINFO;
    sigusr1_act.sa_sigaction = sigusr1_handler;
    sigusr2_act.sa_sigaction = sigusr2_handler;

    sigaction(SIG1(mode), &sigusr1_act, NULL);
    sigaction(SIG2(mode), &sigusr2_act, NULL);

    sigset_t block_mask;
    sigfillset(&block_mask);
    sigdelset(&block_mask, SIG1(mode));
    sigdelset(&block_mask, SIG2(mode));
    sigprocmask(SIG_SETMASK, &block_mask, NULL);

    printf("Catcher PID: %d\n", (int) getpid());

    while (sender_pid == 0);   //we are waiting for first signal

    while (end == 0);  //we are waiting for the last signal (sigusr2)

    printf("Number of signals recived by catcher: %d\n", no_of_recived_sigusr1);
}