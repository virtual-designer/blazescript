/*
* Created by rakinar2 on 8/30/23.
*/

#ifndef BLAZESCRIPT_OPCODE_H
#define BLAZESCRIPT_OPCODE_H

#include <stddef.h>
#include <stdint.h>
#include "bytecode.h"

typedef enum {
    OP_NO_OP,
    OP_HLT,
    OP_MOV_IR,
    OP_ADD_RR,
    OP_SYSCALL,
    OP_REGDUMP,
    OPCODE_COUNT
} opcode_t;

typedef enum {
    AM_NONE,
    AM_IMMEDIATE,
    AM_REGISTER
} addressing_mode_t;

typedef enum {
    SYS_EXIT,
    SYS_REGDUMP,
    SYS_PRINT,
} syscall_t;

typedef struct {
    size_t size;
    addressing_mode_t addrmode;
} operand_info_t;

const char *opcode_to_str(opcode_t opcode);
size_t opcode_get_size(opcode_t opcode);
void opcode_get_operand_info(opcode_t opcode, operand_info_t info[2]);
uint8_t *instruction_exec(opcode_t opcode, struct bytecode *bytecode);

#endif /* BLAZESCRIPT_OPCODE_H */
