#ifndef __OPCODE_H__
#define __OPCODE_H__

#include <inttypes.h>
#include "bytecode.h"
#include "runtimevalues.h"

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
    OP_DECL_VAR,
    OP_STORE_VARVAL,
    OP_PUSH_VARVAL,
    OP_PRINT,
    OP_MOV,
    OP_REGDUMP,
    OP_REGADD,
    OP_REGSUB,
    OP_REGDIV,
    OP_REGMUL,
    OP_REGMOD,
    OP_REGOR,
    OP_REGAND,
    OP_REGXOR,
    OPCODE_COUNT,
} opcode_t;

typedef enum {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    REG_COUNT
} reg_t;

typedef uint8_t *(*opcode_handler_t)(uint8_t *ip __attribute__((maybe_unused)), bytecode_t *bytecode __attribute__((maybe_unused)));

void opcode_init();
opcode_handler_t opcode_get_handler(opcode_t opcode);

extern runtime_val_t registers[REG_COUNT];

#endif
