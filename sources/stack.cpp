#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <mem.h>
#include <string.h>

#include "stack.h"
#include "commoner.h"

#define MIN_CAPACITY 8 //must be dividible by 8
#define MAX_CAPACITY 1 << 25
#define HASH_BASE (uint64_t)1e9 + 7
#define HASH_MOD  1791791791

enum ERROR_t {
    PRINT_STACK            = 1<<0,
    ZERO_STACK_PTR         = 1<<1,
    ZERO_DATA_PTR          = 1<<2,
    CORRUPTED_STACK_CANARY = 1<<3,
    CORRUPTED_DATA_CANARY  = 1<<4,
    CORRUPTED_STACK_HASH   = 1<<5,
    CORRUPTED_DATA_HASH    = 1<<6,
    CAPACITY_TOO_BIG       = 1<<7,
    SIZE_OVER_CAPACITY     = 1<<8,
};

#define STACK_VALID(stk) {\
    int assert_result_ = stack_assert(stk);\
    assert(assert_result_);                \
    if (!assert_result_) return 1;         \
}

#ifdef HASH_PROT
static uint64_t calculate_hash(const void* begin, const void* end) {
    assert(!((uint64_t)begin % 8));
    assert(!((uint64_t)end   % 8));

    uint64_t  curr_hash = 1;
    const uint64_t* curr_ptr  = (const uint64_t*)begin;

    while (curr_ptr != end) {
        curr_hash = (HASH_BASE * curr_hash + (*curr_ptr + 1)) % HASH_MOD;
        curr_ptr++;
    }
    return curr_hash;
}
static uint64_t calculate_stack_hash(const stack_t* stk) {
    assert(stk);
    const void* begin        = (const char*)stk ON_CANARY_PROT(+ ceil_mod_8(sizeof(canary_t)));
    const void* begin_ignore = &stk->stack_hash;
    const void* end_ignore   = &stk->data_hash;
    const void* end          = (const char*)&stk->data + sizeof(stack_t) ON_CANARY_PROT(- 2 * ceil_mod_8(sizeof(canary_t)));

/*
    printf("0x%llx\n0x%llx\n0x%llx\n0x%llx\n\n",
        (uint64_t)begin,
        (uint64_t)begin_ignore,
        (uint64_t)end_ignore,
        (uint64_t)end
    );
*/

    return calculate_hash(begin, begin_ignore) ^ (HASH_BASE * calculate_hash(end_ignore, end));
}
static uint64_t calculate_data_hash (const stack_t* stk) { //todo hash
    assert(stk);
    assert(stk->data);
    const void* begin = (char*)stk->data;
    const void* end   = (char*)stk->data + stk->capacity * sizeof(stack_elem_t);

    return calculate_hash(begin, end);
}
static int reset_hash(stack_t* stk) {
    assert(stk);
    assert(stk->data);
    stk->data_hash  = calculate_data_hash(stk);
    stk->stack_hash = calculate_stack_hash(stk);

    return 0;
}
#endif // HASH_PROT

#ifdef CANARY_PROT
static canary_t* get_left_data_canary(const stack_t* stk) {
    assert(stk);
    if (!stk->data) return 0;

    return (canary_t*)((char*)stk->data - ceil_mod_8(sizeof(canary_t)));
}
static canary_t* get_right_data_canary(const stack_t* stk) {
    assert(stk);
    if (!stk->data) return 0;

    return (canary_t*)((char*)stk->data + sizeof(stack_elem_t) * stk->capacity);
}
#endif // CANARY_PROT

static int stack_recalloc(stack_t* stk) {
    assert(stk);

    size_t new_size  = stk->capacity * sizeof(stack_elem_t);

    #ifdef CANARY_PROT
        size_t canary_size = ceil_mod_8(sizeof(canary_t));
        if(stk->data) stk->data = (stack_elem_t* )((char*)stk->data - canary_size);
        new_size += 2 * canary_size;
    #endif // CANARY_PROT

//    printf("%llu 0x%llx\n", new_size, (uint64_t)stk->data);

    stk->data = (stack_elem_t*)realloc(stk->data, new_size);

//    printf("%llu 0x%llx\n", new_size, (uint64_t)stk->data);

    ON_CANARY_PROT(stk->data = (stack_elem_t* )((char*)stk->data + canary_size));

//    printf("%llu 0x%llx\n", new_size, (uint64_t)stk->data);

    memset(stk->data + stk->size, 0, (stk->capacity - stk->size) * sizeof(stack_elem_t));

    #ifdef CANARY_PROT
        if (stk->data) {
            *get_left_data_canary(stk)  = LEFT_DATA_CANARY_VAL;
            *get_right_data_canary(stk) = RIGHT_DATA_CANARY_VAL;
        }
    #endif // CANARY_PROT

    return 0;
}

