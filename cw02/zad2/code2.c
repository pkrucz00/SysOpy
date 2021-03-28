#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_LIMIT 256

int main(int argc, char** argv){
    if (argc < 1){
        printf("Too few arguments");
        exit(EXIT_FAILURE);
    }
    
    char sign = argv[1][0];
    char file_name[STR_LIMIT];
    strcpy(file_name, argv[2]);

    FILE* fp;
    fp = fopen(file_name, "r");
    if (fp == NULL){
        printf("Unable to open file %s", file_name);
        exit(EXIT_FAILURE);
    }

    char *buff = NULL;
    size_t len = 0;     

    while(getline(&buff, &len, fp) != -1 ){
        if (strchr(buff, sign)){
            printf("%s", buff);
        }
    }

    fclose(fp);
    return 0;
}