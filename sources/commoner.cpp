#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <mem.h>

#include "commoner.h"

#define SWAP(type, x1, x2) {\
    type* a = (type*) x1;\
    type* b = (type*) x2;\
    *a ^= *b;            \
    *b ^= *a;            \
    *a ^= *b;            \
}

typedef void (*swap_t)(void* x1, void* x2);

void FREE(void* data){
    free(*(void**)(data));
    *(void**)(data) = 0;
}

uint64_t ceil_mod_8 (uint64_t num) {
    return (((num - 1) >> 3) + 1) << 3;
}

void swap_1b(void* x1, void* x2) {
    assert(x1);
    assert(x2);
    SWAP(uint8_t, x1, x2);
}

void swap_2b(void* x1, void* x2) {
    assert(x1);
    assert(x2);
    SWAP(uint16_t, x1, x2);
}

void swap_4b(void* x1, void* x2) {
    assert(x1);
    assert(x2);
    SWAP(uint32_t, x1, x2);
}

void swap_8b(void* x1, void* x2) {
    assert(x1);
    assert(x2);
    SWAP(uint64_t, x1, x2);
}

static void long_swap(void** x1, void** x2, size_t size, size_t el_size, swap_t swap) {
    assert(x1);
    assert(x2);

    while (size--) {
        swap(*x1, *x2);
        *x1 = (char*)*x1 + el_size;
        *x2 = (char*)*x2 + el_size;
    }
}

void swap_memory(void* x1, void* x2, size_t size) {
    assert(x1);
    assert(x2);
    long_swap(&x1, &x2,  size >>     3, 8, swap_8b);
    long_swap(&x1, &x2,  size &  0b111, 1, swap_1b);

    return;
}

void swap_memory_2(void* x1, void* x2, size_t size) {
    assert(x1);
    assert(x2);

    void* x3 = malloc(size);
    assert(x3);

    memcpy(x3, x1, size);
    memcpy(x1, x2, size);
    memcpy(x2, x3, size);

    FREE(&x3);

    return;
}

void* get_i(void* data, size_t index, size_t num_of_elemenst, size_t size_of_elements) {
    assert(index < num_of_elemenst);
    return (void*)((size_t)data + index * size_of_elements);
}

