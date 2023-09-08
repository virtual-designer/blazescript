/*
 * Created by rakinar2 on 8/26/23.
 */

#include "include/lib.h"
#include "alloca.h"
#include "datatype.h"
#include "eval.h"
#include "utils.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BUILTIN_FN(exit)
{
    if (argc >= 1 && args[0].type != VAL_INTEGER)
    {
        eval_fn_error = blaze_strdup("#1 argument passed to function exit() must be an integer");
        return *scope->null;
    }

    exit(argc >= 1 ? (int) (args[0].intval & 0xFF) : 0);
}

BUILTIN_FN(println)
{
    val_t ret = BUILTIN_FN_REF(print)(scope, argc, args);
    printf("\n");
    return ret;
}

BUILTIN_FN(print)
{
    if (argc == 0)
    {
        eval_fn_error = blaze_strdup("function println() requires at least 1 argument to be passed");
        return *scope->null;
    }

    for (size_t i = 0; i < argc; i++)
    {
        if (args[i].type == VAL_STRING)
            printf("%s", args[i].strval);
        else
            print_val_internal(&args[i], false);

        if (i != (argc - 1))
            printf(" ");
    }

    return *scope->null;
}

BUILTIN_FN(array)
{
    val_t val = val_create(VAL_ARRAY);

    for (size_t i = 0; i < argc; i++)
    {
        vector_push(val.arrval, (void *) val_copy_deep(&args[i]));
    }

    return val;
}

BUILTIN_FN(read)
{
    val_t val = val_create(VAL_STRING);
    char *line = NULL;
    size_t n = 0;

    if (blaze_getline(&line, &n, stdin) < 0)
    {
        char errstr[1024];
        sprintf(errstr, "read(): failed to read from stdin: %s", strerror(errno));
        eval_fn_error = blaze_strdup(errstr);
        return *scope->null;
    }

    for (ssize_t i = (ssize_t) n - 1; i >= 0; i--)
    {
        if (line[i] == '\n')
        {
            line[i] = 0;
            break;
        }
    }

    val.strval = line;
    return val;
}

static bool array_filter_call_fn(scope_t *scope, val_t *fn, val_t *test_val)
{
    scope_t *new_scope = fn->fnval->scope;
    val_t ret = *scope->null;

    assert(fn->type == VAL_FUNCTION && fn->fnval->type == FN_USER_CUSTOM && "Invalid callback");

    for (size_t i = 0; i < fn->fnval->param_count; i++)
    {
        scope_declare_identifier(new_scope, fn->fnval->param_names[i], *test_val, true);
    }

    for (size_t i = 0; i < fn->fnval->size; i++)
    {
        ret = eval(new_scope, fn->fnval->custom_body[i]);
    }

    fn->fnval->scope = scope_init(scope);
    return ret.type == VAL_BOOLEAN && ret.boolval;
}

BUILTIN_FN(array_filter)
{
    if (argc != 2)
    {
        eval_fn_error = blaze_strdup("function array_filter() requires exactly 2 arguments (vector, function) to be passed");
        return *scope->null;
    }

    val_t array = args[0];
    val_t callback = args[1];

    if (callback.fnval->param_count > 1)
    {
        eval_fn_error = blaze_strdup("callback function passed to array_filter() must accept less than 2 arguments");
        return *scope->null;
    }

    val_t new_array = val_create(VAL_ARRAY);

    for (size_t i = 0; i < array.arrval->length; i++)
    {
        if (array_filter_call_fn(scope, &callback, val_copy_deep(array.arrval->data[i])))
            vector_push(new_array.arrval,
                        (void *)(array.arrval->data[i]));
    }

    return new_array;
}