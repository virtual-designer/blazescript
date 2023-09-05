/*
 * Created by rakinar2 on 8/24/23.
 */

#include "eval.h"
#include "alloca.h"
#include "ast.h"
#include "datatype.h"
#include "log.h"
#include "parser.h"
#include "scope.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

val_t eval_int(scope_t *scope, const ast_node_t *node);
val_t eval_float(scope_t *scope, const ast_node_t *node);
val_t eval_string(scope_t *scope, const ast_node_t *node);
val_t eval_root(scope_t *scope, const ast_node_t *node);
val_t eval_binexp(scope_t *scope, const ast_node_t *node);
val_t eval_var_decl(scope_t *scope, const ast_node_t *node);
val_t eval_identifier(scope_t *scope, const ast_node_t *node);
val_t eval_assignment(scope_t *scope, const ast_node_t *node);
val_t eval_expr_call(scope_t *scope, const ast_node_t *node);
val_t eval_fn_decl(scope_t *scope, const ast_node_t *node);
val_t eval_array_lit(scope_t *scope, const ast_node_t *node);

char *eval_fn_error = NULL;

val_t eval(scope_t *scope, const ast_node_t *node)
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

        case NODE_ARRAY_LIT:
            return eval_array_lit(scope, node);

        default:
            fatal_error("cannot evaluate AST: unsupported AST node");
    }
}

val_t eval_array_lit(scope_t *scope, const ast_node_t *node)
{
    val_t arr = val_create(VAL_ARRAY);

    VECTOR_FOREACH(node->array_lit->elements)
    {
        val_t val = eval(scope, ((ast_node_t **) node->array_lit->elements->data)[i]);
        vector_push(arr.arrval->array, val_copy_deep(&val));
    }

    return arr;
}

val_t eval_fn_decl(scope_t *scope, const ast_node_t *node)
{
    val_t fn = val_create(VAL_FUNCTION);

    fn.fnval->type = FN_USER_CUSTOM;
    fn.fnval->custom_body = NULL;
    fn.fnval->size = 0;

    for (size_t i = 0; i < node->fn_decl->size; i++)
    {
        fn.fnval->custom_body = blaze_realloc(
            fn.fnval->custom_body, sizeof(ast_node_t *) * (++fn.fnval->size));
        fn.fnval->custom_body[fn.fnval->size - 1] = parser_ast_deep_copy(&node->fn_decl->body[i]);
    }

    fn.fnval->param_count = 0;
    fn.fnval->param_names = NULL;

    for (size_t i = 0; i < node->fn_decl->param_count; i++)
    {
        fn.fnval->param_names =
            blaze_realloc(fn.fnval->param_names,
                          sizeof(char *) * (++fn.fnval->param_count));
        fn.fnval->param_names[fn.fnval->param_count - 1] = blaze_strdup(node->fn_decl->param_names[i]);
    }

    fn.fnval->scope = scope_init(scope);

    enum valmap_set_status status = scope_declare_identifier(scope, node->fn_decl->identifier->symbol, fn, true);

    if (status == VAL_SET_EXISTS)
    {
        RUNTIME_ERROR(node->filename,
                      node->line_start,
                      node->column_start,
                      "'%s' is already defined",
                      node->fn_decl->identifier->symbol);
    }

    return *scope->null;
}

val_t eval_expr_call(scope_t *scope, const ast_node_t *node)
{
    char *identifier = node->fn_call->identifier->symbol;

    val_t *val = scope_resolve_identifier(scope, identifier);

    if (val == NULL)
    {
        RUNTIME_ERROR(node->filename,
                      node->line_start,
                      node->column_start,
                      "undefined function '%s'",
                      identifier);
    }

    if (val->type != VAL_FUNCTION)
    {
        RUNTIME_ERROR(node->filename,
                      node->line_start,
                      node->column_start,
                      "'%s' is not a function",
                      identifier);
    }

    val_t *args = NULL;

    for (size_t i = 0; i < node->fn_call->argc; i++)
    {
        val_t arg = eval(scope, &node->fn_call->args[i]);
        args = blaze_realloc(args, sizeof(val_t) * (i + 1));
        args[i] = arg;
    }

    if (val->fnval->type == FN_BUILT_IN)
    {
        val_t ret = val->fnval->built_in_callback(scope, node->fn_call->argc, args);
        blaze_free(args);

        if (eval_fn_error != NULL)
        {
            RUNTIME_ERROR(node->filename,
                          node->line_start,
                          node->column_start,
                          "%s",
                          eval_fn_error);

            blaze_free(eval_fn_error);
            eval_fn_error = NULL;
            exit(-1);
        }

        return ret;
    }

    if (val->fnval->param_count != node->fn_call->argc)
    {
        RUNTIME_ERROR(node->filename,
                      node->line_start,
                      node->column_start,
                      "function '%s' requires %lu arguments, but %lu were passed",
                      identifier, val->fnval->param_count, node->fn_call->argc);
        exit(-1);
    }

    for (size_t i = 0; i < node->fn_call->argc; i++)
    {
        val_t arg = args[i];
        enum valmap_set_status status = scope_declare_identifier(val->fnval->scope, val->fnval->param_names[i], arg, true);

        if (status == VAL_SET_EXISTS)
        {
            RUNTIME_ERROR(node->filename,
                          node->line_start,
                          node->column_start,
                          "cannot redefine '%s' as a function parameter",
                          node->fn_decl->identifier->symbol);

            exit(-1);
        }
    }

    val_t ret;

    for (size_t i = 0; i < val->fnval->size; i++)
    {
        ret = eval(val->fnval->scope, val->fnval->custom_body[i]);
    }

    val_t *copy = val_copy_deep(&ret);
    blaze_free(args);
    val->fnval->scope = scope_init(scope);
    return *copy;
}

