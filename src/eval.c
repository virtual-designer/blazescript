#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "eval.h"
#include "scope.h"
#include "ast.h"
#include "blaze.h"
#include "runtimevalues.h"
#include "xmalloc.h"

#define NUM(node) (node.is_float ? node.floatval : (node.type == VAL_BOOLEAN ? node.boolval : node.intval))

static bool is_float(long double val) 
{
    return ceill(val) != floorl(val);
}

void eval_error(bool should_exit, const char *fmt, ...)
{
    va_list args;
    char fmt_processed[strlen(fmt) + 50];
    va_start(args, fmt);

    sprintf(fmt_processed, "Runtime error: %s\n", fmt);
    vfprintf(stderr, fmt_processed, args);

    va_end(args);

    if (should_exit)
        exit(EXIT_FAILURE);
}

runtime_val_t eval_numeric_binop(runtime_val_t left, runtime_val_t right, ast_operator_t operator)
{
    long double result;

    if (operator == OP_PLUS)
        result = NUM(left) + NUM(right);
    else if (operator == OP_MINUS)
        result = NUM(left) - NUM(right);
    else if (operator == OP_TIMES)
        result = NUM(left) * NUM(right);
    else if (operator == OP_DIVIDE)
    {
        if (NUM(right) == 0)
            eval_error(true, "Result of divison by zero is undefined");

        result = NUM(left) / NUM(right); // TODO: Division by 0 checking
    }
    else if (operator == OP_MOD)
    {
        if (left.is_float || right.is_float)
            eval_error(true, "modulus operator requires the operands to be int, float given");

        if (NUM(right) == 0)
            eval_error(true, "Result of divison by zero is undefined");
        
        result = (long long int) NUM(left) % (long long int) NUM(right); // TODO: Division by 0 checking
    }
    else
        blaze_error(true, "invalid binary operator: %d", operator);
    
    runtime_val_t val = {
        .type = VAL_NUMBER,
        .is_float = is_float(result)
    };

    if (val.is_float)
        val.floatval = result;
    else 
        val.intval = (long long int) result;

    return val;
}

runtime_val_t eval_binop(ast_stmt binop, scope_t *scope)
{
    if (binop.type != NODE_EXPR_BINARY)
    {
        blaze_error(true, "invalid binop found");
    }

    runtime_val_t right = eval(*binop.right, scope);
    runtime_val_t left = eval(*binop.left, scope);

    if ((right.type == VAL_NUMBER && left.type == VAL_NUMBER) ||
        ((right.type == VAL_BOOLEAN || left.type == VAL_BOOLEAN) && 
        (right.type == VAL_NUMBER || left.type == VAL_NUMBER)))
        return eval_numeric_binop(left, right, binop.operator);

    printf("%d %d\n", left.type, right.type);

    return (runtime_val_t) {
        .type = VAL_NULL,
    };
}

runtime_val_t eval_program(ast_stmt prog, scope_t *scope)
{
    runtime_val_t last_eval = { .type = VAL_NULL };

    for (size_t i = 0; i < prog.size; i++)
    {
        last_eval = eval(prog.body[i], scope);
    }

    return last_eval;
}

runtime_val_t eval_var_decl(ast_stmt decl, scope_t *scope)
{
    printf("decl.has_val: %d\n", decl.has_val);

    runtime_val_t *value_heap = xmalloc(sizeof (runtime_val_t));

    assert(value_heap != NULL);

    if (decl.has_val)
    {
        runtime_val_t value = eval(*(decl.varval), scope);
        memcpy(value_heap, &value, sizeof value);
    }
    else 
        value_heap->type = VAL_NULL;

    return *scope_declare_identifier(scope, decl.identifier, value_heap);
}

runtime_val_t eval_identifier(ast_stmt identifier, scope_t *scope)
{
    return *scope_resolve_identifier(scope, identifier.symbol);
}

runtime_val_t eval(ast_stmt astnode, scope_t *scope)
{
    runtime_val_t val;

    switch (astnode.type)
    {
        case NODE_NUMERIC_LITERAL:
            val.type = VAL_NUMBER;
            val.is_float = is_float(astnode.value);

            if (val.is_float)
                val.floatval = astnode.value;
            else
                val.intval = astnode.value;
        break;

        case NODE_DECL_VAR:
            return eval_var_decl(astnode, scope);

        case NODE_IDENTIFIER:
            return eval_identifier(astnode, scope);

        case NODE_PROGRAM:
            return eval_program(astnode, scope);

        case NODE_EXPR_BINARY:
            return eval_binop(astnode, scope);

        default:
            fprintf(stderr, "Eval error: this AST node is not supported\n");
            exit(EXIT_FAILURE);
    }

    return val;
}
