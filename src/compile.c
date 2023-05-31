#include <assert.h>
#include <sys/types.h>
#include <string.h>

#include "opcode.h"
#include "bytecode.h"
#include "compile.h"
#include "ast.h"
#include "vector.h"
#include "utils.h"
#include "runtimevalues.h"

static size_t si = 0;

static void compile_program(ast_stmt astnode, bytecode_t *bytecode)
{
    for (size_t i = 0; i < astnode.size; i++)
        compile(astnode.body[i], bytecode);

    bytecode_push(bytecode, OP_HLT);
}

static void compile_number(ast_stmt astnode, bytecode_t *bytecode)
{
    bytecode_push(bytecode, OP_PUSH);
    bytecode_push(bytecode, (uint8_t) astnode.value);
    si++;
}

static void compile_string(ast_stmt astnode, bytecode_t *bytecode)
{
    size_t len = strlen(astnode.strval);

    bytecode_push(bytecode, OP_PUSH_STR);

    for (size_t i = 0; i < len; i++)
    {
        bytecode_push(bytecode, (uint8_t) astnode.strval[i]);
    }

    bytecode_push(bytecode, STRTERM);
    si++;
}

bool is_number_dt(data_type_t type)
{
    return type == DT_INT || type == DT_FLOAT;
}

data_type_t ast_node_to_dt(ast_stmt node)
{
    if (node.type == NODE_EXPR_BINARY)
    {
        data_type_t left_dt = ast_node_to_dt(*node.left);
        data_type_t right_dt = ast_node_to_dt(*node.right);

        if (is_number_dt(left_dt) && is_number_dt(right_dt))
        {
            return left_dt == DT_FLOAT || right_dt == DT_FLOAT ? DT_FLOAT : DT_INT;
        }
        else if ((left_dt == DT_STRING && right_dt == DT_STRING) ||
                (is_number_dt(left_dt) && right_dt == DT_STRING) ||
                (is_number_dt(right_dt) && left_dt == DT_STRING))
        {
            return DT_STRING;
        }
        else
        {
            utils_error(true, "Unsupported binary operation");
        }
    }

    return node.type == NODE_NUMERIC_LITERAL ? (
        node.is_float ? DT_FLOAT : DT_INT
    ) : (
        node.type == NODE_STRING ? DT_STRING : (
            DT_UNKNOWN
        )
    );
}

runtime_valtype_t inline dt_to_rtval_type(data_type_t type)
{
    return type == DT_INT || type == DT_FLOAT ? (
        VAL_NUMBER
    ) : (
        type == DT_STRING ? VAL_STRING : (
            VAL_NULL
        )
    );
}

static void compile_builtin_call_expr(ast_stmt astnode, bytecode_t *bytecode)
{
    for (ssize_t i = (astnode.args.length - 1); i >= 0; i--)
    {
        ast_stmt arg = VEC_GET(astnode.args, i, ast_stmt);
        compile_force_push(arg, bytecode);
    }

    bytecode_push(bytecode, OP_BUILTIN_FN_CALL);
    bytecode_push(bytecode, astnode.args.length);

    for (size_t i = 0; i < strlen(astnode.callee->identifier); i++)
    {
        bytecode_push(bytecode, astnode.callee->identifier[i]);
    }

    bytecode_push(bytecode, '\0');
}

static void compile_bin_expr(ast_stmt astnode, bytecode_t *bytecode)
{
    compile_force_push(*astnode.left, bytecode);
    compile_force_push(*astnode.right, bytecode);

    uint8_t opcode;

    switch (astnode.operator)
    {
        case OP_PLUS:
            opcode = OP_ADD;
            break;

        case OP_MINUS:
            opcode = OP_SUB;
            break;

        case OP_TIMES:
            opcode = OP_MUL;
            break;

        case OP_DIVIDE:
            opcode = OP_DIV;
            break;

        case OP_MOD:
            opcode = OP_MODULUS;
            break;

        default:
            utils_error(true, "unknown binary operator: %i", astnode.operator);
    }

    bytecode_push(bytecode, opcode);
}

void compile_vardecl(ast_stmt astnode, bytecode_t* bytecode)
{
    bytecode_push(bytecode, OP_DECL_VAR);

    for (size_t i = 0; i < strlen(astnode.identifier); i++)
        bytecode_push(bytecode, astnode.identifier[i]);

    bytecode_push(bytecode, 0);

    if (astnode.has_val)
    {
        compile_force_push(*astnode.varval, bytecode);
        bytecode_push(bytecode, OP_STORE_VARVAL);

        for (size_t i = 0; i < strlen(astnode.identifier); i++)
            bytecode_push(bytecode, astnode.identifier[i]);

        bytecode_push(bytecode, 0);
    }
}

void compile(ast_stmt astnode, bytecode_t *bytecode)
{
    switch (astnode.type)
    {
        case NODE_PROGRAM:
            compile_program(astnode, bytecode);
            return;

        case NODE_EXPR_BINARY:
            compile_bin_expr(astnode, bytecode);
            return;

        case NODE_EXPR_CALL:
            compile_builtin_call_expr(astnode, bytecode);
            return;

        case NODE_DECL_VAR:
            compile_vardecl(astnode, bytecode);
            return;
        
        default:
            utils_error(true, "unsupported AST node found");
            return;
    }
}

void compile_force_push(ast_stmt astnode, bytecode_t *bytecode)
{
    switch (astnode.type)
    {
        case NODE_STRING:
            compile_string(astnode, bytecode);
            return;

        case NODE_NUMERIC_LITERAL:
            compile_number(astnode, bytecode);
            return;

        default:
            return compile(astnode, bytecode);
    }
}