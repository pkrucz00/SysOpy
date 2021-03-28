#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_LIMIT 50 

int main(int argc, char** argv){
    if (argc < 2){
        printf("Too few arguments");
        exit(EXIT_FAILURE);
    }

    char* src_name = strdup(argv[1]);
    char* dest_name = strdup(argv[2]);

    FILE* src_file = fopen(src_name, "r");
    FILE* dest_file = fopen(dest_name, "w");

    char *buff = NULL;
    size_t len = 0;     
    ssize_t read;
    while ((read = getline(&buff, &len, src_file)) != -1 ) { 
        char *tmp = buff;   //wskazanie na jeszcze niezapisany fragment
        char* str_to_copy = malloc(STR_LIMIT + 2);  //50 dopuszczalnych znaków + znak nowej linii + '\0'
        while (read > STR_LIMIT){
            strncpy(str_to_copy, tmp, STR_LIMIT);
            strcat(str_to_copy, "\n\0");
            printf("%s", str_to_copy);

            fprintf(dest_file, "%s", str_to_copy);
            tmp += STR_LIMIT;
            read -= STR_LIMIT;
        }
        fprintf(dest_file, "%s", tmp);
    }
    free(buff);

    fclose(src_file);
    fclose(dest_file);

    free(src_name);
    free(dest_name);

    return 0; 

}