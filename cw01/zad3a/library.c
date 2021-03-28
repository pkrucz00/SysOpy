#include "library.h"

main_table* create_main_table(size_t size){
    main_table* table = calloc(1, sizeof(main_table));
    table -> blocks = calloc(size, sizeof(block*));
    table -> max_size = size;
    table -> current_size = 0;
    return table;
}

block* define_sequence(char *name_sequence){
    block* new_block = (block*) calloc(1, sizeof(block));
    
    char* filename1 = strtok(name_sequence, ":");
    char* filename2 = strtok(NULL, "");
    
    new_block -> file_names[0] = filename1;
    new_block -> file_names[1] = filename2;
    new_block -> longest_line = 0;
    return new_block;
}

FILE* merge(block *block){
    FILE* tmp_file = tmpfile();
    if (tmp_file == NULL){
        printf("Unable to create a temporary file");
        exit(EXIT_FAILURE);
    }
    
    FILE *fp[2];
    for (int i = 0; i < 2; i++){
        fp[i] = fopen(block->file_names[i], "r");
        if (fp[i] == NULL){
            printf("Unable to open file %s", block->file_names[i]);
            exit(EXIT_FAILURE);
        }
    }

    char *buff = NULL;
    size_t len = 0;     
    ssize_t read;       //length of the read line

    size_t size = 0;    //number of read lines
    int i = 0;
    while ((read = getline(&buff, &len, fp[i])) != -1 ) { 
        if (buff[read-1] != '\n'){
            strcat(buff, "\n");
        }
        fputs(buff, tmp_file);
        if (block -> longest_line < strlen(buff) + 1)    block->longest_line = strlen(buff) + 1;
        size++;

        i = (i+1)%2;
    }
    block -> size = size;

    for (int i = 0; i < 2; i++){
        fclose(fp[i]);
    }
    free(buff);
    rewind(tmp_file);

    return tmp_file;
}

int pin_new_block(main_table *table, block* block, FILE* tmp){
    //"filling" the block
    char *buff = NULL;
    size_t len = 0;     //length of the read line
    ssize_t read;

    block -> lines = calloc(block -> size, sizeof(char*));

    int i = 0;
    while ((read = getline(&buff, &len, tmp)) != -1 ) {
        if (buff != NULL){
            block->lines[i] = calloc(block->longest_line, sizeof(char));
            strcpy(block->lines[i], buff);
        }
        i++;
    }
    free(buff);
    fclose(tmp);

    //attaching it to the table
    int block_index = table -> current_size;
    table -> blocks[block_index] = block;
    table -> current_size += 1;

    return block_index;
}

size_t no_of_lines_in_block(block* block){
    return block -> size;
}

void delete_line(main_table* table, int block_ind, int ind){
    block* given_block = table->blocks[block_ind];
    char* line_to_be_removed = given_block->lines[ind];

    for (int i = ind; i < given_block->size - 1; i++){
        given_block->lines[i] = given_block->lines[i+1];
    }
    free(line_to_be_removed);
    given_block->size -= 1;
}

void delete_block(main_table* table, int ind){
    block* block_to_be_removed = table->blocks[ind];
    for (int i = block_to_be_removed->size - 1; i >= 0; i--){
        delete_line(table, ind, i);
    }
    free(block_to_be_removed->lines);
    free(block_to_be_removed);

    for (int i = ind; i < table->current_size - 1; i++){
        table->blocks[i] = table->blocks[i+1];
    }
    table->current_size -= 1;
    
}

void delete_table(main_table* table){
    for (int i = table->current_size - 1; i >= 0; i--){
        delete_block(table, i);
    }
    free(table->blocks);
    free(table);
}

void print_merged_files(main_table* table){
    if (table == NULL){
        printf("ERROR - table not allocated. Try \"create_table size\" first\n");
        exit(EXIT_FAILURE);
    }
    else{
        for (int i = 0; i < table->current_size; i++){
            printf("Block number %d:\n", i);
            block* block = table->blocks[i]; 
            for (int j = 0; j < block->size; j++){
                printf("%s", block->lines[j]);
            }
            printf("\n");
        }
    }
}

