#pragma once
#include <stddef.h>

typedef struct {
    void **storage;
    size_t size;
    size_t capacity;
} vector;

void vec_init(vector *v);
void vec_clear(vector *v);

void vec_insert(vector *v, size_t at, void *value);
void vec_push_back(vector *v, void *value);

void *vec_erase(vector *v, size_t at);
void *vec_pop_back(vector *v);
