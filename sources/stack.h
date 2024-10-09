#ifndef  STACK_CLASS
#define  STACK_CLASS

#include <cmath>
#include <stdint.h>

#include "commoner.h"

#define CANARY_PROT
#define HASH_PROT
#define DEBUG

#ifdef DEBUG
#define ON_DEBUG(...)       __VA_ARGS__
#else
#define ON_DEBUG(...)
#endif

#ifdef CANARY_PROT
#define ON_CANARY_PROT(...) __VA_ARGS__
#else
#define ON_CANARY_PROT(...)
#endif

#ifdef HASH_PROT
#define ON_HASH_PROT(...)   __VA_ARGS__
#else
#define ON_HASH_PROT(...)
#endif

#define DEBUG_PARAMS \
ON_DEBUG(                  \
    ,                      \
    const char* file_name, \
    size_t file_line,         \
    const char* param_name \
)

#ifdef DEBUG
struct debug_t {
    char* file_name;
    char* param_name;
    size_t file_line;
};
#endif // DEBUG

#ifdef CANARY_PROT
enum canary_t {
    LEFT_STACK_CANARY_VAL  =  0xb1eed,
    RIGHT_STACK_CANARY_VAL =  0xdee1b,
    LEFT_DATA_CANARY_VAL   =  0xb100d,
    RIGHT_DATA_CANARY_VAL  =  0xd001b
};
#endif // CANARY_PROT

#define STACK_ELEM_SPEC "%lg"
typedef double stack_elem_t;

struct stack_t {
    ON_CANARY_PROT(canary_t left_canary;)

    stack_elem_t* data;
    size_t size, capacity;

    ON_DEBUG(debug_t param_info;);

    #ifdef HASH_PROT
        uint64_t stack_hash;
        uint64_t  data_hash;
    #endif

    ON_CANARY_PROT(canary_t right_canary;)
};

#define STACK_CTOR(stk)\
stack_ctor(      \
    stk          \
    ON_DEBUG(    \
        ,        \
        __FILE__,\
        __LINE__,\
        #stk + 1 \
    )            \
);

//uint32_t stack_dump(const stack_t* stk, uint32_t error_vector);
uint32_t stack_verify(const stack_t* stk);
int      stack_pop   (stack_t* stk, stack_elem_t* item);
int      stack_push  (stack_t* stk, stack_elem_t item);
int      stack_ctor  (stack_t* stk DEBUG_PARAMS);
int      stack_dtor  (stack_t* stk);
int      stack_print(stack_t* stk);

#endif //STACK_CLASS
