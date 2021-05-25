#include "vector.h"
#include <stdlib.h>
#include <string.h>

void vec_init(vector *v) {
    v->storage = NULL;
    v->size = 0;
    v->capacity = 0;
}

void vec_clear(vector *v) {
    free(v->storage);
    v->storage = NULL;
    v->size = 0;
    v->capacity = 0;
}

void vec_insert(vector *v, size_t at, void *value) {
    if (v->size == v->capacity) {
        v->capacity = v->capacity == 0 ? 1 : 2 * v->capacity;
        v->storage = reallocarray(v->storage, v->capacity, sizeof(*v->storage));
    }

    memmove(&v->storage[at + 1], &v->storage[at], sizeof(*v->storage) * (v->size - at));
    v->storage[at] = value;
    v->size += 1;
}

void vec_push_back(vector *v, void *value) {
    vec_insert(v, v->size, value);
}

void *vec_erase(vector *v, size_t at) {
    void *popped = v->storage[at];
    v->size -= 1;
    memmove(&v->storage[at], &v->storage[at + 1], sizeof(*v->storage) * (v->size - at));

    if (v->size == v->capacity / 4) {
        v->capacity = v->size == 0 ? 0 : v->capacity / 2;

        if (v->capacity == 0) {
            free(v->storage);
            v->storage = NULL;
        }
        else {
            v->storage = reallocarray(v->storage, v->capacity, sizeof(*v->storage));
        }
    }

    return popped;
}

void *vec_pop_back(vector *v) {
    return vec_erase(v, v->size - 1);
}
