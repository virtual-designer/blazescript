/*
 * Created by rakinar2 on 8/26/23.
 */

#ifndef BLAZESCRIPT_LIB_H
#define BLAZESCRIPT_LIB_H

#include "datatype.h"
#include "scope.h"

#define BUILTIN_FN_REF(name) blaze_builin_fn__##name
#define BUILTIN_FN(name) val_t blaze_builin_fn__##name(scope_t *scope, size_t argc, val_t *args)

BUILTIN_FN(println);
BUILTIN_FN(print);
BUILTIN_FN(array);
BUILTIN_FN(array_filter);
BUILTIN_FN(read);
BUILTIN_FN(exit);

struct builtin_function
{
    const char *name;
    val_t (*callback)(scope_t *scope, size_t argc, val_t *args);
};

static struct builtin_function const builtin_functions[] = {
    { "println", BUILTIN_FN_REF(println) },
    { "print", BUILTIN_FN_REF(print) },
    { "vector", BUILTIN_FN_REF(array) },
    { "array_filter", BUILTIN_FN_REF(array_filter) },
    { "read", BUILTIN_FN_REF(read) },
    { "exit", BUILTIN_FN_REF(exit) },
};

#endif /* BLAZESCRIPT_LIB_H */
