/*
 * Created by rakinar2 on 8/24/23.
 */

#define _GNU_SOURCE

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
#include <string.h>

#define BLAZE_NULL ((val_t) { .type = VAL_NULL })
#define BLAZE_INT(__value) ((val_t) { .type = VAL_INTEGER, .intval = & ((val_integer_t) { .value = __value }) })
#define BLAZE_BOOL(__value) ((val_t) { .type = VAL_BOOLEAN, .boolval = & ((val_boolean_t) { .value = __value }) })
#define BLAZE_TRUE BLAZE_BOOL(true)

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
val_t eval_block(scope_t *scope, const ast_node_t *node);
val_t eval_if_stmt(scope_t *scope, const ast_node_t *node);
val_t eval_loop_stmt(scope_t *scope, const ast_node_t *node);

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

        case NODE_BLOCK:
            return eval_block(scope, node);

        case NODE_IF_STMT:
            return eval_if_stmt(scope, node);

        case NODE_LOOP_STMT:
            return eval_loop_stmt(scope, node);

        default:
            fatal_error("cannot evaluate AST: unsupported AST node");
            return *scope->null;
    }
}

bool val_is_truthy(const val_t *val)
{
    return val->type != VAL_NULL &&
           (val->type != VAL_BOOLEAN || val->boolval->value != false) &&
           (val->type != VAL_INTEGER || val->intval->value != 0);
}

val_t eval_loop_stmt(scope_t *scope, const ast_node_t *node)
{
    val_t iter_count_val = node->loop_stmt->iter_count == NULL ? BLAZE_TRUE : eval(scope, node->loop_stmt->iter_count);

    if (iter_count_val.type != VAL_BOOLEAN && iter_count_val.type != VAL_INTEGER)
    {
        RUNTIME_ERROR(node->filename,
                      node->line_start,
                      node->column_start,
                      "type '%s' is not iterable",
                      val_type_to_str(iter_count_val.type));
    }

    if (iter_count_val.type == VAL_INTEGER && iter_count_val.intval->value < 0)
    {
        RUNTIME_ERROR(node->filename,
                      node->line_start,
                      node->column_start,
                      "the iteration count must not be a negative number%s",
                      "");
    }

    val_integer_t *counter_val = blaze_calloc(1, sizeof (val_integer_t));
    long long int counter = 0;
    counter_val->value = counter;
    char *varname = node->loop_stmt->iter_varname;
    scope_t *new_scope = scope_init(scope);

    if (varname != NULL)
        scope_declare_identifier(new_scope, varname, (val_t) {
             .type = VAL_INTEGER,
             .intval = counter_val
         }, false);

    while ((iter_count_val.type == VAL_BOOLEAN && iter_count_val.boolval->value) ||
           (iter_count_val.type == VAL_INTEGER && counter < iter_count_val.intval->value))
    {
        eval(new_scope, node->loop_stmt->body);
        counter++;

        if (varname != NULL)
            counter_val->value = counter;
    }

    blaze_free(counter_val);
    scope_free(new_scope);
    return BLAZE_NULL;
}

val_t eval_if_stmt(scope_t *scope, const ast_node_t *node)
{
    val_t cond_val = eval(scope, node->if_stmt->condition);

    if (val_is_truthy(&cond_val))
        eval(scope, node->if_stmt->if_block);
    else if (node->if_stmt->else_block != NULL)
        eval(scope, node->if_stmt->else_block);

    return BLAZE_NULL;
}

