/*
 * Created by rakinar2 on 8/24/23.
 */

#include "eval.h"
#include "alloca.h"
#include "ast.h"
#include "datatype.h"
#include "file.h"
#include "lib.h"
#include "log.h"
#include "parser.h"
#include "scope.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

val_t *eval_int(scope_t *scope, const ast_node_t *node);
val_t *eval_float(scope_t *scope, const ast_node_t *node);
val_t *eval_string(scope_t *scope, const ast_node_t *node);
val_t *eval_root(scope_t *scope, const ast_node_t *node);
val_t *eval_binexp(scope_t *scope, const ast_node_t *node);
val_t *eval_var_decl(scope_t *scope, const ast_node_t *node);
val_t *eval_identifier(scope_t *scope, const ast_node_t *node);
val_t *eval_assignment(scope_t *scope, const ast_node_t *node);
val_t *eval_expr_call(scope_t *scope, const ast_node_t *node);
val_t *eval_fn_decl(scope_t *scope, const ast_node_t *node);

char *eval_fn_error = NULL;

static val_t **values = NULL;
static size_t values_count = 0;

static inline val_t *val_init()
{
    val_t *val = xmalloc(sizeof (val_t));
    val->nofree = false;

    values = xrealloc(values, sizeof (val_t *) * ++values_count);
    values[values_count - 1] = val;
    val->index = values_count - 1;

    log_debug("Created value: %p", val);
    return val;
}

void val_free_global()
{
    for (size_t i = 0; i < values_count; i++)
    {
        val_free(values[i]);
    }

    free(values);
}

val_t *val_create(val_type_t type)
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

        case VAL_FUNCTION:
            val->fnval = xmalloc(sizeof *(val->fnval));
            break;

        case VAL_NULL:
            break;

        default:
            log_warn("unrecognized value type: %d", val->type);
    }

    return val;
}

val_t *val_copy_deep(val_t *orig)
{
    if (orig->type == VAL_NULL)
        return orig;

    val_t *val = val_create(orig->type);

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
            // FIXME: Shallow copy
            memcpy(val->fnval, orig->fnval, sizeof (*val->fnval));
            break;

        case VAL_NULL:
            break;

        default:
            log_warn("unrecognized value type: %d", val->type);
    }

    return val;
}

void val_free_force(val_t *val)
{
    log_debug("Freeing value: %p", val);
    values[val->index] = NULL;

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

        case VAL_FUNCTION:
            free(val->fnval);
            break;

        case VAL_NULL:
            break;

        default:
            log_warn("unrecognized value type: %d", val->type);
    }

    free(val);
}

void val_free(val_t *val)
{
    if (val == NULL || val->nofree || (val->type == VAL_FUNCTION && val->fnval->type == FN_BUILT_IN))
        return;

    val_free_force(val);
}

val_t *eval(scope_t *scope, const ast_node_t *node)
{
    switch (node->type)
    {
        case NODE_INT_LIT:
            return eval_int(scope, node);

        case NODE_ROOT:
            return eval_root(scope, node);

        case NODE_STRING:
            return eval_string(scope, node);

        case NODE_BINARY_EXPR:
            return eval_binexp(scope, node);

        case NODE_VAR_DECL:
            return eval_var_decl(scope, node);

        case NODE_IDENTIFIER:
            return eval_identifier(scope, node);

        case NODE_ASSIGNMENT:
            return eval_assignment(scope, node);

        case NODE_EXPR_CALL:
            return eval_expr_call(scope, node);

        case NODE_FN_DECL:
            return eval_fn_decl(scope, node);

        default:
            fatal_error("cannot evaluate AST: unsupported AST node");
            return NULL;
    }
}

static val_t *val_copy(val_t *value)
{
    val_t *copy = xcalloc(1, sizeof (val_t));
    memcpy(copy, value, sizeof (val_t));
    values = xrealloc(values, sizeof (val_t *) * ++values_count);
    values[values_count - 1] = copy;
    copy->index = values_count - 1;
    return copy;
}

#define VAL_CHECK_EXIT(val) \
    if (val == NULL) \
        return NULL;

val_t *eval_fn_decl(scope_t *scope, const ast_node_t *node)
{
    val_t *fn = val_create(VAL_FUNCTION);

    fn->fnval->type = FN_USER_CUSTOM;
    fn->fnval->custom_body = node->fn_decl->body;
    fn->fnval->param_count = node->fn_decl->param_count;
    fn->fnval->param_names = node->fn_decl->param_names;
    fn->fnval->size = node->fn_decl->size;

    enum valmap_set_status status = scope_declare_identifier(scope, node->fn_decl->identifier->symbol, fn, true);

    if (status == VAL_SET_EXISTS)
    {
        RUNTIME_ERROR(filebuf_current_file,
                      node->line_start,
                      node->column_start,
                      "'%s' is already defined",
                      node->fn_decl->identifier->symbol);

        return NULL;
    }

    return scope->null;
}

