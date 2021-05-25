#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

#include <unistd.h>

#include <pthread.h>  //link with -pthread

#include <math.h>   // link with -lm

#define MAX_M 255

typedef struct image{
    int width, height;
    int **pixels;  //pixels[y][x]
}image;

typedef struct thread_args{
    image* img;
    image* rev_img;
    int index;
} thread_args;

int no_threads;
// helpers
int safe_atoi(char* buff){
    char* endptr;
    int result = strtol(buff, &endptr, 10);
    if (*endptr){
        perror("Invalid argument\n");
        exit(EXIT_FAILURE);
    }
    return result;
}

int **alloc_2d_table(int n, int m){
    int **result = (int**) calloc(n, sizeof(int*));
    for (int i = 0; i < n; i++){
        result[i] = (int*) calloc(m, sizeof(int));
    }
    return result;
}


void free_2d_table(int **table, int n){
    for (int i = 0; i < n; i++){
        free(table[i]);
    }
    free(table);
}

image* copy_image(image* src){
    image* dest = calloc(1, sizeof(image));
    dest->width = src->width;
    dest->height = src->height;
    dest->pixels = alloc_2d_table(src->height, src->width);
    for (int y = 0; y < dest->height; y++){
        for (int x = 0; x < dest->width; x++){
            dest->pixels[y][x] = src->pixels[y][x];
        }
    }
    return dest;
}

image* load_image(char* path){
    FILE* file = fopen(path, "r");
    if(file == NULL){
        perror("Problem with opening the input file\n");
        exit(EXIT_FAILURE);
    }

    image* img = malloc(sizeof(image));
    int m;
    fscanf(file, "P2\n%d %d\n%d\n", &img->width, &img->height, &m);

    if (m != MAX_M){
        perror("Wrong number of shades\n");
        exit(EXIT_FAILURE);
    }


    img->pixels = alloc_2d_table(img->height, img->width);

    int buff;
    for (int y = 0; y < img->height; y++){
        for (int x = 0; x < img->width; x++){
            fscanf(file, "%d", &buff);
            img->pixels[y][x] = buff;
        }
    }
    
    fclose(file);
    return img;
}

void save_image(image* img, char* path){
    FILE* file = fopen(path, "w");
    if (file == NULL){
        perror("Problem with creating output file\n");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", img->width, img->height);
    fprintf(file, "255\n");

    for (int y = 0; y < img->height; y++){
        for (int x = 0; x < img->width-1; x++){
            fprintf(file, "%d ", img->pixels[y][x]);
        }
        fprintf(file, "%d\n", img->pixels[y][img->width-1]);
    }
    fclose(file);
}

// functions for modification of the image
long int* number_traverse(thread_args* args){
    struct timeval start, stop;
    gettimeofday(&start, NULL);
    
    image* img = args->img;
    image* rev_img = args->rev_img;
    int index = args->index;

    int min_val = index*ceil(256.0 /(double) no_threads);
    int max_val = (index != no_threads - 1) ? (index+1)*ceil(256.0/(double) no_threads) : 256;

    for (int y = 0; y < img->height; y++){
        for (int x = 0; x < img-> width; x++){
            if (img->pixels[y][x] >= min_val && img->pixels[y][x] < max_val){
                rev_img->pixels[y][x] = 255 - img->pixels[y][x];
            }
        }
    }

    gettimeofday(&stop, NULL);
    long int* time = calloc(1, sizeof(long int));
    *time = 1000000*(stop.tv_sec-start.tv_sec) + (stop.tv_usec - start.tv_usec);
    pthread_exit(time);
}

long int* block_traverse(thread_args* args){
    struct timeval start, stop;
    gettimeofday(&start, NULL);
    
    image* img = args->img;
    image* rev_img = args->rev_img;
    int index = args->index;

    int min_w = index*ceil(img->width /(double) no_threads);
    int max_w = (index != no_threads - 1) ? (index+1)*ceil(img->width/(double) no_threads) : img->width;

    for (int y = 0; y < img->height; y++){
        for (int x = min_w; x < max_w; x++){
            rev_img->pixels[y][x] = 255 - img->pixels[y][x];
        }
    }

    gettimeofday(&stop, NULL);
    long int* time = calloc(1, sizeof(long int));
    *time = 1000000*(stop.tv_sec-start.tv_sec) + (stop.tv_usec - start.tv_usec);
    pthread_exit(time);
}


int main(int argc, char** argv){
    if (argc != 5){
        perror("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    no_threads = safe_atoi(argv[1]);
    char *traverse_type = strdup(argv[2]);
    char *path_in = strdup(argv[3]);
    char *path_out = strdup(argv[4]);
    
    image* img = load_image(path_in);
    image* rev_img = copy_image(img);

    pthread_t *threads = calloc(no_threads, sizeof(pthread_t));
    thread_args *args = calloc(no_threads, sizeof(thread_args));
    for (int i = 0; i < no_threads; i++){
        args[i].img = img;
        args[i].rev_img = rev_img;
        args[i].index = i; 
    }
    long int **exec_times = calloc(no_threads, sizeof(long int*));
    for (int i = 0; i < no_threads; i++){
        exec_times[i] = calloc(1, sizeof(long int));
    }
    
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    if (strcmp(traverse_type, "numbers") == 0){
        for (int i = 0; i < no_threads; i++){
            pthread_create(&threads[i], NULL, (void* (*)(void *)) number_traverse, &args[i]);
        }
    } else if (strcmp(traverse_type, "block") == 0){
        for (int i = 0; i < no_threads; i++){
            pthread_create(&threads[i], NULL, (void* (*)(void *)) block_traverse, &args[i]);
        }
    } else {
        perror("Error: unknown traverse type\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < no_threads; i++){
        pthread_join(threads[i], (void*) &exec_times[i]);
    }

    gettimeofday(&stop, NULL);
    long int total_time = 1000000*(stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec);

    save_image(rev_img, path_out);
    
    printf("No threads: %3d, traverse function: %7s\n", no_threads, traverse_type);
    //printing execution times, (IO can contaminate the time measurement) 
    for (int i = 0; i < no_threads; i++){
        printf("thread: %3d\ttime: %7lu [µs]\n", i, *(exec_times[i]) );
    }
    printf("Total time: %5lu [µs]\n\n", total_time);

    free(threads);
    for (int i = 0; i < no_threads; i++){
        free(exec_times[i]);
    }
    free(exec_times);
    
    free(args);   
    free_2d_table(rev_img->pixels, rev_img->height);
    free(rev_img);
    free_2d_table(img->pixels, img->height);
    free(img);
    return 0;
}