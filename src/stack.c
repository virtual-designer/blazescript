/*
 * Created by rakinar2 on 9/2/23.
 */

#include "stack.h"
#include "alloca.h"
#include "utils.h"
#include <stdlib.h>

blaze_stack_t blaze_stack_create(size_t initial_size)
{
    blaze_stack_t stack = {
        .size = initial_size,
        .bytes = xcalloc(1, initial_size),
        .index = 0
    };

    return stack;
}

static void stack_check_realloc(blaze_stack_t *stack)
{
    if (stack->index >= stack->size)
    {
        stack->size += 50;
        stack->bytes = xrealloc(stack->bytes, sizeof (uint8_t) * (stack->size));
    }
}

uint8_t *blaze_stack_push_byte(blaze_stack_t *stack, uint8_t byte)
{
    stack_check_realloc(stack);
    stack->bytes[stack->index++] = byte;
    return stack->bytes + stack->index - 1;
}

uint8_t blaze_stack_pop_byte(blaze_stack_t *stack)
{
    if (stack->index == 0)
        fatal_error("Stack is empty");

    return stack->bytes[--stack->index];
}

void blaze_stack_free(blaze_stack_t *stack)
{
    free(stack->bytes);
    stack->size = 0;
    stack->index = 0;
}