static uint32_t stack_dump(const stack_t* stk, uint32_t error_vector) {
    if (error_vector == 0) return 0;

    uint32_t stack_error = 0;

    LOG_INFO("PRINTS STACK\n");

    if (error_vector & ZERO_STACK_PTR) {
        LOG_FATAL("PTR TO STACK = 0\n");
        return ZERO_STACK_PTR;
    }

    {
    const uint64_t STACK = (uint64_t)stk;
    PRINT_BLOCK("", STACK);
    LOG_BLUE("{\n");
    }

    #ifdef DEBUG
        {
            char* PARAMETER_NAME = stk->param_info.param_name;
            char* FROM_FILE      = stk->param_info.file_name;
            size_t DECLARED_IN_LINE    =  stk->param_info.file_line;
            PRINT_PARAM("\t", "%s",   FROM_FILE, " ");
            PRINT_PARAM("\t", "%llu", DECLARED_IN_LINE, " ");
            PRINT_PARAM("\t", "%s",   PARAMETER_NAME, " ");
        }
    #endif // DEBUG

    #ifdef CANARY_PROT
        {
        const uint64_t LEFT__STACK_CANARY_ = stk->left_canary;
        const uint64_t RIGHT_STACK_CANARY_ = stk->right_canary;
        PRINT_PARAM("\t", "0x%llx",  LEFT__STACK_CANARY_, " = ");
        PRINT_PARAM("\t",  "0x%llx", RIGHT_STACK_CANARY_, " = ");

        if(error_vector & CORRUPTED_STACK_CANARY) {
            stack_error |= CORRUPTED_STACK_CANARY;
            LOG_FATAL("CORRUPTED STACK CANARY\n");
            if(LEFT__STACK_CANARY_  != LEFT_STACK_CANARY_VAL)
                LOG_FATAL("LEFT  STACK CANARY != 0x%llx\n", (uint64_t) LEFT_STACK_CANARY_VAL);
            if(RIGHT_STACK_CANARY_ != RIGHT_STACK_CANARY_VAL)
                LOG_FATAL("RIGHT STACK CANARY != 0x%llx\n", (uint64_t)RIGHT_STACK_CANARY_VAL);
        }
        }
        if (stk->data) {
        const uint64_t LEFT__DATA__CANARY_ = (uint64_t)*get_left_data_canary(stk);
        const uint64_t RIGHT_DATA__CANARY_ = (uint64_t)*get_right_data_canary(stk);
        PRINT_PARAM("\t", "0x%llx",  LEFT__DATA__CANARY_, " = ");
        PRINT_PARAM("\t",  "0x%llx", RIGHT_DATA__CANARY_, " = ");

        if(error_vector & CORRUPTED_DATA_CANARY) {
            stack_error |= CORRUPTED_DATA_CANARY;
            LOG_FATAL("CORRUPTED DATA CANARY\n");
            if(LEFT__DATA__CANARY_  != LEFT_DATA_CANARY_VAL)
                LOG_FATAL("LEFT  DATA  CANARY != 0x%llx\n", (uint64_t) LEFT_DATA_CANARY_VAL);
            if(RIGHT_DATA__CANARY_ != RIGHT_DATA_CANARY_VAL)
                LOG_FATAL("RIGHT DATA  CANARY != 0x%llx\n", (uint64_t)RIGHT_DATA_CANARY_VAL);
        }
        }
    #endif // CANARY_PROT

    #ifdef HASH_PROT
        {
        const uint64_t STACK_HASH = stk->stack_hash;
        PRINT_PARAM("\t", "0x%llx", STACK_HASH, " = ");

        if(error_vector & CORRUPTED_STACK_HASH) {
            stack_error |= CORRUPTED_STACK_HASH;
            uint64_t TRUE_STACK_HASH = calculate_stack_hash(stk);
            LOG_FATAL("CURRUPTED STACK HASH\n" "HASH OF STACK != 0x%llx\n", TRUE_STACK_HASH);
        }
        }
        if (stk->data){
        const uint64_t DATA__HASH = stk->data_hash;
        PRINT_PARAM("\t", "0x%llx", DATA__HASH, " = ");

        if(error_vector & CORRUPTED_DATA_HASH) {
            stack_error |= CORRUPTED_DATA_HASH;
            const uint64_t TRUE_DATA_HASH = calculate_data_hash(stk);
            LOG_FATAL("CURRUPTED DATA  HASH\n" "HASH OF DATA != 0x%llx\n", TRUE_DATA_HASH);
        }
        }
    #endif // HASH_PROT
/*
    if (stack_error) {
        LOG_BLUE("}\n");
        return stack_error;
    }
*/
    { // stack inner
    uint64_t capacity = stk->capacity;
    uint64_t size = stk->size;
    PRINT_PARAM("\t", "%llu", capacity, " = ");
    PRINT_PARAM("\t", "%llu", size, " = ");
    }

    if (stack_error |= error_vector & SIZE_OVER_CAPACITY)
        LOG_FATAL("CAPACITY <= SIZE\n");
    if (stack_error |= error_vector & CAPACITY_TOO_BIG)
        LOG_FATAL("CAPACITY IS TOO BIG\n");
    if (stack_error |= error_vector & ZERO_DATA_PTR)
        LOG_FATAL("PTR TO DATA = 0\n");

    if (stack_error) {
        LOG_BLUE("}\n");
        return stack_error;
    }

    {
    uint64_t data = (uint64_t)(stk->data);
    PRINT_BLOCK("\t", data);
    }
    {
    const stack_elem_t* data = (stk->data);
    LOG_BLUE("\t{\n");
    for(size_t i = 0; i < stk->size; ++i)
        PRINT_ARR_ELEM("\t\t", STACK_ELEM_SPEC, data, i);
    LOG_BLUE("\t}\n");
    }

    LOG_BLUE("}\n");

    return stack_error;
}
uint32_t stack_verify(const stack_t* stk) {
    uint32_t stack_error = 0;
    if (!stk) return ZERO_STACK_PTR;

    #ifdef CANARY_PROT
        if ((stk->left_canary  != LEFT_STACK_CANARY_VAL) ||
            (stk->right_canary != RIGHT_STACK_CANARY_VAL))
            stack_error |= CORRUPTED_STACK_CANARY;

        if (stk->data)
        if (*get_left_data_canary(stk)  != LEFT_DATA_CANARY_VAL ||
            *get_right_data_canary(stk) != RIGHT_DATA_CANARY_VAL)
            stack_error |= CORRUPTED_DATA_CANARY;
    #endif // CANARY_PROT

    #ifdef HASH_PROT
        if (stk->stack_hash != calculate_stack_hash(stk))
            stack_error |= CORRUPTED_STACK_HASH;

        if (stk->data)
        if (stk->data_hash  != calculate_data_hash(stk))
            stack_error |= CORRUPTED_DATA_HASH;
    #endif // HASH_PROT

    if (stk->capacity <= stk->size)   stack_error |= SIZE_OVER_CAPACITY;
    if (stk->capacity > MAX_CAPACITY) stack_error |= CAPACITY_TOO_BIG;
    if (!stk->data)                   stack_error |= ZERO_DATA_PTR;

    return stack_error;
}