val_t eval_block(scope_t *scope, const ast_node_t *node)
{
    scope_t *new_scope = scope_init(scope);

    for (size_t i = 0; i < node->block->size; i++)
    {
        eval(new_scope, &node->block->children[i]);
    }

    scope_free(new_scope);
    return BLAZE_NULL;
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

    enum valmap_set_status status = scope_assign_identifier(scope, node->assignment_expr->assignee->identifier->symbol, *(val_copy_deep(&val)));

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

static long long int val_to_int(val_t *val)
{
    if (val->type == VAL_INTEGER)
        return val->intval->value;

    if (val->type == VAL_NULL)
        return 0;

    if (val->type == VAL_BOOLEAN)
        return val->boolval->value;

    return 1;
}

static char *val_stringify(val_t *val)
{
    char *string = NULL;
    val_type_t type = val->type;

    if (type == VAL_STRING)
        string = blaze_strdup(val->strval->value);
    else if (type == VAL_INTEGER)
        asprintf(&string, "%lld", val->intval->value);
    else if (type == VAL_BOOLEAN)
        string = blaze_strdup(val->boolval->value ? "true" : "false");
    else if (type == VAL_NULL)
        string = blaze_strdup("null");

    return string;
}

static val_t eval_concat(val_t *left, val_t *right, const ast_node_t *node)
{
    val_t val = val_create(VAL_STRING);
    val_type_t ltype = left->type;
    val_type_t rtype = right->type;

    val.strval->value = NULL;

    if (ltype == VAL_STRING && rtype == VAL_STRING)
        asprintf(&val.strval->value, "%s%s",
                 left->strval->value, right->strval->value);
    else if (ltype == VAL_STRING && rtype == VAL_INTEGER)
        asprintf(&val.strval->value, "%s%lld",
                 left->strval->value, right->intval->value);
    else if (ltype == VAL_INTEGER && rtype == VAL_STRING)
        asprintf(&val.strval->value, "%lld%s",
                 left->intval->value, right->strval->value);
    else if (ltype == VAL_STRING && rtype == VAL_BOOLEAN)
        asprintf(&val.strval->value, "%s%s",
                 left->strval->value, right->boolval->value ? "true" : "false");
    else if (ltype == VAL_BOOLEAN && rtype == VAL_STRING)
        asprintf(&val.strval->value, "%s%s",
                 left->boolval->value ? "true" : "false", right->strval->value);
    else if (ltype == VAL_NULL && rtype == VAL_STRING)
        asprintf(&val.strval->value, "null%s", right->strval->value);
    else if (ltype == VAL_STRING && rtype == VAL_NULL)
        asprintf(&val.strval->value, "%snull", left->strval->value);
    else
        RUNTIME_ERROR(node->binexpr->left->filename,
                      node->binexpr->left->line_start,
                      node->binexpr->left->column_start,
                      "cannot use operator '%c' with type string",
                      '+');

    blaze_alloca_tbl_push_ptr(val.strval->value);
    return val;
}

static val_t eval_binexp_string(ast_bin_operator_t operator, val_t *left, val_t *right, const ast_node_t *node)
{
    if (operator == OP_PLUS)
        return eval_concat(left, right, node);

    val_t val = val_create(VAL_BOOLEAN);
    const char *left_str = val_stringify(left);
    const char *right_str = val_stringify(right);

    switch (operator)
    {
        case OP_CMP_EQ:
            val.boolval->value = strcmp(left_str, right_str) == 0;
            break;

        case OP_CMP_EQ_S:
            val.boolval->value = strcmp(left_str, right_str) == 0 && left->type == right->type;
            break;

        case OP_CMP_NE:
            val.boolval->value = strcmp(left_str, right_str) != 0;
            break;

        case OP_CMP_NE_S:
            val.boolval->value = strcmp(left_str, right_str) != 0 && left->type == right->type;
            break;

        default:
            fatal_error("unsupported operator '%c' (%d) being used with type string", operator, operator);
            exit(-1);
    }

    return val;
}

static val_t eval_binexp_cmp_generic(ast_bin_operator_t operator, val_t *left, val_t *right, const ast_node_t *node)
{
    if (left->type == VAL_STRING || right->type == VAL_STRING)
        return eval_binexp_string(operator, left, right, node);

    val_t val = val_init();
    val.type = VAL_BOOLEAN;
    long long int li = val_to_int(left);
    long long int ri = val_to_int(right);

    switch (operator)
    {
        case OP_CMP_LT:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            val.boolval->value = li < ri;
            break;

        case OP_CMP_GT:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            val.boolval->value = li > ri;
            break;

        case OP_CMP_GE:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            val.boolval->value = li >= ri;
            break;

        case OP_CMP_LE:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            val.boolval->value = li <= ri;
            break;

        case OP_CMP_EQ:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            val.boolval->value = li == ri;
            break;

        case OP_CMP_EQ_S:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            val.boolval->value = li == ri && left->type == right->type;
            break;

        case OP_CMP_NE:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            val.boolval->value = li != ri;
            break;

        case OP_CMP_NE_S:
            val.boolval = blaze_malloc(sizeof *(val.boolval));
            val.boolval->value = li != ri && left->type == right->type;
            break;

        default:
        {
            fatal_error("unsupported comparison operator '%c' (%d)", operator, operator);
            exit(-1);
        }
    }

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
            fatal_error("unsupported int operator '%c' (%d)", operator, operator);
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
    ast_bin_operator_t operator = node->binexpr->operator;

    if (operator >= OP_CMP_LT && operator <= OP_CMP_NE_S)
        ret = eval_binexp_cmp_generic(operator, &left, &right, node);
    else if (left.type == VAL_INTEGER && right.type == VAL_INTEGER)
        ret = eval_binexp_int(operator, &left, &right, node);
    else if (left.type == VAL_STRING || right.type == VAL_STRING)
        ret = eval_binexp_string(operator, &left, &right, node);
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
