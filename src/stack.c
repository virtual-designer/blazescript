/*
 * Created by rakinar2 on 9/2/23.
 */

#include "stack.h"
#include "alloca.h"
#include "utils.h"
#include <stdlib.h>

stack_t stack_create(size_t initial_size)
{
    stack_t stack = {
        .size = initial_size,
        .bytes = xcalloc(1, initial_size),
        .index = 0
    };

    return stack;
}

static void stack_check_realloc(stack_t *stack)
{
    if (stack->index >= stack->size)
    {
        stack->size += 50;
        stack->bytes = xrealloc(stack->bytes, sizeof (uint8_t) * (stack->size));
    }
}

uint8_t *stack_push_byte(stack_t *stack, uint8_t byte)
{
    stack_check_realloc(stack);
    stack->bytes[stack->index++] = byte;
    return stack->bytes + stack->index - 1;
}

uint8_t stack_pop_byte(stack_t *stack)
{
    if (stack->index == 0)
        fatal_error("Stack is empty");

    return stack->bytes[--stack->index];
}

void stack_free(stack_t *stack)
{
    free(stack->bytes);
    stack->size = 0;
    stack->index = 0;
}