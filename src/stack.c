#include <stdio.h>

#include "stack.h"
#include "xmalloc.h"
#include "utils.h"

stack_t stack_create(size_t size) 
{
    return (stack_t) {
        .size = size,
        .si = 0,
        .array = xcalloc(sizeof (stack_element_t), size)
    };
}

void stack_free(stack_t *stack)
{
    free(stack->array);
}

void stack_push(stack_t *stack, stack_element_t value)
{
    stack->array[stack->si++] = value;
}

void stack_print(stack_t *stack)
{
    for (size_t i = 0; i < stack->size; i++)
    {
        printf("%04lx: ", i);

        if (stack->array[i].type == ST_VAL_INT)
            printf("%lld", stack->array[i].intval);
        else if (stack->array[i].type == ST_VAL_STRING)
            printf("%s", stack->array[i].strval);
        else 
            printf("[Unknown]");

        if (stack->si == i)
            printf("  <==");

        printf("\n");
    }
}

stack_element_t stack_pop(stack_t *stack)
{
    if (stack->si == 0)
        utils_error(true, "Cannot pop stack as it's empty!");
    
    return stack->array[--stack->si];
}