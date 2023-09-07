/*
* Created by rakinar2 on 8/30/23.
*/

#include "opcode.h"
#include "bytecode.h"
#include "register.h"
#include "stack.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STACK_SIZE 4096
#define OPCODE_HANDLER_REF(inst) blazevm__opcode_handler__##inst
#define OPCODE_HANDLER(inst) uint8_t *blazevm__opcode_handler__##inst(struct bytecode *bytecode, uint8_t *ip)

OPCODE_HANDLER(mov_ir);
OPCODE_HANDLER(add_rr);
OPCODE_HANDLER(regdump);
OPCODE_HANDLER(stackdmp);
OPCODE_HANDLER(syscall);
OPCODE_HANDLER(push_r_b);
OPCODE_HANDLER(pop_r_b);

static const size_t opcode_operand_sizes_lut[OPCODE_COUNT][2] = {
    [OP_NO_OP] = {0, 0},
    [OP_HLT] = {0, 0},
    [OP_MOV_IR] = {1, 8},
    [OP_ADD_RR] = {1, 1},
    [OP_SYSCALL] = {0, 0},
    [OP_REGDUMP] = {0, 0},
    [OP_STACK_DMP] = {0, 0},
    [OP_PUSH_R_B] = {1, 0},
    [OP_POP_R_B] = {1, 0},
};

static const addressing_mode_t opcode_operand_addrmode_lut[OPCODE_COUNT][2] = {
    [OP_HLT] = {AM_NONE, AM_NONE},
    [OP_NO_OP] = {AM_NONE, AM_NONE},
    [OP_MOV_IR] = {AM_REGISTER, AM_IMMEDIATE},
    [OP_ADD_RR] = {AM_REGISTER, AM_REGISTER},
    [OP_SYSCALL] = {AM_NONE, AM_NONE},
    [OP_REGDUMP] = {AM_NONE, AM_NONE},
    [OP_STACK_DMP] = {AM_NONE, AM_NONE},
    [OP_PUSH_R_B] = {AM_REGISTER, AM_NONE},
    [OP_POP_R_B] = {AM_REGISTER, AM_NONE},
};

static const char *opcode_to_str_lut[] = {
    [OP_NO_OP] = "noop",
    [OP_HLT] = "hlt",
    [OP_MOV_IR] = "mov",
    [OP_ADD_RR] = "add",
    [OP_SYSCALL] = "syscall",
    [OP_REGDUMP] = "regdump",
    [OP_STACK_DMP] = "stackdmp",
    [OP_PUSH_R_B] = "pushb",
    [OP_POP_R_B] = "popb",
};

static uint8_t *((*handlers_lut[])(struct bytecode *bytecode, uint8_t *ip)) = {
    [OP_NO_OP] = NULL,
    [OP_HLT] = NULL,
    [OP_MOV_IR] = OPCODE_HANDLER_REF(mov_ir),
    [OP_ADD_RR] = OPCODE_HANDLER_REF(add_rr),
    [OP_SYSCALL] = OPCODE_HANDLER_REF(syscall),
    [OP_REGDUMP] = OPCODE_HANDLER_REF(regdump),
    [OP_STACK_DMP] = OPCODE_HANDLER_REF(stackdmp),
    [OP_PUSH_R_B] = OPCODE_HANDLER_REF(push_r_b),
    [OP_POP_R_B] = OPCODE_HANDLER_REF(pop_r_b),
};

static blaze_stack_t stack;

size_t opcode_get_size(opcode_t opcode)
{
    if ((sizeof (opcode_operand_sizes_lut) / sizeof (opcode_operand_sizes_lut[0])) <= opcode)
        return 1;

    size_t size = (opcode_operand_sizes_lut[opcode][0] +
                  opcode_operand_sizes_lut[opcode][1]) + 1;

    return size < 1 ? 1 : size;
}

void opcode_get_operand_info(opcode_t opcode, operand_info_t info[2])
{
    assert(opcode < (sizeof (opcode_operand_sizes_lut) / sizeof (opcode_operand_sizes_lut[0])) && "Invalid opcode");
    const size_t *ptr_sizes = (const size_t *) *(opcode_operand_sizes_lut + opcode);
    addressing_mode_t *ptr_addrmodes = (addressing_mode_t *) *(opcode_operand_addrmode_lut + opcode);

    operand_info_t info1 = {
        .size = ptr_sizes[0],
        .addrmode = ptr_addrmodes[0]
    };

    operand_info_t info2 = {
        .size = ptr_sizes[1],
        .addrmode = ptr_addrmodes[1]
    };

    info[0] = info1;
    info[1] = info2;
}

