#ifndef COMMON_H
#define COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>

#include <poll.h>
#include <netdb.h>

#define MAX_NO_CLIENTS 12
#define MAX_MSG_LEN 20

#define PING_TIMEOUT 5
#define PING_CHECK_GAP 10

// maintaining the game
typedef enum GRID_FIELD {
    O = -1,
    X = 1,
    EMPTY = 0
} GRID_FIELD;

typedef enum GAME_STATUS{
    O_WON,
    X_WON,
    DRAW,
    GAME_ON
}GAME_STATUS;

typedef struct game {
    int p1;
    int p2;
    GRID_FIELD grid[9];
    GAME_STATUS status;
} game;

game* create_new_game(int p1, int p2);

void move(game* g, int field_index, GRID_FIELD character);

char* display_board(game* g);

GAME_STATUS check_game_status(game* g);

//client server communication common tructures/functions

typedef enum MSG_TYPE {
    login_request,
    login_approved,
    login_rejected,
    game_waiting,
    game_found,
    game_move,
    game_over,
    logout,
    ping
} MSG_TYPE;

typedef struct msg {
    MSG_TYPE type;
    char data[MAX_MSG_LEN];
} msg;

typedef struct client {
    int fd;
    char name[MAX_MSG_LEN];
    int is_responding;
} client;

msg* read_message(int sock_fd);

msg* read_message_noblock(int sock_fd);

void send_message(int sock_fd, MSG_TYPE type, char* content);

client* create_client(int fd, char* name);

//helpers 
int rand_range(int a, int b);

#endif
