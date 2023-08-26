/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_DATATYPE_H
#define BLAZESCRIPT_DATATYPE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    VAL_INTEGER,
    VAL_FLOAT,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_OBJECT,
    VAL_NULL,
    VAL_BOOLEAN,
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
        struct value *(*built_in_callback)(struct value **args, size_t argc);
        ast_node_t *custom_body;
    };
} val_function_t;

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
    };
} val_t;

#endif /* BLAZESCRIPT_DATATYPE_H */