const char *opcode_to_str(opcode_t opcode)
{
    assert(opcode < (sizeof (opcode_to_str_lut) / sizeof (opcode_to_str_lut[0])) && "Invalid opcode");
    return opcode_to_str_lut[opcode];
}

uint8_t *instruction_exec(opcode_t opcode, struct bytecode *bytecode)
{
    if (opcode >= (sizeof (handlers_lut) / sizeof (handlers_lut[0])))
    {
        bytecode_error = xmalloc(22);
        sprintf(bytecode_error, "Invalid opcode: 0x%02x", opcode);
        return NULL;
    }

    if (handlers_lut[opcode] == NULL)
        return NULL;

    return handlers_lut[opcode](bytecode, (uint8_t *) registers[IP]);
}

bool validate_register(struct bytecode *bytecode, register_type_t id)
{
    if (!is_valid_register_id(id))
    {
        bytecode_error = xmalloc(25);
        sprintf(bytecode_error, "Invalid register: 0x%02x", id);
        return false;
    }

    return true;
}

void execution_init()
{
    stack = blaze_stack_create(STACK_SIZE);
}

void execution_end()
{ blaze_stack_free(&stack);
}

OPCODE_HANDLER(push_r_b)
{
    const uint8_t reg_id = *++ip;

    if (!validate_register(bytecode, reg_id))
        return NULL;

    blaze_stack_push_byte(&stack, registers[reg_id] & 0xFF);
    return ++ip;
}

OPCODE_HANDLER(pop_r_b)
{
    const uint8_t reg_id = *++ip;

    if (!validate_register(bytecode, reg_id))
        return NULL;

    registers[reg_id] = blaze_stack_pop_byte(&stack);
    return ++ip;
}

OPCODE_HANDLER(mov_ir)
{
    const uint8_t reg_id = *++ip;

    if (!validate_register(bytecode, reg_id))
        return NULL;

    registers[reg_id] = bytecode_get_qword(bytecode, (size_t) (ip + 1 - registers[IS]));
    ip += 9;
    return ip;
}

OPCODE_HANDLER(add_rr)
{
    const uint8_t reg1_id = *++ip;
    const uint8_t reg2_id = *++ip;

    if (!validate_register(bytecode, reg1_id) ||
        !validate_register(bytecode, reg2_id))
        return NULL;

    registers[reg1_id] += registers[reg2_id];
    return ++ip;
}

OPCODE_HANDLER(regdump)
{
    puts("\n*** regdump:\n");
    for (size_t i = 0; i < REG_COUNT; i++)
    {
        printf("\033[1;34m%%%s\033[0m     \033[1;32m0x%016lx\033[0m\n",
               register_id_to_str(i),
               registers[i]);
    }

    return NULL;
}

OPCODE_HANDLER(stackdmp)
{
    puts("\n*** stack dump:\n");

    for (size_t i = (stack.index < 10 ? 0 : (stack.index - 10)), c = 0; c < stack.size && c < 10; c++, i++)
    {
        printf("%s\033[1;34m[%zu]\033[0m:     \033[1;32m0x%02x\033[0m\n",
               stack.index == i ? " => " : "    ",
               i,
               stack.bytes[i]);
    }

    return NULL;
}

OPCODE_HANDLER(syscall)
{
    uint64_t r0 = registers[R0];

    switch (r0)
    {
        case SYS_EXIT:
        {
            uint64_t r1 = registers[R1];
            bytecode_exit_code = r1;
            break;
        }

        case SYS_REGDUMP:
        {
            OPCODE_HANDLER_REF(regdump)(bytecode, ip);
            break;
        }

        case SYS_STACK_DUMP:
        {
            OPCODE_HANDLER_REF(stackdmp)(bytecode, ip);
            break;
        }

        case SYS_PRINT:
        {
            uint64_t r1 = registers[R1];
            uint64_t r2 = registers[R2];

            if (r1 == 0x00)
                printf("%llu\n", r2);
            else if (r1 == 0x01)
                printf("%p\n", (uint8_t *) r2);
            else if (r1 == 0x02)
                printf("%s\n", (char *) r2);
            else if (r1 == 0x03)
                printf("%c\n", (char) r2);
            else
            {
                bytecode_error = xmalloc(35);
                sprintf(bytecode_error, "Invalid value type: 0x%02lx", r1);
                return NULL;
            }
            
            break;
        }

        default:
            bytecode_error = xmalloc(30);
            sprintf(bytecode_error, "Invalid syscall: 0x%02lx", r0);
            return NULL;
    }

    return NULL;
}
