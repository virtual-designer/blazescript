#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

#include "bytecode.h"
#include "assemble.h"
#include "bstring.h"
#include "utils.h"
#include "opcode.h"

#define INST_MAX 256
#define OPCODE_ASM_HANDLER_REF(name) __opcode_asm_handler__##name
#define OPCODE_ASM_HANDLER_DEFAULT OPCODE_ASM_HANDLER_REF(default)
#define OPCODE_ASM_HANDLER(name) void __opcode_asm_handler__##name(bytecode_t *bytecode, const char *inst, opcode_t opcode, const char **args, size_t argc)

typedef void (*opcode_asm_handler)(bytecode_t *bytecode, const char *inst, opcode_t opcode, const char **args, size_t argc);

static opcode_asm_handler opcode_asm_handlers[OPCODE_COUNT];

static bool is_pure_str(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isalpha(str[i]))
            return false;
    }

    return true;
}

static bool is_numeric(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]))
            return false;
    }

    return true;
}

static int get_regid(const char *str)
{
    if (strcmp(str, "r0") == 0)
        return 0;
    else if (strcmp(str, "r1") == 0)
        return 1;
    else if (strcmp(str, "r2") == 0)
        return 2;
    else if (strcmp(str, "r3") == 0)
        return 3;
    else if (strcmp(str, "r4") == 0)
        return 4;
    else if (strcmp(str, "r5") == 0)
        return 5;
    else if (strcmp(str, "r6") == 0)
        return 6;
    else
        return -1;
}

static opcode_t inst_to_opcode(const char *inst)
{
    if (strcmp(inst, "hlt") == 0)
        return OP_HLT; 
    else if (strcmp(inst, "nop") == 0)
        return OP_NOP; 
    else if (strcmp(inst, "mov") == 0)
        return OP_MOV; 
    else if (strcmp(inst, "push") == 0)
        return OP_PUSH; 
    else if (strcmp(inst, "dump") == 0)
        return OP_DUMP; 
    else if (strcmp(inst, "regdump") == 0)
        return OP_REGDUMP; 
    else if (strcmp(inst, "add") == 0)
        return OP_REGADD; 
    else if (strcmp(inst, "sub") == 0)
        return OP_REGSUB; 
    else if (strcmp(inst, "mul") == 0)
        return OP_REGMUL; 
    else if (strcmp(inst, "div") == 0)
        return OP_REGDIV; 
    else if (strcmp(inst, "mod") == 0)
        return OP_REGMOD; 
    else if (strcmp(inst, "or") == 0)
        return OP_REGOR; 
    else if (strcmp(inst, "and") == 0)
        return OP_REGAND; 
    else if (strcmp(inst, "xor") == 0)
        return OP_REGXOR; 
    else 
    {
        utils_error(true, "Invalid instruction '%s' (2)", inst);
        return 0;
    }
}

static bool opcode_has_arg(const opcode_t opcode)
{
    switch (opcode) 
    {
        case OP_MOV:
        case OP_PUSH:
        case OP_REGADD:
        case OP_REGSUB:
        case OP_REGMUL:
        case OP_REGDIV:
        case OP_REGMOD:
        case OP_REGOR:
        case OP_REGAND:
        case OP_REGXOR:
            return true;

        default:
            return false;
    }
}

static void args_expect_number(const char **args, int index)
{
    if (!is_numeric(args[index]))
        utils_error(true, "operand #%d of `push' instruction must be numeric", index + 1);
}

static void args_expect_register(const char **args, int index)
{
    if (args[index][0] != '%' || get_regid(args[index] + 1) == -1)
        utils_error(true, "invalid register `%s'", args[index]);
}

static void args_expect_count(const char *inst, int argc, int expected)
{
    if (argc != expected)
        utils_error(true, "`%s' instruction expects exactly %d operands", inst, expected);
}

/* Opcode handlers */

OPCODE_ASM_HANDLER(default)
{
    bytecode_push(bytecode, opcode);
}

OPCODE_ASM_HANDLER(push)
{
    bytecode_push(bytecode, opcode);

    args_expect_count(inst, argc, 1);
    args_expect_number(args, 0);

    int operand = atoi(args[0]);

    bytecode_push(bytecode, operand & 0xFF);
}

static void bytecode_push_dword(bytecode_t *bytecode, int operand)
{
    bytecode_push(bytecode, operand & 0xFF);
    bytecode_push(bytecode, (operand >> 8) & 0xFF);
    bytecode_push(bytecode, (operand >> 16) & 0xFF);
    bytecode_push(bytecode, (operand >> 24) & 0xFF);
}