val_t eval_assignment(scope_t *scope, const ast_node_t *node)
{
    val_t val = eval(scope, node->assignment_expr->value);

    enum valmap_set_status status = scope_assign_identifier(scope, node->assignment_expr->assignee->identifier->symbol, val);

    if (status == VAL_SET_NOT_FOUND)
    {
        RUNTIME_ERROR(node->filename,
                      node->assignment_expr->assignee->line_start,
                      node->assignment_expr->assignee->column_start,
                      "use of undeclared identifier '%s'",
                      node->assignment_expr->assignee->identifier->symbol);
        exit(-1);
    }
    else if (status == VAL_SET_IS_CONST)
    {
        RUNTIME_ERROR(node->filename,
                      node->assignment_expr->assignee->line_start,
                      node->assignment_expr->assignee->column_start,
                      "cannot assign to constant '%s'",
                      node->assignment_expr->assignee->identifier->symbol);
        exit(-1);
    }

    return val;
}

val_t eval_identifier(scope_t *scope, const ast_node_t *node)
{
    val_t *val = scope_resolve_identifier(scope, node->identifier->symbol);

    if (val == NULL)
    {
        RUNTIME_ERROR(node->filename, node->line_start,
                      node->column_start, "use of undeclared identifier '%s'",
                      node->identifier->symbol);
        exit(-1);
    }

    return *val;
}

val_t eval_int(scope_t *scope, const ast_node_t *node)
{
    val_t val = val_create(VAL_INTEGER);
    val.intval->value = node->integer->intval;
    return val;
}

val_t eval_string(scope_t *scope, const ast_node_t *node)
{
    val_t val = val_create(VAL_STRING);
    val.strval->value = blaze_strdup(node->string->strval);
    return val;
}

static val_t eval_binexp_int(ast_bin_operator_t operator, val_t *left, val_t *right, const ast_node_t *node)
{
    val_t val = val_init();
    val.type = VAL_INTEGER;

    switch (operator)
    {
        case OP_PLUS:
            val.intval = blaze_malloc(sizeof *(val.intval));
            val.intval->value = left->intval->value + right->intval->value;
            break;

        case OP_MINUS:
            val.intval = blaze_malloc(sizeof *(val.intval));
            val.intval->value = left->intval->value - right->intval->value;
            break;

        case OP_TIMES:
            val.intval = blaze_malloc(sizeof *(val.intval));
            val.intval->value = left->intval->value * right->intval->value;
            break;

        case OP_DIVIDE:
            if (right->intval->value == 0)
                RUNTIME_ERROR(node->binexpr->right->filename,
                          node->binexpr->right->line_start,
                          node->binexpr->right->column_start,
                          "cannot divide %lli by zero", left->intval->value);

            val.type = VAL_FLOAT;
            val.floatval = blaze_malloc(sizeof *(val.intval));
            val.floatval->value = (long double) left->intval->value / (long double) right->intval->value;
            break;

        case OP_MODULUS:
            if (right->intval->value == 0)
                RUNTIME_ERROR(node->binexpr->right->filename,
                              node->binexpr->right->line_start,
                              node->binexpr->right->column_start,
                              "cannot divide %lli by zero", left->intval->value);

            val.intval = blaze_malloc(sizeof *(val.intval));
            val.intval->value = left->intval->value % right->intval->value;
            break;

        default:
        {
            fatal_error("unsupported operator '%c' (%d)", operator, operator);
            exit(-1);
        }
    }

    return val;
}

val_t eval_var_decl(scope_t *scope, const ast_node_t *node)
{
    val_t val = node->var_decl->value == NULL ? *scope->null : eval(scope, node->var_decl->value);

    enum valmap_set_status status = scope_declare_identifier(scope, node->var_decl->name, val, node->var_decl->is_const);

    if (status == VAL_SET_EXISTS)
    {
        RUNTIME_ERROR(
            node->filename, node->line_start, node->column_start,
            "cannot redeclare identifier '%s'", node->identifier->symbol);

        exit(-1);
    }

    return *scope->null;
}

val_t eval_binexp(scope_t *scope, const ast_node_t *node)
{
    val_t left = eval(scope, node->binexpr->left);
    val_t right = eval(scope, node->binexpr->right);
    val_t ret = {
        .type = VAL_NULL
    };

    if (left.type == VAL_INTEGER && right.type == VAL_INTEGER)
    {
        ret = eval_binexp_int(node->binexpr->operator, &left, &right, node);
    }
    else
        RUNTIME_ERROR(node->filename, node->line_start, node->column_start,
                        "unsupported binary operation (lhs: %s(%d), rhs: %s(%d))",
                        val_type_to_str(left.type), left.type,
                        val_type_to_str(right.type), right.type);

    return ret;
}

val_t eval_root(scope_t *scope, const ast_node_t *node)
{
    val_t value = *scope->null;

    for (size_t i = 0; i < node->root->size; i++)
    {
        value = eval(scope, &node->root->nodes[i]);
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
        [VAL_OBJECT] = "OBJECT",
        [VAL_ARRAY] = "ARRAY"
    };

    size_t length = sizeof (translate) / sizeof (const char *);

    assert(type < length && "Invalid value type");
    return translate[type];
}
