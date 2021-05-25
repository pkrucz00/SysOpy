#include "common.h"
#include <time.h>

int server_qd;   //queue of hte main server

int my_qd;      //private queue of this process
int my_id;      // id in server process

int commun_qd;  // communication queue descriptor with another client 
pid_t commun_pid;   //pid of the connected client

char q_path[MAX_PATH_LEN];
//-----------helpers----------------


void set_notification(){
    struct sigevent reception;
    reception.sigev_notify = SIGEV_SIGNAL;
    reception.sigev_signo = SIGUSR1;

    mq_notify(my_qd, &reception);
}

// char rand_char() {
//     return rand() % ('z' - 'a' + 1) + 'a';
// }

// void send_notification(){
//     if (commun_pid == 0){
//         perror("Client error: invalid pid of another process\n");
//         exit(EXIT_FAILURE);
//     }
    
//     kill(commun_pid, SIGUSR1);
// }



//------- message handlers ----------

void send_disconnect_server() {
    my_msg msg;
    msg.sender_pid = getpid();
    if (mq_send(server_qd, (char* ) &msg, size, DISCONNECT) == -1){
        perror("Client error: problem with disconnecting from the server\n");
        exit(EXIT_FAILURE);
    }
}

void send_init(){
    my_msg to_server;
    to_server.sender_pid = getpid();
    sprintf(to_server.msg_text, "%d", my_qd);

    if (mq_send(server_qd, (char*) &to_server, size, INIT) == -1){
        perror("Client error: problem with sending init message to server\n");
        exit(EXIT_FAILURE);
    }

    my_msg from_server;

    if (mq_receive(my_qd, (char*) &from_server, size, NULL) == -1) {
        perror("Client error: problem with receiving init message from server\n");
        exit(EXIT_FAILURE);
    }

    sscanf(from_server.msg_text, "%d", &my_id);
    printf("Your client ID is %d\n", my_id);
}


void send_disconnect(){
    if (commun_qd != 0){
        my_msg msg;
        msg.sender_pid = getpid();
        
        if (mq_send(commun_qd, (char*) &msg, size, DISCONNECT) == -1){
            perror("Client error: problem with sending DISCONNECT message to another client\n");
            exit(EXIT_FAILURE);
        }
        
        printf("------chat ended------\n");

        mq_close(commun_qd);
        commun_qd = 0;
        commun_pid = 0;
    }
    send_disconnect_server();
    
}

void send_stop(){
    send_disconnect();

    my_msg stop_msg;
    stop_msg.sender_pid = getpid();

    mq_send(server_qd, (char*) &stop_msg, size, STOP);
    mq_close(server_qd);
    mq_unlink(q_path);
}

void sigint_handler(){
    printf("\nGoodbye!\n");
    exit(0);
}


void send_message(char* msg_text){
    my_msg to_client;
    to_client.sender_pid = getpid();
    strcpy(to_client.msg_text, msg_text);

    if (mq_send(commun_qd, (char*) &to_client, size, CONNECT) == -1) {
        perror("Client error: Problem with sending message to another client\n");
        exit(EXIT_FAILURE);
    }
}


void send_list(){
    my_msg to_server;
    to_server.sender_pid = getpid();
    
    if (mq_send(server_qd, (char*) &to_server, size, LIST) == -1){
        perror("Clinet error: Problem with sending LIST to server\n");
        exit(EXIT_FAILURE);
    }
}

void send_connect(int commun_id){
    if (commun_id == my_id){
        printf("The client you want to connect with is yourself\n");
        exit(EXIT_FAILURE);
    }
    
    my_msg to_server;
    to_server.sender_pid = getpid();
    sprintf(to_server.msg_text, "%d", commun_id);

     
    if (mq_send(server_qd, (char*) &to_server, size, CONNECT) == -1){
        perror("Client error: Cannot send the connect message to the server\n");
        exit(EXIT_FAILURE);
    }
     
    my_msg from_server;
    
    if (mq_receive(my_qd, (char*) &from_server, size, NULL) == -1){
        perror("Client error: Cannot receive the 'connect' message from the server\n");
        exit(EXIT_FAILURE);
    }
    char comm_path[MAX_PATH_LEN];
    
    sprintf((char*) comm_path, "/%s", from_server.msg_text);
    commun_qd = open_q(comm_path);
    commun_pid = atoi(from_server.msg_text);

    my_msg to_client;
    to_client.sender_pid = getpid();
    sprintf(to_client.msg_text, "%d", getpid());

    
    if (mq_send(commun_qd, (char*) &to_client, size, CONNECT) == -1){
        perror("Clinet warning: Problem with sending CONNECT message to client\n");
        exit(EXIT_FAILURE);
    }
    
    set_notification();
    printf("--------chat--------\n");
}


void receive_notification(){
    my_msg to_me;
    uint mtype;
    if (mq_receive(my_qd, (char*) &to_me, size, &mtype) == -1){
        perror("Client error: problem with receiving messages\n");
        exit(EXIT_FAILURE);
    }
    
    if (mtype == CONNECT){
        if (commun_qd == 0){
            commun_pid = to_me.sender_pid;
            char comm_path[MAX_PATH_LEN];
            sprintf((char*) comm_path, "/%d", commun_pid);
            commun_qd = open_q(comm_path);

            printf("----chat----\n");
        }
        else {
            printf("%d> %s\n", commun_pid, to_me.msg_text);
            fflush(stdout);
        }
        set_notification();
    } else if (mtype == DISCONNECT){
        commun_qd = 0;
        commun_pid = 0;
        send_disconnect_server();
        printf("----chat ended-----\n");
    }
}



int main(){
    srand(time(NULL));

    if (atexit(send_stop) == -1){
        perror("Client error: problem with atexit\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Problem with SIGINT handler\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGUSR1, receive_notification) == SIG_ERR){
        perror("Problem with SIGUSR1 handler\n");
        exit(EXIT_FAILURE);
    }

    sprintf(q_path, "/%d", getpid());


    server_qd = open_q(SERVER_Q_PATH);

    my_qd = create_q(q_path);

    send_init();

    char text[MAX_MSG_LEN];
    set_notification();

    while (1){
        fgets(text, MAX_MSG_LEN+1, stdin);
        text[strcspn(text, "\n")] = '\0';  //deleting the '\n' at the end

        if (strcmp(text, "DISCONNECT") == 0){
            send_disconnect();
        } else if (commun_qd != 0){
            send_message(text);
        } else if (strcmp(text, "LIST") == 0){
            send_list();
        } else if (strcmp(text, "STOP") == 0){
            exit(0);
        } else {
            char command[MAX_MSG_LEN];
            int commun_id;
            sscanf(text, "%s %d", command, &commun_id);

            if (strcmp(command, "CONNECT") == 0){
                send_connect(commun_id);
            } else {
                printf("Uknown command %s\n", command);
                fflush(stdout);
            }
        }
    }
    return 0;
}