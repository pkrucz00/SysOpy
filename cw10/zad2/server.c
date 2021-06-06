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

    if ((sock_fd_local = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
        perror("Problem with creating local socket\n");
        exit(EXIT_FAILURE);
    }
    if (bind(sock_fd_local, (struct sockaddr*) &sock_struct_local, sizeof(sock_struct_local)) == -1){
        perror("Problem with binding local server\n");
        exit(EXIT_FAILURE);
    }
    printf("UNIX socket listens on %s\n", socket_path);
}

void start_server_network(){
    struct hostent* host_entry = gethostbyname("localhost");
    struct in_addr* host_address = malloc(sizeof(struct in_addr));
    host_address = (struct in_addr*) host_entry->h_addr;

    sock_struct_ipv4.sin_family = AF_INET;
    sock_struct_ipv4.sin_port = htons(port_num);
    sock_struct_ipv4.sin_addr.s_addr = host_address->s_addr;

    if ((sock_fd_ipv4 = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Problem with creating network socket\n");
        exit(EXIT_FAILURE);
    }
    if (bind(sock_fd_ipv4, (struct sockaddr*) &sock_struct_ipv4, sizeof(sock_struct_ipv4)) == -1){
        perror("Problem with binding the socket\n");
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
    } if (close(sock_fd_local) == -1){
        perror("problem with closing the local server\n");
        exit(EXIT_FAILURE);
    } if (unlink(socket_path) == -1){
        perror("Problem with unlinking socket path\n");
        exit(EXIT_FAILURE);
    }
    if (close(sock_fd_ipv4) == -1){
        perror("Problem with closing network server\n");
        exit(EXIT_FAILURE);
    }
}

void sigint_handler(){
    exit(0);
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
        send_message_to(clients[registered_index]->fd, game_waiting, NULL, clients[registered_index]->addr);
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
        send_message_to(clients[x_ind]->fd, game_found, "X", clients[x_ind]->addr);
        client_signs[o_ind] = O;
        send_message_to(clients[o_ind]->fd, game_found, "O", clients[o_ind]->addr);
            
        waiting_client = -1;
    }
}


//thread_create functions
int register_client(int fd, struct sockaddr* addr, char* name) {
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
    clients[free_index] = create_client(fd, addr, name);
    return free_index;
}

void unregister_client(char* name) {
    for (int i = 0; i < MAX_NO_CLIENTS; i++){
        if (clients[i] && strcmp(clients[i]->name, name) == 0){
            clients[i] = NULL;
        }
    }
}

int get_user_index(char* name) {
    for (int i = 0; i < MAX_NO_CLIENTS; i++){
        if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0)
        return i;
    }
    return -1;
}


void* process_connections() {
    struct pollfd fds[2];  
    fds[0].fd = sock_fd_local;
    fds[0].events = POLLIN;

    fds[1].fd = sock_fd_ipv4;
    fds[1].events = POLLIN;

    waiting_client = -1;
    while (1){
        for (int i = 0; i < 2; i++){
            fds[i].events = POLLIN;
            fds[i].revents = 0;
        }
        printf("Pollin...\n");
        poll(fds, 2, -1);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < 2; i++){
            if (fds[i].revents & POLLIN){
                printf("New message received\n");
                struct sockaddr* addr = (struct sockaddr*) malloc(sizeof(struct sockaddr));
                socklen_t len = sizeof(&addr);
                msg* new_msg = read_message_from(fds[i].fd, addr, &len);
                if (new_msg->type == login_request){
                    printf("Registering user. Name %s\n", new_msg->user);
                    int registered_index = register_client(fds[i].fd, addr, new_msg->user);
                    printf("Client registered at index %d\n", registered_index);
                    if (registered_index == -1){
                        send_message_to(fds[i].fd, login_rejected, "name already in use", addr);
                    } else {
                        send_message_to(fds[i].fd, login_approved, NULL, addr);
                        make_game(registered_index);
                    }
                }  else if (new_msg->type == logout){
                    unregister_client(new_msg->user);
                    printf("User %s logged out\n", new_msg->user);
                } else if (new_msg->type == game_move){
                    printf("Move made: %s\n", new_msg->data);
                    int i = get_user_index(new_msg->user);
                    game* g = games[client_games[i]];
                    int field_in = atoi(new_msg->data);
                    GRID_FIELD sign = client_signs[i];
                    move(g, field_in, sign);
                    int other = g->p1 == i ? g->p2 : g->p1;

                    if (g->status == GAME_ON){
                        send_message_to(clients[other]->fd, game_move, display_board(g), clients[other]->addr);
                    } else if (g->status == DRAW) {
                        send_message_to(clients[other]->fd, game_over, "DRAW", clients[other]->addr);
                        send_message_to(clients[i]->fd, game_over, "DRAW", clients[i]->addr);
                    } else {
                        char* who_won = strdup( (g->status == O_WON ? "O WON" : "X WON")); 
                        send_message_to(clients[i]->fd, game_over, who_won, clients[i]->addr);
                        send_message_to(clients[other]->fd, game_over, who_won, clients[other]->addr);
                    }
                } else if (new_msg->type == ping){
                    int i = get_user_index(new_msg->user);
                    clients[i]->is_responding = 1;
                } 
                else {
                    printf("Unknown message type\n");
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
                send_message_to(clients[i]->fd, ping, NULL, clients[i]->addr);
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        sleep(PING_TIMEOUT);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_NO_CLIENTS; i++){
            if (clients[i] != NULL && clients[i]->is_responding == 0){
                printf("Client %d hasnt responded. Disconnecting.\n", i);
                unregister_client(clients[i]->name);
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
    if (signal(SIGINT, sigint_handler) == SIG_ERR){
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

    shutdown_server();
    return 0;
}