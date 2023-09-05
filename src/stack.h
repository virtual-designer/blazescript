/*
 * Created by rakinar2 on 9/2/23.
 */

#ifndef BLAZESCRIPT_STACK_H
#define BLAZESCRIPT_STACK_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *bytes;
    size_t size;
    size_t index;
} stack_t;

stack_t stack_create(size_t initial_size);
uint8_t *stack_push_byte(stack_t *stack, uint8_t byte);
uint8_t stack_pop_byte(stack_t *stack);
void stack_free(stack_t *stack);

#endif /* BLAZESCRIPT_STACK_H */
