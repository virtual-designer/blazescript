#ifndef __STACK_H__
#define __STACK_H__

#include "runtimevalues.h"

typedef struct {
    size_t size;
    size_t si;
    runtime_val_t *array;
} stack_t;

stack_t stack_create(size_t size);
void stack_free(stack_t *stack);
void stack_push(stack_t *stack, runtime_val_t value);
void stack_print(stack_t *stack);
runtime_val_t stack_pop(stack_t *stack);

#endif