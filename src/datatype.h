/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_DATATYPE_H
#define BLAZESCRIPT_DATATYPE_H

#include "array.h"
#include "ast.h"
#include <stdbool.h>
#include <stddef.h>

struct scope;

typedef enum {
    VAL_INTEGER,
    VAL_FLOAT,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_OBJECT,
    VAL_NULL,
    VAL_BOOLEAN,
    VAL_ARRAY
} val_type_t;

typedef struct {
    long long int value;
} val_integer_t;

typedef struct {
    long double value;
} val_float_t;

typedef struct {
    char *value;
} val_string_t;

typedef struct {
    bool value;
} val_boolean_t;

typedef struct {
    enum {
        FN_BUILT_IN,
        FN_USER_CUSTOM
    } type;
    union {
        struct value *(*built_in_callback)(struct scope *scope, size_t argc, struct value **args);
        struct {
            ast_node_t **custom_body;
            size_t size;
            size_t param_count;
            char **param_names;
            struct scope *scope;
        };
    };
} val_function_t;

typedef struct {
    array_t *array;
} val_array_t;

typedef struct value {
    val_type_t type;
    size_t index;
    bool nofree;

    union {
        val_integer_t *intval;
        val_float_t *floatval;
        val_string_t *strval;
        val_boolean_t *boolval;
        val_function_t *fnval;
        val_array_t *arrval;
    };
} val_t;

#endif /* BLAZESCRIPT_DATATYPE_H */
