#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

extern char** environ;
const char* MY_ENV_VAR = "THIS_IS_INITIAL_CALL";

int main(int argc, char** argv){
    if (argc != 4) {
        printf("Bad number of arguments");
        exit(EXIT_FAILURE);
    }

    char* dir_name;

    if (getenv(MY_ENV_VAR) != NULL){
        dir_name = argv[1];
    } else {
        if (chdir(argv[1]) == -1){
            perror("Couldn't open a directory");
            exit(EXIT_FAILURE);
        }
        dir_name = "./";
        setenv(MY_ENV_VAR, "", 1);
    }

    char* needle = argv[2];
    int depth = atoi(argv[3]);

    DIR* dir = opendir(dir_name);
    struct dirent* curr_dir;
    while ( (curr_dir = readdir(dir))){
        if ( (strcmp(".", curr_dir->d_name) == 0) ||
             (strcmp("..", curr_dir->d_name) == 0 ) )
              continue;

        if (curr_dir->d_type == DT_REG){
            char *extension = strrchr(curr_dir->d_name, '.');

            if (extension != NULL && (strcmp( extension, ".txt") == 0)){
                char *fpath = malloc(strlen(dir_name) + strlen(curr_dir->d_name) + 1);
                strcpy(fpath, dir_name);
                strcat(fpath, curr_dir->d_name);
                

                FILE *file = fopen(fpath, "r");
                if (file != NULL){
                    char *buff = NULL;
                    size_t len = 0;
                    while ((getline(&buff, &len, file)) != -1 ) { 
                        if ( strstr(buff, needle) ) {
                            printf("%d: %s\n", getpid(), fpath);
                            break;
                        }
                    }
                    free(buff);
                    fclose(file);   
                }
                free(fpath);
            }
        }
        else if (curr_dir->d_type == DT_DIR && depth > 0){
            char *subdir_name = malloc(strlen(dir_name) + strlen(curr_dir->d_name) + 2);
            strcpy(subdir_name, dir_name);
            strcat(subdir_name, curr_dir->d_name);
            strcat(subdir_name, "/");

            char new_depth_str[12];
            sprintf(new_depth_str, "%d", depth-1);

            char *new_args[] = {argv[0], subdir_name, needle, new_depth_str, NULL};

            fflush(stdout);
            fflush(stderr);
            
            pid_t ppid = fork();
            if (ppid == 0){
                execve("/proc/self/exe", new_args, environ);
                _exit(EXIT_FAILURE);
            }

            free(subdir_name);
        }
    }

    closedir(dir);

    while(wait(NULL) != -1); 
}