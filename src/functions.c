#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "runtimevalues.h"
#include "scope.h"
#include "vector.h"
#include "map.h"
#include "functions.h"
#include "eval.h"

static runtime_val_t __native_null()
{
    return (runtime_val_t) { .type = VAL_NULL };
}

/* The `NATIVE_FN()` macro takes the base name of the native function,
   and declares a function prefixed with __native_ and suffixed with _fn.
   The function will take the following arguments:

     - vector_t args
     - scope_t *scope 
    
   These arguments can be used anywhere in the function body. */

NATIVE_FN(println)
{
    for (size_t i = 0; i < args.length; i++)
    {
        runtime_val_t arg = VEC_GET(args, i, runtime_val_t);
        handle_result(&arg, true, 1, false);
    }
    
    VEC_FREE(args);
    return __native_null();
}

NATIVE_FN(print)
{
    for (size_t i = 0; i < args.length; i++)
    {
        runtime_val_t arg = VEC_GET(args, i, runtime_val_t);
        handle_result(&arg, false, 1, false);
    }
    
    VEC_FREE(args);
    return __native_null();
}

NATIVE_FN(pause)
{
    if (args.length != 0) 
        eval_error(true, "pause() does not accept any parameters");
    
    VEC_FREE(args);

    pause();
    return __native_null();
}

NATIVE_FN(read)
{
    if (args.length > 1) 
        eval_error(true, "read() accepts only 1 optional parameter");

    if (args.length == 1) 
    {
        if (VEC_GET(args, 0, runtime_val_t).type != VAL_STRING)
            eval_error(true, "Parameter #1 of read() must be a String");
        
        printf("%s", VEC_GET(args, 0, runtime_val_t).strval);
    }

    char *line = NULL;
    size_t n = 0;

    getline(&line, &n, stdin);

    line[strlen(line) - 1] = '\0';
    
    VEC_FREE(args);

    return (runtime_val_t) {
        .type = VAL_STRING,
        .strval = line
    };
}

NATIVE_FN(typeof)
{
    if (args.length != 1)
        eval_error(true, "typeof() expects exactly 1 parameter");

    runtime_val_t val = {
        .type = VAL_STRING
    };

    switch (VEC_GET(args, 0, runtime_val_t).type)
    {
        case VAL_STRING:
            val.strval = strdup("String");
            break;

        case VAL_NUMBER:
            val.strval = strdup(val.is_float ? "Number (Float)" : "Number (Integer)");
            break;

        case VAL_BOOLEAN:
            val.strval = strdup("Boolean");
            break;

        case VAL_NATIVE_FN:
            val.strval = strdup("Native Function");
            break;

        case VAL_NULL:
            val.strval = strdup("NULL");
            break;

        case VAL_OBJECT:
            val.strval = strdup("Object");
            break;

        case VAL_USER_FN:
            val.strval = strdup("Function");
            break;

        default:
            val.strval = strdup("Unknown");
            break;
    }
    
    VEC_FREE(args);
    return val;
}
