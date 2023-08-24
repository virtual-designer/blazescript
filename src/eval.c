/*
 * Created by rakinar2 on 8/24/23.
 */

#include "eval.h"
#include "alloca.h"
#include "ast.h"
#include "datatype.h"
#include "log.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

val_t *eval_int(const ast_node_t *node);
val_t *eval_float(const ast_node_t *node);
val_t *eval_string(const ast_node_t *node);
val_t *eval_root(const ast_node_t *node);
val_t *eval_binexp(const ast_node_t *node);

static inline val_t *val_init()
{
    return xmalloc(sizeof (val_t));
}

static inline val_t *val_create(val_type_t type)
{
    val_t *val = val_init();
    val->type = type;

    switch (val->type)
    {
        case VAL_INTEGER:
            val->intval = xmalloc(sizeof *(val->intval));
            break;

        case VAL_FLOAT:
            val->floatval = xmalloc(sizeof *(val->floatval));
            break;

        case VAL_STRING:
            val->strval = xmalloc(sizeof *(val->strval));
            break;

        case VAL_BOOLEAN:
            val->boolval = xmalloc(sizeof *(val->boolval));
            break;

        case VAL_NULL:
            break;

        default:
            log_warn("unrecognized value type: %d", val->type);
    }

    return val;
}

void val_free(val_t *val)
{
    switch (val->type)
    {
        case VAL_INTEGER:
            free(val->intval);
            break;

        case VAL_FLOAT:
            free(val->floatval);
            break;

        case VAL_STRING:
            free(val->strval->value);
            free(val->strval);
            break;

        case VAL_BOOLEAN:
            free(val->boolval);
            break;

        case VAL_NULL:
            break;

        default:
            log_warn("unrecognized value type: %d", val->type);
    }

    free(val);
}

val_t *eval(const ast_node_t *node)
{
    switch (node->type)
    {
        case NODE_INT_LIT:
            return eval_int(node);

        case NODE_ROOT:
            return eval_root(node);

        case NODE_STRING:
            return eval_string(node);

        case NODE_BINARY_EXPR:
            return eval_binexp(node);

        default:
            fatal_error("cannot evaluate AST: unsupported AST node");
    }
}

val_t *eval_int(const ast_node_t *node)
{
    val_t *val = val_create(VAL_INTEGER);
    val->intval->value = node->integer->intval;
    return val;
}

val_t *eval_string(const ast_node_t *node)
{
    val_t *val = val_create(VAL_STRING);
    val->strval->value = strdup(node->string->strval);
    return val;
}

static val_t *eval_binexp_int(ast_bin_operator_t operator, val_t *left, val_t *right)
{
    val_t *val = val_init();
    val->type = VAL_INTEGER;

    switch (operator)
    {
        case OP_PLUS:
            val->intval = xmalloc(sizeof *(val->intval));
            val->intval->value = left->intval->value + right->intval->value;
            break;

        case OP_MINUS:
            val->intval = xmalloc(sizeof *(val->intval));
            val->intval->value = left->intval->value - right->intval->value;
            break;

        case OP_TIMES:
            val->intval = xmalloc(sizeof *(val->intval));
            val->intval->value = left->intval->value * right->intval->value;
            break;

        case OP_DIVIDE:
            if (right->intval->value == 0)
                fatal_error("cannot divide by zero");

            val->type = VAL_FLOAT;
            val->floatval = xmalloc(sizeof *(val->intval));
            val->floatval->value = (long double) left->intval->value / (long double) right->intval->value;
            break;

        case OP_MODULUS:
            if (right->intval->value == 0)
                fatal_error("cannot divide by zero");

            val->intval = xmalloc(sizeof *(val->intval));
            val->intval->value = left->intval->value % right->intval->value;
            break;

        default:
            fatal_error("unsupported operator '%c' (%d)", operator, operator);
    }

    return val;
}

val_t *eval_binexp(const ast_node_t *node)
{
    val_t *left = eval(node->binexpr->left);
    val_t *right = eval(node->binexpr->right);
    val_t *ret;

    if (left->type == VAL_INTEGER && right->type == VAL_INTEGER)
        ret = eval_binexp_int(node->binexpr->operator, left, right);
    else
        fatal_error("unsupported binary operation (lhs: %s(%d), rhs: %s(%d))",
                    val_type_to_str(left->type), left->type,
                    val_type_to_str(right->type), right->type);

    val_free(left);
    val_free(right);

    return ret;
}

val_t *eval_root(const ast_node_t *node)
{
    val_t *value = val_create(VAL_NULL);

    for (size_t i = 0; i < node->root->size; i++)
    {
        val_free(value);
        value = eval(&node->root->nodes[i]);
    }

    return value;
}

const char *val_type_to_str(val_type_t type)
{
    const char *translate[] = {
        [VAL_INTEGER] = "INTEGER",
        [VAL_BOOLEAN] = "BOOLEAN",
        [VAL_STRING] = "STRING",
        [VAL_FLOAT] = "FLOAT",
        [VAL_FUNCTION] = "FUNCTION",
        [VAL_NULL] = "NULL",
        [VAL_OBJECT] = "OBJECT"
    };

    size_t length = sizeof (translate) / sizeof (const char *);

    assert(type < length && "Invalid value type");
    return translate[type];
}

void print_val(val_t *val)
{
    printf("Value { type: %s(%d)", val_type_to_str(val->type), val->type);

    switch (val->type)
    {
        case VAL_INTEGER:
            printf(", value: \033[1;33m%lld\033[0m", val->intval->value);
            break;

        case VAL_FLOAT:
            printf(", value: \033[1;33m%Lf\033[0m", val->floatval->value);
            break;

        case VAL_STRING:
            printf(", value: \033[32m\"%s\"\033[0m", val->strval->value);
            break;

        case VAL_BOOLEAN:
            printf(", value: \033[36m%s\033[0m", val->boolval->value == true ? "true" : "false");
            break;

        case VAL_NULL:
            printf(", value: \033[2mnull\033[0m");
            break;

        default:
            fatal_error("unrecognized value type: %d", val->type);
    }

    printf(" }\n");
}