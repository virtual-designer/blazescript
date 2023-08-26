/*
 * Created by rakinar2 on 8/26/23.
 */

#ifndef BLAZESCRIPT_LIB_H
#define BLAZESCRIPT_LIB_H

#include "datatype.h"
#include "scope.h"

#define BUILTIN_FN_REF(name) blaze_builin_fn__##name
#define BUILTIN_FN(name) val_t *blaze_builin_fn__##name(scope_t *scope, size_t argc, val_t **args)

BUILTIN_FN(println);

struct builtin_function
{
    const char *name;
    val_t *(*callback)(scope_t *scope, size_t argc, val_t **args);
};

static struct builtin_function const builtin_functions[] = {
    { "println", BUILTIN_FN_REF(println) }
};

#endif /* BLAZESCRIPT_LIB_H */
