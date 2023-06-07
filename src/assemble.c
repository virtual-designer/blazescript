#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

#include "bytecode.h"
#include "assemble.h"
#include "string.h"
#include "utils.h"
#include "opcode.h"

#define INST_MAX 256
#define OPCODE_ASM_HANDLER_REF(name) __opcode_asm_handler__##name
#define OPCODE_ASM_HANDLER_DEFAULT OPCODE_ASM_HANDLER_REF(default)
#define OPCODE_ASM_HANDLER(name) void __opcode_asm_handler__##name(bytecode_t *bytecode, opcode_t opcode, const char **args, size_t argc)

typedef void (*opcode_asm_handler)(bytecode_t *bytecode, opcode_t opcode, const char **args, size_t argc);

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
    else 
        utils_error(true, "Invalid instruction '%s' (2)", inst);
}

static bool opcode_has_arg(const opcode_t opcode)
{
    switch (opcode) 
    {
        case OP_MOV:
        case OP_PUSH:
            return true;

        default:
            return false;
    }
}

/* Opcode handlers */

OPCODE_ASM_HANDLER(default)
{
    bytecode_push(bytecode, opcode);
}

OPCODE_ASM_HANDLER(push)
{
    bytecode_push(bytecode, opcode);

    if (argc != 1)
        utils_error(true, "`push' instruction expects exactly 1 operands");

    if (!is_numeric(args[0]))
        utils_error(true, "operand #1 of `push' instruction must be numeric");

    int operand = atoi(args[0]);

    bytecode_push(bytecode, operand & 0xFF);
}

OPCODE_ASM_HANDLER(mov)
{
    bytecode_push(bytecode, opcode);

    if (argc != 2)
        utils_error(true, "`mov' instruction expects exactly 2 operands");

    if (!is_numeric(args[1]))
        utils_error(true, "operand #2 of `mov' instruction must be numeric");

    if (args[0][0] != '%' || get_regid(args[0] + 1) == -1)
        utils_error(true, "invalid register `%s'", args[0]);

    int regid = get_regid(args[0] + 1);
    int operand = atoi(args[1]);

    bytecode_push(bytecode, regid);
    bytecode_push(bytecode, operand & 0xFF);
    bytecode_push(bytecode, (operand >> 8) & 0xFF);
    bytecode_push(bytecode, (operand >> 16) & 0xFF);
    bytecode_push(bytecode, (operand >> 24) & 0xFF);
}

/* End opcode handlers */

static void opcode_asm_handlers_init()
{
    for (size_t i = 0; i < OPCODE_COUNT; i++)
        opcode_asm_handlers[i] = OPCODE_ASM_HANDLER_DEFAULT;

    opcode_asm_handlers[OP_MOV] = OPCODE_ASM_HANDLER_REF(mov);
    opcode_asm_handlers[OP_PUSH] = OPCODE_ASM_HANDLER_REF(push);
}

void assemble(char *code, bytecode_t *bytecode)
{
    opcode_asm_handlers_init();

    size_t len = strlen(code);

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

        opcode_t opcode = inst_to_opcode(inst);

        // printf("%04lx %s  ", i, inst);

        if (opcode_has_arg(opcode)) 
        {
            char **args = NULL;
            size_t argc = 0;

            while (i < len && code[i] != '\r' && code[i] != '\n')
            {
                char *arg = NULL;
                size_t arg_len = 0;

                while (i < len && code[i] != ',' && code[i] != '\r' && code[i] != '\n')
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
                    i++;

                arg = xrealloc(arg, ++arg_len);
                arg[arg_len - 1] = 0;

                args = xrealloc(args, sizeof (char *) * ++argc);
                args[argc - 1] = arg;
            }

            // for (int i2 = 0; i2 < argc; i2++)
            // {
            //     printf("%s", args[i2]);

            //     if (i2 != (argc - 1))
            //         printf(", ");
            // }

            opcode_asm_handlers[opcode](bytecode, opcode, args, argc);
        }
        else 
            opcode_asm_handlers[opcode](bytecode, opcode, NULL, 0);

        // printf("\n");
    }
}