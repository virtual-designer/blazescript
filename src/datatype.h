/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_DATATYPE_H
#define BLAZESCRIPT_DATATYPE_H

#include "ast.h"
#include "vector.h"
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
    bool value;
} val_boolean_t;

typedef struct {
    enum {
        FN_BUILT_IN,
        FN_USER_CUSTOM
    } type;
    union {
        struct value (*built_in_callback)(struct scope *scope, size_t argc, struct value *args);
        struct {
            ast_node_t **custom_body;
            size_t size;
            size_t param_count;
            char **param_names;
            struct scope *scope;
        };
    };
} val_function_t;

typedef struct value {
    val_type_t type;
    bool nofree;
    struct value *self_ptr;

    union {
        long long int intval;
        long double floatval;
        char *strval;
        bool boolval;
        val_function_t *fnval;
        vector_t *arrval;
    };
} val_t;

void val_free(val_t *val);
void print_val(val_t *val);
void print_val_internal(val_t *val, bool quote_strings);
const char *val_type_to_str(val_type_t type);
val_t val_create(val_type_t type);
val_t *val_create_heap(val_type_t type);
void val_free_global();
void val_free_force(val_t *val);
val_t *val_copy_deep(val_t *orig);
val_t val_init();
val_t *val_init_heap();
val_t *val_copy(val_t *value);
void val_alloc_tbl_global_init();
void val_alloc_tbl_global_free();
void val_free_force_no_root(val_t *val);

extern struct val_alloc_tbl val_alloc_tbl;

#endif /* BLAZESCRIPT_DATATYPE_H */
