#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "runtimevalues.h"
#include "scope.h"
#include "vector.h"
#include "functions.h"
#include "eval.h"
#include "utils.h"

void print_rtval(runtime_val_t *result, bool newline, int tabs, bool quote_strings)
{
    if (result->type == VAL_NULL)
        printf("\033[35mnull\033[0m");
    else if (result->type == VAL_BOOLEAN)
        printf("\033[34m%s\033[0m", result->boolval ? "true" : "false");
    else if (result->type == VAL_STRING)
        printf("%s" COLOR("%s", "%s") "%s", quote_strings ? COLOR("2", "\"") : "", quote_strings ? "35" : "0", result->strval, quote_strings ? COLOR("2", "\"") : "");
    else if (result->type == VAL_NUMBER)
    {
        if (result->is_float)
            printf("\033[33m%Lf\033[0m", result->floatval);
        else
            printf("\033[33m%lld\033[0m", result->intval);    
    }
    else if (result->type == VAL_OBJECT)
    {
        printf(COLOR("1;36", "[Object]") " " COLOR("1;33", "{") "\n");

        for (size_t i = 0, c = 0; i < result->properties.size; i++)
        {
            if (result->properties.array[i] == NULL)
                continue;
            
            for (int j = 0; j < tabs; j++)
                putchar('\t');

            printf("%s: ", result->properties.array[i]->key);
            print_rtval(result->properties.array[i]->value->value, false, tabs + 1, true);
            printf("%s\n", c != (result->properties.count - 1) ? "," : "");
            c++;
        }

        for (int i = 0; i < (tabs - 1); i++)
            putchar('\t');
        
        printf(COLOR("1;33", "}"));
    }
    else 
        printf("[Unknown Type: %d]", result->type);

    if (newline)
        printf("\n");
}

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

        if (arg.type == VAL_STRING)
            printf("%s", arg.strval);
        else
            print_rtval(&arg, false, 1, false);

        if (i != (args.length - 1))
            printf(" ");
    }

    printf("\n");
    fflush(stdout);

    VEC_FREE(args);
    return __native_null();
}

NATIVE_FN(print)
{
    for (size_t i = 0; i < args.length; i++)
    {
        runtime_val_t arg = VEC_GET(args, i, runtime_val_t);
        print_rtval(&arg, false, 1, false);

        if (i != (args.length - 1))
            printf(" ");
    }
    
    fflush(stdout);
    
    VEC_FREE(args);
    return __native_null();
}

NATIVE_FN(pause)
{
    if (args.length != 0) 
        eval_error(true, "pause() does not accept any parameters");
    
    VEC_FREE(args);
    
#if defined(__WIN32__)
    while (true)
        getchar();
#else
    pause();
#endif
    
    return __native_null();
}

NATIVE_FN(sleep)
{
    if (args.length != 1) 
        eval_error(true, "sleep() takes exactly 1 parameter");

    runtime_val_t number = VEC_GET(args, 0, runtime_val_t);

    if (number.type != VAL_NUMBER) 
        eval_error(true, "Parameter #1 of sleep() must be a Number");

    if ((number.is_float ? number.floatval : number.intval) < 0) 
        eval_error(true, "Parameter #1 of sleep() must be a positive number");
    
    usleep((unsigned long long int) ((number.is_float ? number.floatval : number.intval) * 1000000));

    VEC_FREE(args);    
    return __native_null();
}

#if defined(__WIN32__) && !defined(getline)
ssize_t getline(char **lineptr, FILE *stream) 
{
    *lineptr = NULL;
    ssize_t len = 0, i = 0;
    char last = 'a';
    
    while (!feof(stream) && last != '\n' && last != '\r') 
    {   
        if (len >= i)
        {
            *lineptr = xrealloc(*lineptr, (len == 0 ? 1 : len) * 2);
            len = (len == 0 ? 1 : len) * 2;
        }
        
        last = getchar();
        (*lineptr)[i] = last;
        
        i++;
    }
    
    return len;
}
#endif

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

#if defined(__WIN32__)
    getline(&line, stdin);
#else
    getline(&line, &n, stdin);
#endif

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

    runtime_val_t arg = VEC_GET(args, 0, runtime_val_t);

    switch (arg.type)
    {
        case VAL_STRING:
            val.strval = strdup("String");
            break;

        case VAL_NUMBER:
            val.strval = strdup(arg.is_float ? "Number (Float)" : "Number (Integer)");
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
