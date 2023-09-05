/*
 * Created by rakinar2 on 8/26/23.
 */

#include "include/lib.h"
#include "alloca.h"
#include "eval.h"
#include "datatype.h"
#include <assert.h>
#include <stdio.h>

BUILTIN_FN(println)
{
    if (argc == 0)
    {
        eval_fn_error = blaze_strdup("function println() requires at least 1 argument to be passed");
        return *scope->null;
    }

    for (size_t i = 0; i < argc; i++)
    {
        if (args[i].type == VAL_STRING)
            printf("%s", args[i].strval->value);
        else
            print_val_internal(&args[i], false);

        if (i != (argc - 1))
            printf(" ");
    }

    printf("\n");

    return *scope->null;
}

BUILTIN_FN(array)
{
    val_t val = val_create(VAL_ARRAY);

    for (size_t i = 0; i < argc; i++)
    {
        vector_push(val.arrval->array, (void *) val_copy_deep(&args[i]));
    }

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
    return ret.type == VAL_BOOLEAN && ret.boolval->value;
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

    for (size_t i = 0; i < array.arrval->array->length; i++)
    {
        if (array_filter_call_fn(scope, &callback, val_copy_deep(array.arrval->array->data[i])))
            vector_push(new_array.arrval->array,
                        (void *)(array.arrval->array->data[i]));
    }

    return new_array;
}