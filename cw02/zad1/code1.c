#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/stat.h>

#define STR_LIMIT 256


bool print_line(int file){      //zakładamy, ze plik już został otwarty
    char c;
    while(read(file, &c, 1) == 1 && c != '\n'){
        printf("%c", c);
    }
    printf("\n");
    return c != '\n';  //nie zakończyliśmy na znaku nowej linii => mamy koniec pliku
}

int main(int argc, char** argv){
    // wczytywanie nazw pliku
    char** f_names = calloc(2, sizeof(char*));
    f_names[0] = calloc(STR_LIMIT, sizeof(char));
    f_names[1] = calloc(STR_LIMIT, sizeof(char));

    if (argc > 2){
        strcpy(f_names[0], argv[1]);
        strcpy(f_names[1], argv[2]);
    } 
    else{
        for (int i = 0; i < 2; i++){
            printf("Podaj nazwe %d. pliku:\n", i+1);
            fgets(f_names[i], STR_LIMIT, stdin);
            f_names[i][strcspn(f_names[i], "\n")] = 0; //usunięcie znaku białej linii
        }
    }

    //otwieranie plików
    int fp[2];
    for (int i = 0; i < 2; i++){
        fp[i] = open(f_names[i], O_RDONLY);
        if (fp[i] == -1){
            printf("Unable to open file %s", f_names[i]);
            if (i == 1){
                close(fp[0]);
            }
            free(f_names[0]);
            free(f_names[1]);
            free(f_names);
            exit(EXIT_FAILURE);
        }
    }

    bool end_of_file[2];    //sprawdzamy, czy plik się już zakończył
    end_of_file[0] = end_of_file[1] = false;   
    int i = 0;
    while (!end_of_file[0] && !end_of_file[1]) { 
        end_of_file[i] = print_line(fp[i]);
        i = (i+1)%2;
    }
    while(!end_of_file[i]){
        end_of_file[i] = print_line(fp[i]);
    }

    //zwalnianie pamięci i zamykanie plików
    for (int i = 0; i < 2; i++){
        close(fp[i]);
        free(f_names[i]);
    }
    free(f_names);
    return 0;
}