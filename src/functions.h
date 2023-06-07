#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include "runtimevalues.h"
#include "scope.h"
#include "vector.h"

#define NATIVE_FN(name) runtime_val_t __native_##name##_fn(vector_t args, scope_t *scope)
#define NATIVE_FN_REF(name) __native_##name##_fn
#define NATIVE_FN_TYPE(identifier) runtime_val_t (*identifier)(vector_t args, scope_t *scope)

NATIVE_FN(println);
NATIVE_FN(print);
NATIVE_FN(pause);
NATIVE_FN(sleep);
NATIVE_FN(typeof);
NATIVE_FN(read);

void print_rtval(runtime_val_t *result, bool newline, int tabs, bool quote_strings);

#endif
