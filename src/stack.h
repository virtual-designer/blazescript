#ifndef __STACK_H__
#define __STACK_H__

#include "runtimevalues.h"

typedef enum {
    ST_VAL_INT = VAL_NUMBER,
    ST_VAL_STRING = VAL_STRING
} stack_element_type_t;

typedef struct {
    stack_element_type_t type;

    union {
        long long int intval;
        char *strval;
    };
} stack_element_t;

typedef struct {
    size_t size;
    size_t si;
    stack_element_t *array;
} stack_t;

stack_t stack_create(size_t size);
void stack_free(stack_t *stack);
void stack_push(stack_t *stack, stack_element_t value);
void stack_print(stack_t *stack);
stack_element_t stack_pop(stack_t *stack);

#endif