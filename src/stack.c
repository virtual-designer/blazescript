#include <stdio.h>

#include "stack.h"
#include "xmalloc.h"
#include "utils.h"

bstack_t stack_create(size_t size) 
{
    return (bstack_t) {
        .size = size,
        .si = 0,
        .array = xcalloc(sizeof (runtime_val_t), size)
    };
}

void stack_free(bstack_t *stack)
{
    free(stack->array);
}

void stack_push(bstack_t *stack, runtime_val_t value)
{
    stack->array[stack->si++] = value;
}

void stack_print(bstack_t *stack)
{
    puts("* STACK DUMP");

    for (size_t i = 0; i < stack->size; i++)
    {
        printf("%04lx: ", i);

        if (stack->array[i].type == VAL_NUMBER)
        {
            if (stack->array[i].is_float)
                printf("%Lf", stack->array[i].floatval);
            else
                printf("%lld", stack->array[i].intval);
        }
        else if (stack->array[i].type == VAL_STRING)
            printf("\"%s\"", stack->array[i].strval);
        else 
            printf("[Unknown %d]", stack->array[i].type);

        if ((stack->si - 1) == i)
            printf("  <==");

        printf("\n");
    }
}

runtime_val_t stack_pop(bstack_t *stack)
{
    if (stack->si == 0)
        utils_error(true, "Cannot pop stack as it's empty!");
    
    return stack->array[--stack->si];
}