val_t *eval_expr_call(scope_t *scope, const ast_node_t *node)
{
    char *identifier = node->fn_call->identifier->symbol;

    val_t *val = scope_resolve_identifier(scope, identifier);

    if (val == NULL)
    {
        RUNTIME_ERROR(filebuf_current_file,
                      node->line_start,
                      node->column_start,
                      "undefined function '%s'",
                      identifier);
        return NULL;
    }

    if (val->type != VAL_FUNCTION)
    {
        RUNTIME_ERROR(filebuf_current_file,
                      node->line_start,
                      node->column_start,
                      "'%s' is not a function",
                      identifier);
        return NULL;
    }

    val_t **args = NULL;

    for (size_t i = 0; i < node->fn_call->argc; i++)
    {
        val_t *arg = eval(scope, node->fn_call->args[i]);
        VAL_CHECK_EXIT(arg);
        args = xrealloc(args, sizeof (val_t *) * (i + 1));
        args[i] = arg;
    }

    if (val->fnval->type == FN_BUILT_IN)
    {
        val_t *ret = val->fnval->built_in_callback(scope, node->fn_call->argc, args);
        free(args);
        VAL_CHECK_EXIT(ret);

        if (eval_fn_error != NULL)
        {
            RUNTIME_ERROR(filebuf_current_file,
                          node->line_start,
                          node->column_start,
                          "%s",
                          eval_fn_error);

            free(eval_fn_error);
            eval_fn_error = NULL;
            return NULL;
        }

        return ret;
    }

    if (val->fnval->param_count != node->fn_call->argc)
    {
        RUNTIME_ERROR(filebuf_current_file,
                      node->line_start,
                      node->column_start,
                      "function '%s' requires %lu arguments, but %lu were passed",
                      identifier, val->fnval->param_count, node->fn_call->argc);
        return NULL;
    }

    scope_t *new_scope = scope_init(scope);

    for (size_t i = 0; i < node->fn_call->argc; i++)
    {
        val_t *arg = args[i];
        enum valmap_set_status status = scope_declare_identifier(new_scope, val->fnval->param_names[i], arg, true);

        if (status == VAL_SET_EXISTS)
        {
            RUNTIME_ERROR(filebuf_current_file,
                          node->line_start,
                          node->column_start,
                          "cannot redefine '%s' as a function parameter",
                          node->fn_decl->identifier->symbol);

            return NULL;
        }
    }

    val_t *ret;

    for (size_t i = 0; i < val->fnval->size; i++)
    {
        ret = eval(new_scope, val->fnval->custom_body[i]);
        VAL_CHECK_EXIT(ret);
    }

    val_t *copy = val_copy_deep(ret);
    free(args);
    scope_free(new_scope);
    return copy;
}

val_t *eval_assignment(scope_t *scope, const ast_node_t *node)
{
    val_t *val = eval(scope, node->assignment_expr->value);

    VAL_CHECK_EXIT(val);

    enum valmap_set_status status = scope_assign_identifier(scope, node->assignment_expr->assignee->identifier->symbol, val);

    if (status == VAL_SET_NOT_FOUND)
    {
        RUNTIME_ERROR(filebuf_current_file,
                      node->assignment_expr->assignee->line_start,
                      node->assignment_expr->assignee->column_start,
                      "use of undeclared identifier '%s'",
                      node->assignment_expr->assignee->identifier->symbol);
        return NULL;
    }
    else if (status == VAL_SET_IS_CONST)
    {
        RUNTIME_ERROR(filebuf_current_file,
                      node->assignment_expr->assignee->line_start,
                      node->assignment_expr->assignee->column_start,
                      "cannot assign to constant '%s'",
                      node->assignment_expr->assignee->identifier->symbol);
        return NULL;
    }

    return val;
}

val_t *eval_identifier(scope_t *scope, const ast_node_t *node)
{
    val_t *val = scope_resolve_identifier(scope, node->identifier->symbol);

    if (val == NULL)
    {
        RUNTIME_ERROR(filebuf_current_file, node->line_start,
                      node->column_start, "use of undeclared identifier '%s'",
                      node->identifier->symbol);
        return NULL;
    }

    return val;
}

val_t *eval_int(scope_t *scope, const ast_node_t *node)
{
    val_t *val = val_create(VAL_INTEGER);
    val->intval->value = node->integer->intval;
    return val;
}

val_t *eval_string(scope_t *scope, const ast_node_t *node)
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
        {
            fatal_error("unsupported operator '%c' (%d)", operator, operator);
            return NULL;
        }
    }

    return val;
}

val_t *eval_var_decl(scope_t *scope, const ast_node_t *node)
{
    val_t *val = node->var_decl->value == NULL ? scope->null : eval(scope, node->var_decl->value);

    VAL_CHECK_EXIT(val);

    enum valmap_set_status status = scope_declare_identifier(scope, node->var_decl->name, val, node->var_decl->is_const);

    if (status == VAL_SET_EXISTS)
    {
        RUNTIME_ERROR(
            filebuf_current_file, node->line_start, node->column_start,
            "cannot redeclare identifier '%s'", node->identifier->symbol);

        return NULL;
    }

    return scope->null;
}

val_t *eval_binexp(scope_t *scope, const ast_node_t *node)
{
    val_t *left = eval(scope, node->binexpr->left);
    VAL_CHECK_EXIT(left);
    val_t *right = eval(scope, node->binexpr->right);
    VAL_CHECK_EXIT(right);
    val_t *ret = NULL;

    if (left->type == VAL_INTEGER && right->type == VAL_INTEGER)
    {
        ret = eval_binexp_int(node->binexpr->operator, left, right);
    }
    else
        fatal_error("unsupported binary operation (lhs: %s(%d), rhs: %s(%d))",
                    val_type_to_str(left->type), left->type,
                    val_type_to_str(right->type), right->type);

    return ret;
}

val_t *eval_root(scope_t *scope, const ast_node_t *node)
{
    val_t *value = NULL;

    for (size_t i = 0; i < node->root->size; i++)
    {
        value = eval(scope, node->root->nodes[i]);
        VAL_CHECK_EXIT(value);
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