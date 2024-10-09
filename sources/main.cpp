#include <stdio.h>

#include "stack.h"

#define JUST_DUMP 1

int main() {
    stack_t stk = {};
    freopen("text_file/unit_test.txt", "rt", stdin);
    char action = 0;
    size_t range = 0;
    stack_elem_t item = 0;
    while (scanf("%c", &action) && '#' != action) {
        printf("%c\n", action);
        switch(action) {
            case 'C':
                STACK_CTOR(&stk);
                break;
            case 'D':
                stack_dtor(&stk);
                break;
            case 'p':
                stack_pop(&stk, &item);
                printf(STACK_ELEM_SPEC "\n", item);
                break;
            case 'P':
                scanf("%llu", &range);
                for (size_t i = 0; i < range; ++i) {
                    stack_pop(&stk, &item);
                    printf(STACK_ELEM_SPEC "\n", item);
                }
                break;
            case 'd':
                stack_print(&stk);
                break;
            case 'a':
                scanf(STACK_ELEM_SPEC, &item);
                stack_push(&stk, item);
                break;
            case 'A':
                scanf("%llu", &range);
                scanf(STACK_ELEM_SPEC, &item);
                for(size_t i = 0; i < range; ++i)
                    stack_push(&stk, item);
                break;
            default:
                break;
        }
    }
}
