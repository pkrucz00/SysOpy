#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>

#include <limits.h>

#define MAX_NO_CHARS  100

// char* itoa(int value) {
//   static char buff[6];
//   sprintf(buff, "%i", value);
//   return strdup(buff);
// }

int main(int argc, char** argv){
    if (argc != 5){
        perror("Wrong number of arguments");
        exit(1);
    }

    int fd = open(argv[1], O_WRONLY);   //fifo descriptor
    if (fd == -1){
        perror("Couldn't open fifo\n");
        exit(EXIT_FAILURE);
    }
    int line_no = atoi(argv[2]);
    FILE* in = fopen(argv[3], "r");
    if (in == NULL){
        perror("Couldn't open input file\n");
        exit(EXIT_FAILURE);
    }
    int len = atoi(argv[4]);

    char read_buffer[MAX_NO_CHARS + 1];
    char pipe_buffer[MAX_NO_CHARS + 4];
    
    while (fread(read_buffer, sizeof(char), len, in) != 0){
        sprintf(pipe_buffer, "%d|%s\n", line_no, read_buffer);
        write(fd, pipe_buffer, strlen(pipe_buffer));
        sleep(2);
    }

    close(fd);
    fclose(in);
}
