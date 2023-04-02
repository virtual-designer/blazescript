#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "eval.h"
#include "ast.h"
#include "blaze.h"
#include "runtimevalues.h"

static bool is_float(long double val) 
{
    return ceill(val) != floorl(val);
}

#define NUM(node) (node.is_float ? node.floatval : node.intval)

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
        result = NUM(left) / NUM(right);
    else if (operator == OP_MOD)
    {
        if (left.is_float || right.is_float)
            blaze_error(true, "modulus operator requires the operands to be int, float given");
        
        result = left.intval % right.intval;
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

runtime_val_t eval_binop(ast_stmt binop)
{
    if (binop.type != NODE_EXPR_BINARY)
    {
        blaze_error(true, "invalid binop found");
    }

    runtime_val_t right = eval(*binop.right);
    runtime_val_t left = eval(*binop.left);

    if (right.type == VAL_NUMBER && left.type == VAL_NUMBER) {
        return eval_numeric_binop(left, right, binop.operator);
    }

    printf("%d %d\n", left.type, right.type);

    return (runtime_val_t) {
        .type = VAL_NULL,
    };
}

runtime_val_t eval_program(ast_stmt prog)
{
    runtime_val_t last_eval = { .type = VAL_NULL };

    for (size_t i = 0; i < prog.size; i++)
    {
        last_eval = eval(prog.body[i]);
    }

    return last_eval;
}

runtime_val_t eval(ast_stmt astnode)
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

        case NODE_PROGRAM:
            return eval_program(astnode);

        case NODE_EXPR_BINARY:
            return eval_binop(astnode);

        default:
            fprintf(stderr, "Eval error: this AST node is not supported\n");
            exit(EXIT_FAILURE);
    }

    return val;
}
