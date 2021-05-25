#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <stdarg.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "vector.h"

#define IN 0
#define OUT 1

const char *COMMAND_DELIM = "|";
const char *ARGUMENT_DELIM = " \f\n\r\t\v"; // spaces according to man 

typedef vector vec_char;            //vector<char*>
typedef vector vec_of_vec_char;     //vector<vector<char*>>
typedef vector vec_pinfo;           //vector<pipe_info>

typedef struct {
    char *name;
    vec_of_vec_char exec_args;  // 2D vector of commands with arguments
} pipe_info;



int pipe_info_ptr_cmp(const void *p1, const void *p2) {
    pipe_info * const *c1 = p1;
    pipe_info * const *c2 = p2;

    return strcmp((*c1)->name, (*c2)->name);
}

int pipe_info_search_cmp(const void *key, const void *elem) {
    char * const *key_str = key;
    pipe_info * const *pipe = elem;

    return strcmp(*key_str, (*pipe)->name);
}

void free_vec_char_content(vec_char *v) {
    while (v->size > 0) {
        free(vec_pop_back(v));
    }
}

void free_vec_of_vec_char_content(vec_of_vec_char *v) {
    while (v->size > 0) {
        vec_char *vc = vec_pop_back(v);
        free_vec_char_content(vc);
        free(vc);
    }
}

void free_pipes(vec_pinfo *pipes) {
    while (pipes->size > 0) {
        pipe_info *pipe = vec_pop_back(pipes);

        free_vec_of_vec_char_content(&pipe->exec_args);
        free(pipe->name);
        free(pipe);
    }
}

void terminate_all_processes_handler(int sig) {
    kill(0, SIGTERM);
    _exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int return_value = EXIT_FAILURE;

    if (argc != 2) {
        fprintf(stderr, "invalid argument count\n");
        return return_value;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror(argv[1]);
        return return_value;
    }
    
    
    struct sigaction act;
    act.sa_handler = terminate_all_processes_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    
    sigaction(SIGTERM, &act, NULL);
    setpgid(0, 0);
    
    vec_pinfo pipes;
    vec_init(&pipes);

    bool def_flag = true;

    char *line = NULL;
    size_t n = 0;
    ssize_t line_no = -1;
    while (getline(&line, &n, input) != -1) {
        line_no++;

        //definitions come before execution lines, separated with single space
        if (line[0] == '\n') {
            if (def_flag) {
                def_flag = false;
                //sort our dictionary so we can bsearch in it
                qsort(pipes.storage, pipes.size, sizeof(*pipes.storage), pipe_info_ptr_cmp);
            }
            continue;
        }

        if (def_flag) {
            char *pipe_def = strchr(line, '=') + 1;
            char *before_eq = strtok(line, "=");

            if (before_eq) {
                pipe_info *pipe = malloc(sizeof(*pipe));
                pipe->name = NULL;
                vec_init(&pipe->exec_args);
                vec_push_back(&pipes, pipe);

                char *stripped_name = strtok(before_eq, ARGUMENT_DELIM);
                if (!stripped_name) {
                    fprintf(stderr, "line %zd: empty name of definition\n", line_no);
                    goto end;
                }
                pipe->name = strdup(stripped_name);

                char *command_saveptr = NULL;
                char *command = strtok_r(pipe_def, COMMAND_DELIM, &command_saveptr); //reentrant!

                if (!command) {
                    fprintf(stderr, "line %zd: empty command def\n", line_no);
                    goto end;
                }

                while (command) {
                    char *arg_saveptr = NULL;
                    char *arg = strtok_r(command, ARGUMENT_DELIM, &arg_saveptr);

                    if (!arg) {
                        fprintf(stderr, "line %zd, empty args def\n", line_no);
                        goto end;
                    }

                    vec_char *args = malloc(sizeof(*args));
                    vec_init(args);
                    while (arg) {
                        vec_push_back(args, strdup(arg));
                        arg = strtok_r(NULL, ARGUMENT_DELIM, &arg_saveptr);
                    }
                    vec_push_back(args, NULL); //make suitable to pass to exec - must be NULL terminated

                    vec_push_back(&pipe->exec_args, args);
                    command = strtok_r(NULL, COMMAND_DELIM, &command_saveptr);
                }
            }
            else {
                fprintf(stderr, "line %zd: malformed definition\n", line_no);
                goto end;
            }
        }
        else {
            vec_of_vec_char exec_args_shallow; //we'll concat command lists, by shallow copy of ptrs to args lists
            vec_init(&exec_args_shallow);

            char *name_saveptr = NULL;
            char *name = strtok_r(line, COMMAND_DELIM, &name_saveptr);

            if (!name) {
                fprintf(stderr, "line %zd: empty named pipe chain\n", line_no);

                goto end;
            }

            while (name) {
                char *stripped_name_saveptr = NULL;
                char *stripped_name = strtok_r(name, ARGUMENT_DELIM, &stripped_name_saveptr);

                if (!stripped_name) {
                    fprintf(stderr, "line %zd: empty named pipe in chain\n", line_no);

                    vec_clear(&exec_args_shallow);
                    goto end;
                }

                pipe_info **pipe_ptr = bsearch( &stripped_name, pipes.storage, pipes.size, sizeof(*pipes.storage), pipe_info_search_cmp);

                if (!pipe_ptr) {
                    fprintf(stderr, "line %zd: named pipe not found\n", line_no);

                    vec_clear(&exec_args_shallow);
                    goto end;
                }

                pipe_info *pipe = *pipe_ptr;
                for (size_t i = 0; i < pipe->exec_args.size; i++) {
                    vec_push_back(&exec_args_shallow, pipe->exec_args.storage[i]);
                }

                name = strtok_r(NULL, COMMAND_DELIM, &name_saveptr);
            }

            int stdout_to_inject;
            for (ssize_t i = exec_args_shallow.size - 1; i >= 0; i--) {  //downcount (to prevent clogging)
                vec_char *args_vec = exec_args_shallow.storage[i];
                char **args = (char**)args_vec->storage;

                int fd[2];
                if (i > 0) {
                    pipe(fd);
                }

                if (fork() == 0) {
                    if (i < exec_args_shallow.size - 1) {
                        dup2(stdout_to_inject, STDOUT_FILENO);
                    }
                    if (i > 0) {
                        close(fd[OUT]);
                        dup2(fd[IN], STDIN_FILENO);
                    }
                    execvp(args[0], args);
                    goto end; //in case it fails
                }

                if (i < exec_args_shallow.size - 1) {
                    close(stdout_to_inject);
                }
                if (i > 0) {
                    close(fd[IN]);
                    stdout_to_inject = fd[OUT];
                }
            }

            int wstatus;
            while (wait(&wstatus) > 0) {
                int exit_status;
                if (WIFEXITED(wstatus) && (exit_status = WEXITSTATUS(wstatus)) != 0) {
                    fprintf(stderr, "MISSION ABORT, SOME PROCESS RETURNED NONZERO CODE: %d\n", exit_status);
                    raise(SIGTERM);
                }
            }

            vec_clear(&exec_args_shallow);
        }
    }

    return_value = EXIT_SUCCESS;

    end:
    free(line);
    free_pipes(&pipes);
    fclose(input);

    return return_value;
}