OPCODE_ASM_HANDLER(binop)
{
    bytecode_push(bytecode, opcode);
    args_expect_count(inst, argc, 2);
    args_expect_register(args, 0);

    if (args[1][0] == '%')
        args_expect_register(args, 1);
    else    
        args_expect_number(args, 1);

    int reg1 = get_regid(args[0] + 1),
        operand2 = args[1][0] == '%' ? get_regid(args[1] + 1) : atoi(args[1]);

    bytecode_push(bytecode, args[1][0] == '%' ? 0x01 : 0x00);
    bytecode_push(bytecode, reg1);

    if (args[1][0] == '%')
        bytecode_push(bytecode, operand2);
    else 
        bytecode_push_dword(bytecode, operand2);
}

OPCODE_ASM_HANDLER(mov)
{
    bytecode_push(bytecode, opcode);

    args_expect_count(inst, argc, 2);
    args_expect_register(args, 0);
    args_expect_number(args, 1);

    int regid = get_regid(args[0] + 1);
    int operand = atoi(args[1]);

    bytecode_push(bytecode, regid);
    bytecode_push_dword(bytecode, operand);
}

/* End opcode handlers */

static void opcode_asm_handlers_init()
{
    for (size_t i = 0; i < OPCODE_COUNT; i++)
        opcode_asm_handlers[i] = OPCODE_ASM_HANDLER_DEFAULT;

    opcode_asm_handlers[OP_MOV] = OPCODE_ASM_HANDLER_REF(mov);
    opcode_asm_handlers[OP_PUSH] = OPCODE_ASM_HANDLER_REF(push);
    opcode_asm_handlers[OP_REGADD] = OPCODE_ASM_HANDLER_REF(binop);
    opcode_asm_handlers[OP_REGSUB] = OPCODE_ASM_HANDLER_REF(binop);
    opcode_asm_handlers[OP_REGMUL] = OPCODE_ASM_HANDLER_REF(binop);
    opcode_asm_handlers[OP_REGDIV] = OPCODE_ASM_HANDLER_REF(binop);
    opcode_asm_handlers[OP_REGMOD] = OPCODE_ASM_HANDLER_REF(binop);
    opcode_asm_handlers[OP_REGOR] = OPCODE_ASM_HANDLER_REF(binop);
    opcode_asm_handlers[OP_REGAND] = OPCODE_ASM_HANDLER_REF(binop);
    opcode_asm_handlers[OP_REGXOR] = OPCODE_ASM_HANDLER_REF(binop);
}

void assemble(char *code, bytecode_t *bytecode)
{
    opcode_asm_handlers_init();

    size_t len = strlen(code);
    
    if (code[len - 1] != '\n' && code[len - 1] != '\r')
        utils_error(true, "Assembly file must end with a final newline");

    for (size_t i = 0; i < len; i++) 
    {
        if (isspace(code[i]))
            continue;
        
        char *inst = NULL;
        size_t inst_len = 0;

        while (isalpha(code[i]) && i < len) 
        {            
            inst = xrealloc(inst, ++inst_len);
            inst[inst_len - 1] = code[i];
            i++;
        }

        inst = xrealloc(inst, ++inst_len);
        inst[inst_len - 1] = 0;

        while (i < len && (code[i] == '\t' || code[i] == ' '))
            i++;

        opcode_t opcode = inst_to_opcode(inst);

        if (opcode_has_arg(opcode)) 
        {
            char **args = NULL;
            size_t argc = 0;

            while (i < len && code[i] != '\r' && code[i] != '\n' && code[i] != ';')
            {
                char *arg = NULL;
                size_t arg_len = 0;

                while (i < len && code[i] != ',' && code[i] != ';' && code[i] != '\r' && code[i] != '\n')
                {
                    if (isspace(code[i])) 
                    {
                        i++;
                        continue;
                    }

                    arg = xrealloc(arg, ++arg_len);
                    arg[arg_len - 1] = code[i];
                    i++;
                }

                if (code[i] == ',')
                {
                    i++;

                    while (i < len && (code[i] == '\t' || code[i] == ' '))
                        i++;
                }

                arg = xrealloc(arg, ++arg_len);
                arg[arg_len - 1] = 0;

                args = xrealloc(args, sizeof (char *) * ++argc);
                args[argc - 1] = arg;
            }

            opcode_asm_handlers[opcode](bytecode, inst, opcode, (const char **) args, argc);
        }
        else 
            opcode_asm_handlers[opcode](bytecode, inst, opcode, NULL, 0);

        while (i < len && (code[i] == '\t' || code[i] == ' '))
            i++;

        if (code[i] == ';') 
        {
            while (i < len && code[i] != '\n' && code[i] != '\r')
                i++;
        }

        while (i < len && (code[i] == '\t' || code[i] == ' '))
            i++;   

        i--;
    }
}
