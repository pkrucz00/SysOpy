#include "common.h"

//game functions

game* create_new_game(int p1, int p2){
    game* g = (game*) malloc(sizeof(game));
    g -> p1 = p1;
    g -> p2 = p2;
    for (int i = 0; i < 9; i++){
        g -> grid[i] = EMPTY;
    }
    g -> status = GAME_ON;

    return g;
}

void move(game* g, int field_index, GRID_FIELD character){
    g->grid[field_index] = character;
    g->status = check_game_status(g);
}

char* display_board(game* g){
    char* result = (char*) calloc(13, sizeof(char));
    int curr_ind = 0;
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            int field_num_key = 3*i + j;
            GRID_FIELD sign = g->grid[field_num_key];
            char sign_char_representation = sign == X ? 'X' : (sign == O ? 'O' : '0' + field_num_key);
            
            result[curr_ind++] = sign_char_representation;
        }
        result[curr_ind++] = '\n';
    }
    return result;
}

GAME_STATUS check_game_status(game* g){
    int possible_strikes[8][3] = { {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6} };
    for (int i = 0; i < 8; i++){
        int strike_sum_check = 0;
        for (int j = 0; j < 3; j++){
            strike_sum_check += g->grid[possible_strikes[i][j]];
        } 
        if (strike_sum_check == -3){
            return O_WON;
        }
        if (strike_sum_check == 3){
            return X_WON;
        }
    }
    for (int i = 0; i < 9; i++){
        if (g->grid[i] == EMPTY){
            return GAME_ON;
        }
    }
    return DRAW;
}

//client-server functions
enum MSG_TYPE get_message_type(char* message) {
    enum MSG_TYPE type;
    scanf(message, "%d", (int*) &type);
    return type;
}

char* get_message_data(char* message) {
    int tmp;
    char* content = (char*) calloc(MAX_MSG_LEN-1, sizeof(char));
    sscanf(message, "%d:%[^:]", &tmp, content);
    return content;
}

msg* read_message(int sock_fd) {
    msg* result_msg = (msg*) malloc(sizeof(msg));
    char* raw_msg = (char*) calloc(MAX_MSG_LEN, sizeof(char));
    if(read(sock_fd, (void*) raw_msg, MAX_MSG_LEN) < 0){
        perror("Problem with reading the message\n");
        exit(EXIT_FAILURE);
    } 

    result_msg->type = get_message_type(raw_msg);
    strcpy(result_msg->data, get_message_data(raw_msg));
    free(raw_msg);
    return result_msg;
}

msg* read_message_nonblocking(int sock_fd){
    msg* result_msg = (msg*) malloc(sizeof(msg));
    char* raw_msg = (char*) calloc(MAX_MSG_LEN, sizeof(char));
    if(reav(sock_fd, (void*) raw_msg, MAX_MSG_LEN, MSG_DONTWAIT) < 0){
        perror("Problem with reading the message\n");
        exit(EXIT_FAILURE);
    } 

    result_msg->type = get_message_type(raw_msg);
    strcpy(result_msg->data, get_message_data(raw_msg));
    free(raw_msg);
    return result_msg;
}

client* create_client(int fd, char* name){
    client* c = (client*) malloc(sizeof(client));
    c -> fd = fd;
    c -> is_responding = 1;
    strcpy(c->name, name);
    return c;
}

// helpers
int rand_range(int a, int b){
    return rand()%(b - a + 1) + a;
}


