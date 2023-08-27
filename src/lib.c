/*
 * Created by rakinar2 on 8/26/23.
 */

#include "lib.h"
#include "eval.h"
#include <stdio.h>
#include <string.h>

BUILTIN_FN(test) {
    for (unsigned i = 0; i < 10; i++)
        printf("%u\n", i);
    return scope->null;
}

BUILTIN_FN(println)
{
    if (argc == 0)
    {
        eval_fn_error = strdup("function println() requires at least 1 argument to be passed");
        return NULL;
    }

    for (size_t i = 0; i < argc; i++)
    {
        if (args[i]->type == VAL_STRING)
            printf("%s", args[i]->strval->value);
        else
            print_val_internal(args[i], false);

        if (argc > 1)
            printf(" ");
    }

    printf("\n");

    return scope->null;
}

