#ifndef LAB1_LIBRARY_H
#define LAB1_LIBRARY_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct block {      //struktura na blok wierszy
    char **lines;   //wiersze
    char *file_names[2];
    size_t size;
    int longest_line;
} block;

typedef struct main_table { //struktura na główną tablicę
    struct block **blocks;
    size_t max_size;
    size_t current_size;
} main_table;


//create_table rozmiar
main_table* create_main_table(size_t size);

//merge_files no_of_pairs file1a.txt:file1b.txt file2a.txt:file2b.txt ...
block* define_sequence(char *name_sequence);

FILE* merge(block *block);

int pin_new_block(main_table *table, block* block, FILE* tmp);

size_t no_of_lines_in_block(block *lines_block);

//remove_row block_index row_index
void delete_line(main_table* table, int block_ind, int del_ind);

//remove_block index
void delete_block(main_table *table, int del_ind);

//remove_table
void delete_table(main_table *table);

//print_merged
void print_merged_files(main_table *table);

#endif
