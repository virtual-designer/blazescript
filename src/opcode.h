#ifndef __OPCODE_H__
#define __OPCODE_H__

#include <inttypes.h>
#include "bytecode.h"

typedef enum {
    OP_NOP = 0x00,
    OP_HLT,
    OP_TEST,
    OP_PUSH,
    OP_POP,
    OP_ADD,
    OP_DUMP,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MODULUS,
    OP_SCOPE,
    OP_RET,
    OP_BUILTIN_FN_CALL,
    OP_PUSH_STR,
    OP_POP_STR,
    OPCODE_COUNT
} opcode_t;

typedef enum {
    FN_PRINTLN
} builtin_fn_t;

typedef uint8_t *(*opcode_handler_t)(uint8_t *ip, bytecode_t *bytecode);

void opcode_init();
opcode_handler_t opcode_get_handler(opcode_t opcode);

#endif
