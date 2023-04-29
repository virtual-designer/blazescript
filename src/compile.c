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

static void compile_program(ast_stmt astnode, bytecode_t *bytecode)
{
    for (size_t i = 0; i < astnode.size; i++)
        compile(astnode.body[i], bytecode);

    bytecode_push(bytecode, OP_HLT);
}

static void compile_number(ast_stmt astnode, bytecode_t *bytecode)
{
    
}

data_type_t ast_node_to_dt(ast_stmt node)
{
    return node.type == NODE_NUMERIC_LITERAL ? (
        node.is_float ? DT_FLOAT : DT_INT
    ) : (
        node.type == NODE_STRING ? DT_STRING : (
            DT_UNKNOWN
        )
    );
}

runtime_valtype_t dt_to_rtval_type(data_type_t type)
{
    return type == DT_INT || type == DT_FLOAT ? (
        VAL_NUMBER
    ) : (
        type == DT_STRING ? VAL_STRING : (
            VAL_NULL
        )
    );
}

static void compile_callexpr(ast_stmt astnode, bytecode_t *bytecode)
{    
    bytecode_push(bytecode, OP_BUILTIN_FN_CALL);
    bytecode_push_bytes(bytecode, (uint8_t *) astnode.callee->identifier, strlen(astnode.callee->identifier) + 1);
    bytecode_push(bytecode, astnode.args.length);

    for (size_t i = 0; i < astnode.args.length; i++)
    {
        ast_stmt arg = VEC_GET(astnode.args, i, ast_stmt);
        bytecode_push(bytecode, (uint8_t) ast_node_to_dt(arg));

        if (arg.type == NODE_NUMERIC_LITERAL)
            bytecode_push(bytecode, (uint8_t) arg.value);
        else if (arg.type == NODE_STRING)
        {
            size_t len = strlen(arg.strval);
            bytecode_push(bytecode, (uint8_t) len);

            for (size_t i = 0; i < len; i++)
            {
                bytecode_push(bytecode, (uint8_t) arg.strval[i]);
            }
        }
        else
            assert(false && "Unsupported AST Node");
    }
}

static void compile_binexpr(ast_stmt astnode, bytecode_t *bytecode)
{
    compile(*astnode.left, bytecode);
    compile(*astnode.right, bytecode);

    if (astnode.left->type == NODE_NUMERIC_LITERAL)
    {
        bytecode_push(bytecode, OP_PUSH);
        bytecode_push(bytecode, (uint8_t) astnode.left->value);
    }

    if (astnode.right->type == NODE_NUMERIC_LITERAL)
    {
        bytecode_push(bytecode, OP_PUSH);
        bytecode_push(bytecode, (uint8_t) astnode.right->value);
    }

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
    // bytecode_push(bytecode, OP_DUMP);
}

void compile(ast_stmt astnode, bytecode_t *bytecode)
{
    switch (astnode.type)
    {
        case NODE_PROGRAM:
            compile_program(astnode, bytecode);
            return;

        case NODE_NUMERIC_LITERAL:
            compile_number(astnode, bytecode);
            return;

        case NODE_EXPR_BINARY:
            compile_binexpr(astnode, bytecode);
            return;

        case NODE_EXPR_CALL:
            compile_callexpr(astnode, bytecode);
            return;
        
        default:
            utils_error(true, "unsupported AST node found");
            return;
    }
}
