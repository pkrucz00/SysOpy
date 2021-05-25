#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include <errno.h>
#include <signal.h>

#define MAX_MSG_LEN 256
#define MAX_USERS 10
#define PROJECT_ID 'Y'

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5

typedef struct my_msg {
    long mtype;
    pid_t sender_pid;
    char msg_text[MAX_MSG_LEN];
} my_msg;

const int size = sizeof(my_msg) - sizeof(long);

int create_q(const char* path, char proj_id){
    key_t key = ftok(path, proj_id);
    if (key == -1){
        perror("Error with ftok\n");
        exit(EXIT_FAILURE);
    }

    int qd = msgget(key, IPC_CREAT|IPC_EXCL|0666);
    if (qd == -1){
        perror("Error creating queue\n");
        exit(EXIT_FAILURE);
    }
    return qd;

}

#endif //COMMON_H
