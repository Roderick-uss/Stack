#ifndef COMMON
#define COMMON

#include "colors.h"
#define LOG_CERR( str, ...) fprintf(stderr, YELLOW str WHITE __VA_OPT__(,) __VA_ARGS__)
#define LOG_INFO( str, ...) fprintf(stderr, GREEN  str WHITE __VA_OPT__(,) __VA_ARGS__)
#define LOG_FATAL(str, ...) fprintf(stderr, RED    str WHITE __VA_OPT__(,) __VA_ARGS__)
#define LOG_BLUE( str, ...) fprintf(stderr, BLUE   str WHITE __VA_OPT__(,) __VA_ARGS__)
#define LOG_CYAN( str, ...) fprintf(stderr, CYAN   str WHITE __VA_OPT__(,) __VA_ARGS__)

#define ERROR_CHECK(error, expr) if (!error) error |= !expr

#define PRINT_PARAM(suff, spec, param, separator){\
    LOG_INFO(suff #param);     \
    LOG_CYAN("%s", separator); \
    LOG_CERR(spec "\n", param);\
}
#define PRINT_BLOCK(suff, ptr){\
    LOG_INFO(suff #ptr " [");\
    LOG_CERR("0x%llx", ptr); \
    LOG_INFO("]\n");         \
}
#define PRINT_ARR_ELEM(suff, spec, arr, index){\
    LOG_INFO(suff #arr "[");        \
    LOG_BLUE("%-2llu", index);        \
    LOG_INFO("]");                  \
    LOG_CYAN(" = ");                \
    LOG_CERR(spec "\n", arr[index]);\
}

struct STRING_t {
    const char* begin;
    size_t size;
};
uint64_t ceil_mod_8 (uint64_t num);
void* get_i(void* data, size_t index, size_t num_of_elemenst, size_t size_of_elements);
void swap_memory(void* x1, void* x2, size_t size);
void swap_memory_2(void* x1, void* x2, size_t size);
void swap_1b(void * x1, void* x2);
void swap_2b(void * x1, void* x2);
void swap_4b(void * x1, void* x2);
void swap_8b(void * x1, void* x2);
void FREE(void* data);

#endif // COMMON
