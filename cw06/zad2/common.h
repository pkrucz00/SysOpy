#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <fcntl.h>

#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <signal.h>

#define MAX_MSG_LEN 255
#define MAX_USERS 10
#define MAX_PATH_LEN 256

#define STOP 5
#define DISCONNECT 4
#define LIST 3
#define CONNECT 2
#define INIT 1

#define SERVER_Q_PATH "/server_queue"

typedef struct my_msg {
    pid_t sender_pid;
    char msg_text[MAX_MSG_LEN];
} my_msg;

const int size = sizeof(my_msg);

mqd_t create_q(const char* path){
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = size;
    
    mqd_t qd = mq_open(path, O_RDONLY|O_CREAT|O_EXCL, 0666, &attr);
    if (qd == -1){
        perror("Error creating queue\n");
        exit(EXIT_FAILURE);
    }
    return qd;

}

mqd_t open_q(char path[]){
    mqd_t qd = mq_open(path, O_WRONLY);
    if (qd == -1){
        perror("Error: problem with opening the queue\n");
        exit(EXIT_FAILURE);
    }
    return qd;
}

#endif //COMMON_H