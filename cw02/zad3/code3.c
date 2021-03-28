#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STR_LIMIT 256

bool is_a_square_root(int n){
    for (int i = 0; i*i <= n; i++){
        if (i*i == n)
            return true;
    }
    return false;
}


int main(void){
    //przypisanie nazw zmiennym
    char file_name[] = "dane.txt";

    FILE* write_files[3];
    char* wf_names[3];
    wf_names[0] = "a.txt";
    wf_names[1] = "b.txt";
    wf_names[2] = "c.txt";


    //otwieranie plików
    FILE* file = fopen(file_name, "r");
    for (int i = 0; i < 3; i++){
        write_files[i] = fopen(wf_names[i], "w");
    }


    //odczyt i zapis danych
    int number = 0;
    int acc_even = 0;  //licznik parzystych liczb
    while (!feof (file)){  
        fscanf (file, "%d", &number);
        if (number % 2){
            acc_even++;
        }if ((number/10) % 10 == 7 || (number/10) % 10 == 0){
            fprintf(write_files[1], "%d\n", number);
        }if (is_a_square_root(number)){
            fprintf(write_files[2], "%d\n", number);
        }
    }
    fprintf(write_files[0], "%d\n", acc_even);


    //zamykanie wszystkich plików
    fclose (file); 
    for (int i = 0; i < 3; i++){
        fclose(write_files[i]);
    }
    return 0; 
}