#include "common.h"

client* clients[MAX_NO_CLIENTS];

int port_num;
char* socket_path;

int sock_fd_local;
struct sockaddr_un sock_struct_local;

int sock_fd_ipv4;
struct sockaddr_in sock_struct_ipv4;

pthread_t thread_connect;
pthread_t thread_ping;

int waiting_client;

game* games[MAX_NO_CLIENTS / 2];
GRID_FIELD client_signs[MAX_NO_CLIENTS];
int client_games[MAX_NO_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

//starting servers
void start_server_local(){
    sock_struct_local.sun_family = AF_UNIX;
    strcpy(sock_struct_local.sun_path, socket_path);

    if ((sock_fd_local = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("Problem with creating local socket\n");
        exit(EXIT_FAILURE);
    }
    if (bind(sock_fd_local, (struct sockaddr*) &sock_struct_local, sizeof(sock_struct_local)) == -1){
        perror("Problem with binding local server\n");
        exit(EXIT_FAILURE);
    }
    if (listen(sock_fd_local, MAX_NO_CLIENTS) == -1){
        perror("Problem with setting the socket as a passive socket\n");
        exit(EXIT_FAILURE);
    }
    printf("UNIX soccet listens on %s\n", socket_path);
}

void start_server_network(){
    struct hostent* host_entry = gethostbyname("localhost");
    struct in_addr* host_address = malloc(sizeof(struct in_addr));
    host_address = (struct in_add*) host_entry->h_addr;

    sock_struct_ipv4.sin_family = AF_INET;
    sock_struct_ipv4.sin_port = htons(port_num);
    sock_struct_ipv4.sin_addr.s_addr = host_address->s_addr;

    if ((sock_fd_ipv4 = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Problem with creating network socket\n");
        exit(EXIT_FAILURE);
    }
    if (bind(sock_fd_ipv4, (struct sockaddr*) &sock_struct_ipv4, sizeof(sock_struct_ipv4)) == -1){
        perror("Problem with binding the socket\n");
        exit(EXIT_FAILURE);
    }
    if (listen(sock_fd_ipv4, MAX_NO_CLIENTS) == -1){
        perror("Problem with setting the socket as a passive socket\n");
        exit(EXIT_FAILURE);
    }
    printf("INET socket listening on %s:%d\n", inet_ntoa(*host_address), port_num);
    free(host_address);
}

void start_server(){
    start_server_local();
    start_server_network();
}

//shutting down server
void shutdown_server(){
    if (pthread_cancel(thread_connect) == -1){
        perror("Problem with canceling the connection thead\n");
        exit(EXIT_FAILURE);
    } if (pthread_cancel(thread_ping) == -1){
        perror("Problem with canceling the ping thead\n");
        exit(EXIT_FAILURE);
    } if (shutdown(sock_fd_local, SHUT_RDWR) == -1){
        perror("Problem with shutting down the local server\n");
        exit(EXIT_FAILURE);
    } if (close(sock_fd_local) == -1){
        perror("problem with closing the local server\n");
        exit(EXIT_FAILURE);
    } if (unlink(socket_path) == -1){
        perror("Problem with unlinking socket path\n");
        exit(EXIT_FAILURE);
    }
    if (shutdown(sock_fd_ipv4, SHUT_RDWR) == -1){
        perror("Problem with shutting down the network server\n");
        exit(EXIT_FAILURE);
    } if (close(sock_fd_ipv4) == -1){
        perror("Problem with closing network server\n");
        exit(EXIT_FAILURE);
    }
}

void sigint_handler(){
    exit(0);
}

//closing connection
void close_connection(int fd){
    if (shutdown(fd, SHUT_RDWR) == -1){
        perror("Problem with shutting down connection\n");
        exit(EXIT_FAILURE);
    }
    if (close(fd) == -1){
        perror("Problem with closing descriptor\n");
        exit(EXIT_FAILURE);
    }
}

//game maintaining code

int add_game(int p1, int p2){
    for (int i = 0; i < MAX_NO_CLIENTS; i++){
        if (games[i] == NULL) {
            games[i] = create_new_game(p1, p2);
            return i;
        }
    }
    return -1;
}

void make_game(int registered_index) {
    if (waiting_client == -1) {
        printf("No one is waiting\n");
        send_message(clients[registered_index]->fd, game_waiting, NULL);
        waiting_client = registered_index;
    } else {
        printf("Waiting client: %d\n", waiting_client);
        int game_index = add_game(registered_index, waiting_client);

        client_games[registered_index] = game_index;
        client_games[waiting_client] = game_index;

        int coin_toss = rand_range(0,1);
        int x_ind = coin_toss ? registered_index : waiting_client;
        int o_ind = coin_toss ? waiting_client : registered_index;  
        
        client_signs[x_ind] = X;
        send_message(clients[x_ind]->fd, game_found, "X");
        client_signs[o_ind] = O;
        send_message(clients[o_ind]->fd, game_found, "O");
            
        waiting_client = -1;
    }
}


//thread_create functions
int register_client(int fd, char* name) {
    int free_index = -1;
    for (int i = 0; i < MAX_NO_CLIENTS; i++) {
        if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0){
            return -1;
        } if (clients[i] == NULL && free_index == -1){
            free_index = i;
        }
    }
    if (free_index == -1){
        return -1;
    }
    clients[free_index] = create_client(fd, name);
    return free_index;
}

void unregister_client(int fd) {
    for (int i = 0; i < MAX_NO_CLIENTS; i++){
        if (clients[i] && clients[i]->fd == fd){
            clients[i] == NULL;
        }
    }
}

int process_login(int sock_fd){
    printf("New login pending...\n");

    int client_sock_fd;
    if ((client_sock_fd = accept(sock_fd, NULL, NULL)) == -1){
        perror("Problem with accepting socket descriptor\n");
        exit(EXIT_FAILURE);
    }

    msg* new_msg = read_message(client_sock_fd);
    printf("Received name %s\n", new_msg->data);

    int registered_index = register_client(client_sock_fd, new_msg->data);
    if (registered_index == -1){
        printf("Login rejected\n");
        send_message(client_sock_fd, login_rejected, "name exists");
        close_connection(client_sock_fd);
    } else {
        printf("Login accepted...\n");
        send_message(client_sock_fd, login_approved, NULL);
    }
    return registered_index;
}


void* process_connections() {
    struct pollfd fds[MAX_NO_CLIENTS+2];  //2 additional dexcriptors are for local and network server
    fds[MAX_NO_CLIENTS].fd = sock_fd_local;
    fds[MAX_NO_CLIENTS].events = POLLIN;

    fds[MAX_NO_CLIENTS+1].fd = sock_fd_ipv4;
    fds[MAX_NO_CLIENTS+1].events = POLLIN;

    waiting_client = -1;
    while (1){
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_NO_CLIENTS; i++){
            fds[i].fd = clients[i] != NULL ? clients[i]->fd : -1;
            fds[i].events = POLLIN;
            fds[i].revents = 0;
        }
        pthread_mutex_unlock(&clients_mutex);

        fds[MAX_NO_CLIENTS].revents = 0;
        fds[MAX_NO_CLIENTS + 1].revents = 0;

        printf("Pollin...\n");
        poll(fds, MAX_NO_CLIENTS+2, -1);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_NO_CLIENTS+2; i++){
            if (i < MAX_NO_CLIENTS && clients[i] == NULL)
                continue;

            if (fds[i].revents & POLLHUP){
                close_connection(fds[i].fd);
                unregister_client(fds[i].fd);
            } else if (fds[i].revents & POLLIN){
                if (fds[i].fd == sock_fd_local || fds[i].fd == sock_fd_ipv4){   // server descriptors
                    int registered_index = process_login(fds[i].fd);
                    printf("Client registered at index %d\n", registered_index);
                    if (registered_index >= 0)
                        make_game(registered_index);
                }
                else{                       // client descriptors
                    printf("New message received\n");
                    msg* new_msg = read_message(fds[i].fd);
                    if (new_msg->type == game_move){
                        printf("Move made: %s\n", new_msg->data);
                        game* g = games[client_games[i]];
                        int field_in = atoi(new_msg->data);
                        GRID_FIELD sign = client_signs[i];
                        move(g, field_in, sign);
                        int other = g->p1 == i ? g->p2 : g->p1;

                        if (g->status == GAME_ON){
                            send_message(fds[other].fd, game_move, display_board(g));
                        } else {
                            send_message(fds[i].fd, game_over, "GAME OVER");
                            send_message(fds[other].fd, game_over, "GAME OVER");
                        }
                    } else if (new_msg->type == logout){
                        close_connection(fds[i].fd);
                        unregister_client(fds[i].fd);
                    } else if (new_msg->type == ping){
                        clients[i]->is_responding = 1;
                    } else {
                        printf("Unknown message type\n");
                    }
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    pthread_exit((void*) 0);
}

void* process_ping(){
    while (1){
        sleep(PING_CHECK_GAP);

        printf("Pinging clients\n");

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_NO_CLIENTS; i++){
            if (clients[i] != NULL) {
                clients[i] -> is_responding = 0;
                send_message(clients[i]->fd, ping, NULL);
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        sleep(PING_TIMEOUT);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_NO_CLIENTS; i++){
            if (clients[i] != NULL && clients[i]->is_responding == 0){
                printf("Client %d hasnt responded. Disconnecting.\n", i);
                close_connection(clients[i]->fd);
                unregister_client(clients[i]->fd);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    pthread_exit((void*) 0);
}


int main(int argc, char** argv){
    if (argc != 3) {
        perror("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    port_num = atoi(argv[1]);
    socket_path = argv[2];

    if (atexit(shutdown_server) == -1){
        perror("Problem with initializing atexit\n");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGINT, sigint_handler) == -1){
        perror("Problem with setting signal handler\n");
        exit(EXIT_FAILURE);
    }

    start_server();

    if (pthread_create(&thread_connect, NULL, process_connections, NULL) == -1){
        perror("Problem with creating connection thread\n");
        exit(EXIT_FAILURE);
    } if (pthread_create(&thread_ping, NULL, process_ping, NULL) == -1){
        perror("Problem with creating connection thread\n");
        exit(EXIT_FAILURE);
    } 

    if (pthread_join(thread_connect, NULL) == -1){
        perror("Problem with joining connection thread\n");
        exit(EXIT_FAILURE);
    }if (pthread_join(thread_ping, NULL) == -1){
        perror("Problem with joining ping thread\n");
        exit(EXIT_FAILURE);
    }

    stop_server();
    return 0;
}



