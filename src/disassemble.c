/*
 * Created by rakinar2 on 9/1/23.
 */

#include "disassemble.h"
#include "bytecode.h"
#include "opcode.h"
#include "register.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void disassemble_operand(FILE *__restrict__ fp, struct bytecode *bytecode, size_t size, addressing_mode_t mode, size_t i)
{
    assert(mode != AM_NONE);

    if (mode == AM_REGISTER)
    {
        assert(size == 1);
        fprintf(fp, "%%%s", register_id_to_str(bytecode_get_byte(bytecode, i)));
        return;
    }

    switch (size)
    {
        case 1:
            fprintf(fp, "0x%02x", bytecode_get_byte(bytecode, i));
            break;

        case 2:
            fprintf(fp, "0x%x", bytecode_get_word(bytecode, i));
            break;

        case 4:
            fprintf(fp, "0x%x", bytecode_get_dword(bytecode, i));
            break;

        case 8:
            fprintf(fp, "0x%lx", bytecode_get_qword(bytecode, i));
            break;

        default:
            assert(false && "Invalid size");
    }
}

void disassemble(FILE *__restrict__ fp, struct bytecode *bytecode)
{
    const size_t spaces = 10;

    for (size_t i = 0; i < bytecode->size; i++)
    {
        opcode_t opcode = bytecode->bytes[i];
        size_t inst_size = opcode_get_size(opcode);
        operand_info_t info[2];
        opcode_get_operand_info(opcode, info);
        const char *opcode_str = opcode_to_str(opcode);

        fprintf(fp, " %08lx:  ",
               (uint64_t) &bytecode->bytes[i]);

        if (inst_size > 0)
        {
            for (size_t a = 0; a < inst_size; a++)
            {
                fprintf(fp, "%02x ", bytecode->bytes[i + a]);
            }

            for (size_t s = (inst_size * 3) - 1; s < 50; s++)
            {
                printf(" ");
            }
        }

        fprintf(fp, "%s", opcode_str);

        if (inst_size < 2)
        {
            fprintf(fp, "\n");
            continue;
        }
        else
        {
             for (size_t s = strlen(opcode_str); s < spaces; s++)
             {
                 printf(" ");
             }
        }

        i++;

        if (info[0].size > 0)
        {
            disassemble_operand(fp, bytecode, info[0].size, info[0].addrmode, i);

            while (info[0].size --> 0)
                i++;
        }

        if (info[1].size > 0)
        {
            fprintf(fp, ", ");
            disassemble_operand(fp, bytecode, info[1].size, info[1].addrmode, i);

            while (info[1].size --> 0)
                i++;

            i--;
        }

        fprintf(fp, "\n");
    }
}