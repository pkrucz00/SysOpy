#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


//floating point exeption
void fpe_handler(int sig, siginfo_t* info, void* ucontext){
    printf("Signal number: %d\t PID: %d\t Errno number: %d\n",
        info->si_signo,
        (int) info->si_pid,
        info->si_errno); 
    exit(0);
}

void sigsegv_handler(int sig, siginfo_t* info, void* ucontext){
    printf("Signal number: %d\t PID: %d\t Signal code: %d\n",
        info->si_signo,
        (int) info->si_pid,
        info->si_code);
    exit(0);
}

void sigabrt_handler(int sig, siginfo_t* info, void* ucontext){
    printf("Signal number: %d\t PID: %d\t Exit value of signal: %d\n",
        info->si_signo,
        (int) info->si_pid,
        info->si_status);
    exit(0);
}

void sigcont_handler(int sig){
    printf("Hello. I'm the continue signal handler. Have a nice day!\n");
}


int COUNTER = 3;
void recursive_handler(int sig){
    int rec_call_num = 4 - COUNTER;
    printf("Start recursive call number %d\n", rec_call_num);
    COUNTER -= 1;
    if (COUNTER){
        raise(sig);
    }

    printf(" Stop recursive call number %d\n", rec_call_num); //this will downcount with SA_NODEFER, upcount otherwise
}

int main(){
    pid_t child_pd;

    //checking the SA_SIGINFO flag
    child_pd = fork();
    if (child_pd == 0){
        struct sigaction act = {.sa_flags = SA_SIGINFO, .sa_sigaction=fpe_handler};
        sigaction(SIGFPE, &act, NULL);    
        
        //causes FPE
        int a = 0;
        int b = 1 / a;
        if (b);  //making -Wall happy
        
    }
    wait(NULL);

    child_pd = fork();
    if (child_pd == 0){
        struct sigaction act = {.sa_flags = SA_SIGINFO, .sa_sigaction=sigsegv_handler};
        sigaction(SIGSEGV, &act, NULL);    
        
        //causes seg fault
        raise(SIGSEGV);
    }
    wait(NULL);

    child_pd = fork();
    if (child_pd == 0){
        struct sigaction act = {.sa_flags = SA_SIGINFO, .sa_sigaction=sigabrt_handler};
        sigaction(SIGABRT, &act, NULL);    
        
        raise(SIGABRT);
    }
    wait(NULL);

    //checking the SA_RESETHAND flag
    child_pd = fork();
    if (child_pd == 0){
        struct sigaction act = {.sa_flags = SA_RESETHAND, .sa_handler=sigcont_handler};
        sigaction(SIGCONT, &act, NULL); 

        raise(SIGCONT); //the handler should print the message to stdout
        raise(SIGCONT); //the message should be ignored (it's the default)
        raise(SIGCONT); //one more message to be sure everything's okay
        exit(0);
    }
    wait(NULL);

    //checking the SA_NODEFER flag
    child_pd = fork();
    if (child_pd == 0){
        struct sigaction act = {.sa_flags = SA_NODEFER, .sa_handler=recursive_handler};
        sigaction(SIGCONT, &act, NULL); 

        raise(SIGCONT);
        exit(0);
    }
    wait(NULL);
}
