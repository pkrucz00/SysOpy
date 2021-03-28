#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_LIMIT 256



char *str_replace(char *orig, char *n1, char *n2) {
    char *result; 
        
    int len_n1 = strlen(n1);  
    int len_n2 = strlen(n2); 

    if (len_n1 == 0)
        return NULL;    //zakładamy, ze znak pusty zwraca NULL - nieprawidłowa operacja
    
    // zliczenie wszystkich podłańcuchów (dla dobrego oszacowania potrzebnej pamięci)
    char *s_char;   // wskazanie na pierwszy znak podłańcucha
    char *tmp;      // tutaj - wskazanie na pierwszy znak ZA znalezionym podłańcuchem
    tmp = orig;
    s_char = strstr(tmp, n1);
    int acc = 0;
    while (s_char != NULL){
        tmp = s_char + len_n1;
        acc++;
        s_char = strstr(tmp, n1);
    }

    //alokacja pamięci na wynikowy łańcuch
    result = malloc(strlen(orig) + (len_n2 - len_n1) * acc + 1);
    tmp = result;

    if (result == NULL)
        return NULL;
    
    // właściwa pętla
    //    tmp wskazuje na ostatni znak 
    int remainder;   //odległość między znalezionym podłańcuchem, a końcem podłańcucha z poprzedniej iteracji
    char* orig_cpy = orig;  //kolejny wskaźnik na wejściowy łańcuch (by nie zgubić oryginału)
                            //wskazuje na pierwszy znak za przetworzonym w poprzedniej iteracji podłańcuchem n1 w orig
    while (acc--) {
        //znalezienie podłańcucha
        s_char = strstr(orig, n1);
        remainder = s_char - orig_cpy;

        //kopiowanie znaków przed wzorcem n1
        strncpy(tmp, orig_cpy, remainder);
        tmp += remainder;

        // kopiowanie znaków z n2 w miejsce n1
        strcpy(tmp, n2);
        tmp += len_n2;

        //przesunięcie o liczbę wklejonych znaków
        orig_cpy += remainder + len_n1;
    }
    strcpy(tmp, orig_cpy); //kopiowanie "ogonka" (bądź całości, gdy wzorca nie znaleziono)
    return result;
}

int main(int argc, char** argv){
    if (argc < 4){
        printf("Too few arguments");
        exit(EXIT_FAILURE);
    }

    char* src_name = strdup(argv[1]);
    char* dest_name = strdup(argv[2]);
    char* n1 = strdup(argv[3]);
    char* n2 = strdup(argv[4]);

    FILE* src_file = fopen(src_name, "r");
    FILE* dest_file = fopen(dest_name, "w");

    char *buff = NULL;
    size_t len = 0;     
    ssize_t read;
    while ((read = getline(&buff, &len, src_file)) != -1 ) { 
        char* changed_line = str_replace(buff, n1, n2);
        fprintf(dest_file, "%s", changed_line);
        free(changed_line);
    }
    free(buff);

    fclose(src_file);
    fclose(dest_file);

    free(src_name);
    free(dest_name);
    free(n1);
    free(n2);

    return 0; 

}