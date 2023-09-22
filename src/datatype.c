/*
 * Created by rakinar2 on 8/30/23.
 */

#include "datatype.h"
#include "alloca.h"
#include "log.h"
#include "parser.h"
#include "scope.h"
#include "utils.h"
#include "valalloc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct val_alloc_tbl val_alloc_tbl;

void val_alloc_tbl_global_init()
{
    val_alloc_tbl = val_alloc_tbl_init();
}

void val_alloc_tbl_global_free()
{
    val_alloc_tbl_free(&val_alloc_tbl, true);
}

val_t val_init()
{
    return (val_t) {
        .nofree = false,
        .self_ptr = NULL
    };
}

val_t *val_init_heap()
{
    val_t *val = val_alloc(&val_alloc_tbl);
    val->nofree = false;

    log_debug("Created value: %p", val);
    return val;
}

val_t *val_copy(val_t *value)
{
    val_t *copy = val_alloc(&val_alloc_tbl);
    memcpy(copy, value, sizeof (val_t));
    copy->self_ptr = copy;
    return copy;
}

void val_free_global()
{
    // FIXME
}

val_t *val_create_heap(val_type_t type)
{
    val_t val = val_create(type);
    val_t *val_ret = val_init_heap();
    memcpy(val_ret, &val, sizeof val);
    val_ret->self_ptr = val_ret;
    return val_ret;
}

val_t val_create(val_type_t type)
{
    val_t val = val_init();
    val.type = type;

    switch (val.type)
    {
        case VAL_FUNCTION:
            val.fnval = xcalloc(1, sizeof *(val.fnval));
            val.fnval->scope = NULL;
            break;

        case VAL_ARRAY:
            val.arrval = vector_init();
            break;

        case VAL_NULL:
        case VAL_INTEGER:
        case VAL_FLOAT:
        case VAL_STRING:
        case VAL_BOOLEAN:
            break;

        default:
            log_warn("unrecognized value type: %d", val.type);
    }

    return val;
}

val_t *val_copy_deep(val_t *orig)
{
    if (orig->type == VAL_NULL)
        return blaze_null();

    val_t *val = val_create_heap(orig->type);

    switch (orig->type)
    {
        case VAL_INTEGER:
            val->intval = orig->intval;
            break;

        case VAL_FLOAT:
            val->floatval = orig->floatval;
            break;

        case VAL_STRING:
            val->strval = strdup(orig->strval);
            break;

        case VAL_BOOLEAN:
            val->boolval = orig->boolval;
            break;

        case VAL_FUNCTION:
            memcpy(val->fnval, orig->fnval, sizeof (*val->fnval));

            if (val->fnval->type == FN_USER_CUSTOM)
            {
                val->fnval->custom_body = NULL;
                val->fnval->size = 0;

                for (size_t i = 0; i < orig->fnval->size; i++)
                {
                    val->fnval->custom_body = xrealloc(
                        val->fnval->custom_body,
                        sizeof(ast_node_t *) * (++val->fnval->size));
                    val->fnval->custom_body[val->fnval->size - 1] = parser_ast_deep_copy(orig->fnval->custom_body[i]);
                }

                val->fnval->param_count = 0;
                val->fnval->param_names = NULL;

                for (size_t i = 0; i < orig->fnval->param_count; i++)
                {
                    val->fnval->param_names = xrealloc(
                        val->fnval->param_names,
                        sizeof(char *) * (++val->fnval->param_count));
                    val->fnval->param_names[val->fnval->param_count - 1] = strdup(orig->fnval->param_names[i]);
                }

                val->fnval->scope = scope_init(orig->fnval->scope);
            }

            break;

        case VAL_NULL:
            val->type = VAL_NULL;
            break;

        default:
            log_warn("unrecognized value type: %d", val->type);
    }

    return val;
}

void val_free_force_no_root(val_t *val)
{
    if (val->nofree)
        return;

    log_debug("Freeing value: %p", val);

    if (val->type == VAL_FUNCTION)
        log_debug("Attempt to free function: %p", val->fnval);

    switch (val->type)
    {
        case VAL_STRING:
            log_debug("Freeing string: %p", val);
            free(val->strval);
            val->strval = NULL;
            break;

        case VAL_ARRAY:
            vector_free(val->arrval);
            val->arrval = NULL;
            break;

        case VAL_FUNCTION:
            if (val->fnval->type == FN_USER_CUSTOM)
            {
                for (size_t i = 0; i < val->fnval->size; i++)
                    parser_ast_free(val->fnval->custom_body[i]);

                free(val->fnval->custom_body);

                for (size_t i = 0; i < val->fnval->param_count; i++)
                    free(val->fnval->param_names[i]);

                free(val->fnval->param_names);
                scope_free(val->fnval->scope);
                val->fnval->scope = NULL;
                free(val->fnval);
            }

            break;

        case VAL_INTEGER:
        case VAL_FLOAT:
        case VAL_BOOLEAN:
        case VAL_NULL:
            break;

        default:
            log_warn("unrecognized value type: %d", val->type);
    }
}

void val_free_force(val_t *val)
{
    val_free_force_no_root(val);
    val_alloc_free(&val_alloc_tbl, val, true);
}

void val_free(val_t *val)
{
    if (val == NULL || val->nofree || (val->type == VAL_FUNCTION && val->fnval->type == FN_BUILT_IN))
        return;

    val_free_force(val);
}
