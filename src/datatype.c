/*
 * Created by rakinar2 on 8/30/23.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "datatype.h"
#include "alloca.h"
#include "log.h"
#include "parser.h"
#include "scope.h"
#include "utils.h"

static val_t **values = NULL;
static size_t values_count = 0;

val_t val_init()
{
    return (val_t) {
        .nofree = false
    };
}

val_t *val_init_heap()
{
    val_t *val = blaze_malloc(sizeof(val_t));
    val->nofree = false;

    log_debug("Created value: %p", val);
    return val;
}

val_t *val_copy(val_t *value)
{
    val_t *copy = blaze_calloc(1, sizeof(val_t));
    memcpy(copy, value, sizeof (val_t));
    values = blaze_realloc(values, sizeof(val_t *) * ++values_count);
    values[values_count - 1] = copy;
    return copy;
}

void val_free_global()
{

}

val_t *val_create_heap(val_type_t type)
{
    val_t val = val_create(type);
    val_t *val_ret = val_init_heap();
    memcpy(val_ret, &val, sizeof val);
    return val_ret;
}

val_t val_create(val_type_t type)
{
    val_t val = val_init();
    val.type = type;

    switch (val.type)
    {
        case VAL_INTEGER:
            val.intval = blaze_malloc(sizeof *(val.intval));
            break;

        case VAL_FLOAT:
            val.floatval = blaze_malloc(sizeof *(val.floatval));
            break;

        case VAL_STRING:
            val.strval = blaze_malloc(sizeof *(val.strval));
            break;

        case VAL_BOOLEAN:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            break;

        case VAL_FUNCTION:
            val.fnval = blaze_calloc(1, sizeof *(val.fnval));
            val.fnval->scope = NULL;
            break;

        case VAL_ARRAY:
            val.arrval = blaze_calloc(1, sizeof *(val.arrval));
            val.arrval->array = vector_init();
            break;

        case VAL_NULL:
            break;

        default:
            log_warn("unrecognized value type: %d", val.type);
    }

    return val;
}

val_t *val_copy_deep(val_t *orig)
{
    if (orig->type == VAL_NULL)
        return orig;

    val_t *val = val_create_heap(orig->type);

    switch (orig->type)
    {
        case VAL_INTEGER:
            val->intval->value = orig->intval->value;
            break;

        case VAL_FLOAT:
            val->floatval->value = orig->floatval->value;
            break;

        case VAL_STRING:
            val->strval->value = orig->strval->value;
            break;

        case VAL_BOOLEAN:
            val->boolval->value = orig->boolval->value;
            break;

        case VAL_FUNCTION:
            memcpy(val->fnval, orig->fnval, sizeof (*val->fnval));

            if (val->fnval->type == FN_USER_CUSTOM)
            {
                val->fnval->custom_body = NULL;
                val->fnval->size = 0;

                for (size_t i = 0; i < orig->fnval->size; i++)
                {
                    val->fnval->custom_body = blaze_realloc(
                        val->fnval->custom_body,
                        sizeof(ast_node_t *) * (++val->fnval->size));
                    val->fnval->custom_body[val->fnval->size - 1] = parser_ast_deep_copy(orig->fnval->custom_body[i]);
                }

                val->fnval->param_count = 0;
                val->fnval->param_names = NULL;

                for (size_t i = 0; i < orig->fnval->param_count; i++)
                {
                    val->fnval->param_names = blaze_realloc(
                        val->fnval->param_names,
                        sizeof(char *) * (++val->fnval->param_count));
                    val->fnval->param_names[val->fnval->param_count - 1] = blaze_strdup(orig->fnval->param_names[i]);
                }

                val->fnval->scope = scope_init(orig->fnval->scope);
            }

            break;

        case VAL_NULL:
            break;

        default:
            log_warn("unrecognized value type: %d", val->type);
    }

    return val;
}

void val_free_force_no_root(val_t *val)
{
    log_debug("Freeing value: %p", val);

    switch (val->type)
    {
    case VAL_INTEGER:
        blaze_free(val->intval);
        break;

    case VAL_FLOAT:
        blaze_free(val->floatval);
        break;

    case VAL_STRING:
        blaze_free(val->strval->value);
        blaze_free(val->strval);
        break;

    case VAL_BOOLEAN:
        blaze_free(val->boolval);
        break;

    case VAL_ARRAY:
        vector_free(val->arrval->array);
        blaze_free(val->arrval);
        break;

    case VAL_FUNCTION:
        if (val->fnval->type == FN_USER_CUSTOM)
        {
            for (size_t i = 0; i < val->fnval->size; i++)
                parser_ast_free(val->fnval->custom_body[i]);

            blaze_free(val->fnval->custom_body);

            for (size_t i = 0; i < val->fnval->param_count; i++)
                blaze_free(val->fnval->param_names[i]);

            blaze_free(val->fnval->param_names);
            scope_free(val->fnval->scope);
            val->fnval->scope = NULL;
        }

        blaze_free(val->fnval);
        break;

    case VAL_NULL:
        break;

    default:
        log_warn("unrecognized value type: %d", val->type);
    }

    blaze_free(val);
}

void val_free_force(val_t *val)
{
    val_free_force_no_root(val);
    blaze_free(val);
}

void val_free(val_t *val)
{
    if (val == NULL || val->nofree || (val->type == VAL_FUNCTION && val->fnval->type == FN_BUILT_IN))
        return;

    val_free_force(val);
}

void print_val_internal(val_t *val, bool quote_strings)
{
    if (val == NULL)
    {
        puts("[NULL]");
        return;
    }

    switch (val->type)
    {
    case VAL_INTEGER:
        printf("\033[1;33m%lld\033[0m", val->intval->value);
        break;

    case VAL_FLOAT:
        printf("\033[1;33m%Lf\033[0m", val->floatval->value);
        break;

    case VAL_STRING:
        printf("\033[32m%s%s%s\033[0m", quote_strings ? "\"" : "", val->strval->value, quote_strings ? "\"" : "");
        break;

    case VAL_BOOLEAN:
        printf("\033[36m%s\033[0m", val->boolval->value == true ? "true" : "false");
        break;

    case VAL_NULL:
        printf("\033[2mnull\033[0m");
        break;

    case VAL_ARRAY:
        printf("\033[34mArray\033[0m [");

        for (size_t i = 0; i < val->arrval->array->length; i++)
        {
            print_val_internal((val_t *) val->arrval->array->data[i], true);

            if (i != val->arrval->array->length - 1)
                printf(", ");
        }

        printf("]");
        break;

    case VAL_FUNCTION:
        printf("\033[2m[Function%s]\033[0m", val->fnval->type == FN_USER_CUSTOM ? "" : " Built-in");
        break;

    default:
        fatal_error("unrecognized value type: %d", val->type);
    }
}

void print_val(val_t *val)
{
    print_val_internal(val, true);
    printf("\n");
}