static int stack_assert(const stack_t* stk) {
    return !stack_dump(stk, stack_verify(stk));
}

#ifdef DEBUG
static int debug_ctor(debug_t* deb DEBUG_PARAMS) {
    assert(deb);

    deb->file_name  = (char*)calloc(strlen(file_name)  + 1, sizeof(char));
    deb->param_name = (char*)calloc(strlen(param_name) + 1, sizeof(char));
    if(!deb->param_name) return 1;
    if(!deb->file_name)  return 1;

    strcpy(deb->file_name,  file_name);
    strcpy(deb->param_name, param_name);
    deb->file_line  =  file_line;

//    *deb = {&file_name_, &param_name_, file_line};

    return 0;
}
static int debug_dtor(debug_t* deb) {
    assert(deb);

    free((void*)deb->param_name);
    free((void*)deb->file_name);
    *deb = {};

    return 0;
}
#endif // DEBUG

int stack_ctor (stack_t* stk DEBUG_PARAMS) {
    assert(stk);
    *stk = {};

    ON_DEBUG(debug_ctor(&stk->param_info, file_name, file_line, param_name));


    #ifdef CANARY_PROT
        stk->left_canary  =  LEFT_STACK_CANARY_VAL;
        stk->right_canary = RIGHT_STACK_CANARY_VAL;
    #endif // CANARY_PROT
    stk->capacity = MIN_CAPACITY;
    stack_recalloc(stk);


    ON_HASH_PROT(reset_hash(stk));

    STACK_VALID(stk);

    return 0;
}
int stack_dtor(stack_t* stk) {
    STACK_VALID(stk);
    ON_DEBUG(debug_dtor(&stk->param_info));

    size_t data_size = stk->capacity * sizeof(stack_elem_t);

    #ifdef CANARY_PROT
        size_t canary_size = ceil_mod_8(sizeof(canary_t));
        stk->data = (stack_elem_t*)((char*)stk->data - canary_size);
        data_size += 2 * canary_size;
    #endif

    memset(stk->data, 0, data_size);

    free(stk->data);
    *stk = {};

    return 0;
}

int stack_pop(stack_t* stk, stack_elem_t* item) {
    STACK_VALID(stk);

    stk->size--;
    *item = stk->data[stk->size];
    stk->data[stk->size] = 0;

    if (stk->capacity != MIN_CAPACITY && stk->capacity == stk->size / 4) {
        stk->capacity /= 2;
        stack_recalloc(stk);
    }
    ON_HASH_PROT(reset_hash(stk));

    STACK_VALID(stk);
    return 0;
}
int stack_push(stack_t* stk, stack_elem_t item) {
    STACK_VALID(stk);

    stk->data[stk->size] = item;
    stk->size++;
    if (stk->capacity != MAX_CAPACITY && stk->capacity == stk->size) {
        stk->capacity *= 2;
        stack_recalloc(stk);
    }
    ON_HASH_PROT(reset_hash(stk));

    STACK_VALID(stk);
    return 0;
}

int stack_print(stack_t* stk) {
    stack_dump(stk, stack_verify(stk) | PRINT_STACK);
    return 0;
}
