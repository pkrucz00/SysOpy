#include "common.h"
#include <time.h>

int server_qd;   //queue of hte main server

int my_qd;      //private queue of this process
int my_id;      // id in server process

int commun_qd;  // communication queue descriptor with another client 
pid_t commun_pid;   //pid of the connected client


//-----------helpers----------------

char rand_char() {
    return rand() % ('z' - 'a' + 1) + 'a';
}

void send_notification(){
    if (commun_pid == 0){
        perror("Client error: invalid pid of another process\n");
        exit(EXIT_FAILURE);
    }
    
    kill(commun_pid, SIGUSR1);
}



//------- message handlers ----------

void send_disconnect_server() {
    my_msg msg;
    msg.mtype = DISCONNECT;
    msg.sender_pid = getpid();
    msgsnd(server_qd, &msg, size, 0);
}

void send_init(){
    my_msg to_server;
    to_server.mtype = INIT;
    to_server.sender_pid = getpid();
    sprintf(to_server.msg_text, "%d", my_qd);

    if (msgsnd(server_qd, &to_server, size, 0) == -1){
        perror("Client error: problem with sending init message to server\n");
        exit(EXIT_FAILURE);
    }

    my_msg from_server;

    if (msgrcv(my_qd, &from_server, size, INIT, 0) == -1) {
        perror("Client error: problem with receiving init message from server\n");
        exit(EXIT_FAILURE);
    }

    sscanf(from_server.msg_text, "%d", &my_id);
    printf("Your client ID is %d\n", my_id);
}


void send_disconnect(){
    if (commun_qd != 0){
        my_msg msg;
        msg.mtype = DISCONNECT;
        msg.sender_pid = getpid();
        
        msgsnd(commun_qd, &msg, size, 0);
        send_notification();
        printf("------chat ended------\n");

        commun_qd = 0;
        commun_pid = 0;
    }
    send_disconnect_server();
    
}

void send_stop(){
    msgctl(my_qd, IPC_RMID, NULL);
    
    send_disconnect();

    my_msg stop_msg;
    stop_msg.mtype = STOP;
    stop_msg.sender_pid = getpid();
     

    msgsnd(server_qd, &stop_msg, size, 0);

     
}

void sigint_handler(){
    printf("\nGoodbye!\n");
    exit(0);
}


void send_message(char* msg_text){
    my_msg to_client;
    to_client.mtype = CONNECT;
    to_client.sender_pid = getpid();
    strcpy(to_client.msg_text, msg_text);

    if (msgsnd(commun_qd, &to_client, size, 0) == -1) {
        perror("Client error: Problem with sending message to another client\n");
        exit(EXIT_FAILURE);
    }
    send_notification();
}


void send_list(){
    my_msg to_server;
    to_server.mtype = LIST;
    to_server.sender_pid = getpid();
    
    if (msgsnd(server_qd, &to_server, size, 0) == -1){
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
    to_server.mtype = CONNECT;
    to_server.sender_pid = getpid();
    sprintf(to_server.msg_text, "%d", commun_id);

     
    if (msgsnd(server_qd, &to_server, size, 0) == -1){
        perror("Client error: Cannot send the connect message to the server\n");
        exit(EXIT_FAILURE);
    }
     

    my_msg from_server;
    printf("I've receive message from %d\n", my_qd);
    if (msgrcv(my_qd, &from_server, size, CONNECT, 0) == -1){
        perror("Client error: Cannot receive the 'connect' message from the server\n");
        exit(EXIT_FAILURE);
    }
    
    sscanf(from_server.msg_text, "%d %d", &commun_qd, &commun_pid);

    my_msg to_client;
    to_client.mtype = CONNECT;
    to_client.sender_pid = getpid();
    sprintf(to_client.msg_text, "%d", my_qd);

    
    if (msgsnd(commun_qd, &to_client, size, 0) == -1){
        perror("Clinet warning: Problem with sending CONNECT message to client\n");
        exit(EXIT_FAILURE);
    }
    
    send_notification();
    printf("--------chat--------\n");
}


void receive_notification(){
    my_msg to_me;
    if (msgrcv(my_qd, &to_me, size, -(INIT+1), 0) == -1){
        perror("Client error: problem with receiving messages\n");
        exit(EXIT_FAILURE);
    }
    
    if (to_me.mtype == CONNECT){
        if (commun_qd == 0){
            commun_pid = to_me.sender_pid;
            commun_qd = atoi(to_me.msg_text);
            printf("----chat----\n");
        }
        else {
            printf("%d> %s\n", commun_pid, to_me.msg_text);
            fflush(stdout);
        }
    } else if (to_me.mtype == DISCONNECT){
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
        perror("Problem with SIGINT handler\n");
        exit(EXIT_FAILURE);
    }

    char *home_path = getenv("HOME");
    if (home_path == NULL){
        perror("Client error: Problem with getting HOME variable\n");
        exit(EXIT_FAILURE);
    }

    key_t k = ftok(home_path, PROJECT_ID);
    if (k == -1) {
        perror("Client error: problem with getting the queue key\n");
        exit(EXIT_FAILURE);
    }

    server_qd = msgget(k, 0);
    if (server_qd == -1) {
        perror("Client error: Problem with getting the server queue\n");
        exit(EXIT_FAILURE);
    }

    my_qd = create_q(home_path, rand_char());

    send_init();

    char text[MAX_MSG_LEN];
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