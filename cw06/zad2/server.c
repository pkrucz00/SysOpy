# include "common.h"

pid_t user_pids[MAX_USERS];
int user_qds[MAX_USERS];
int user_connected[MAX_USERS];
int user_count;

int server_qd;


//--------exit handlers------------
void sigint_handler(){
    exit(EXIT_FAILURE);
}


void close_queue(){
    for (int i = 0; i < user_count; i++){
        mq_close(user_qds[i]);
        kill(user_pids[i], SIGINT);
    }
    mq_unlink(SERVER_Q_PATH);
}

// ----------- helpers------------------
int find_user_by_pid(pid_t pid){
    for (int i = 0; i < user_count; i++){
        if (user_pids[i] == pid)
            return i;
    }
    perror("Server error: coundn't find user with a given pid\n");
    exit(EXIT_FAILURE);
}


//-----------message handlers-------------------
void init_client(my_msg *msg){
    int i;
    for (i = 0; i < user_count; i++){
        if (user_pids[i] == 0)
            break;
    }
    if (i == MAX_USERS){
        perror("Server error: Too many users!");
        exit(EXIT_FAILURE);
    }
    
    user_pids[i] = msg->sender_pid;

    char user_path[10];
    sprintf((char*)user_path, "/%d", user_pids[i]);
    user_qds[i] = open_q(user_path);

    my_msg answer_to_client;
    answer_to_client.sender_pid = getpid();
    sprintf(answer_to_client.msg_text, "%d", i);

    if (user_count == i){
        user_count++;
    }
    if (mq_send(user_qds[i], (char*)&answer_to_client, size, INIT) == -1){
        perror("Server error: Can't send init message\n");
        exit(EXIT_FAILURE);
    }

    printf("Client with qd %d connected\n", user_qds[i]);
}


void connect_clients(my_msg *msg){
    int client_id = atoi(msg->msg_text);
    int client_pid = user_pids[client_id];
    
    my_msg to_client;
    to_client.sender_pid = getpid();
    int idx = find_user_by_pid(msg->sender_pid);

    sprintf(to_client.msg_text, "%d", client_pid);
    user_connected[idx] = 1;
    user_connected[client_id] = 1;

    if (mq_send(user_qds[idx], (char*) &to_client, size, CONNECT) == -1){
        perror("Server error: Problem with sending CONNECT message\n");
        exit(EXIT_FAILURE);
    }
    
    
}

void list_clients(my_msg *msg){    
    printf("----LIST OF USERS----\n");
    printf("client   availability\n");
    for (int i = 0; i < user_count; i++){
        if (user_pids[i] != 0){
            if (user_connected[i] == 0){
                printf("%6d - available\n", i);
            } else {
                printf("%6d - not available\n", i);
            }
        }
    }
    printf("---------------------\n");
}

void disconnect_clients(my_msg *msg){
    int client_idx = find_user_by_pid(msg->sender_pid);
    user_connected[client_idx] = 0;
}

void stop_clients(my_msg *msg){
    int client_idx = find_user_by_pid(msg->sender_pid);

    printf("Deleting user id: %d\n", client_idx);
    user_pids[client_idx] = 0;
    user_connected[client_idx] = 0;
    user_qds[client_idx] = 0;
}


void process_message(my_msg *msg, uint mtype){
    switch (mtype){
        case INIT:
            init_client(msg);
            break;
        case CONNECT:
            connect_clients(msg);
            break;
        case LIST:
            list_clients(msg);
            break;
        case DISCONNECT:
            disconnect_clients(msg);
            break;
        case STOP:
            stop_clients(msg);
            break; 
        default:
            perror("Server error: problem with parsing message");
            exit(EXIT_FAILURE);
    }
}


int main(){
    if (atexit(close_queue) != 0){
        perror("Server error: problem with atexit\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Server error: problem with SIGINT handler\n");
        exit(EXIT_FAILURE);
    }

    server_qd = create_q(SERVER_Q_PATH);

    my_msg msg_buff;
    uint mtype;

    while (1){
        if (mq_receive(server_qd, (char *) &msg_buff, size, &mtype) == -1){
            perror("Server error: problem with receiving a message\n");
            exit(EXIT_FAILURE);
        }

        process_message(&msg_buff, mtype);
    }
    
    return 0;
}