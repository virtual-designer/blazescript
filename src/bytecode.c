/*
 * Created by rakinar2 on 8/30/23.
 */

#include "bytecode.h"
#include "alloca.h"
#include "opcode.h"
#include "register.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

char *bytecode_error = NULL;
uint8_t bytecode_exit_code = 0;

struct bytecode bytecode_init()
{
    return (struct bytecode) BYTECODE_INIT;
}

struct bytecode bytecode_init_from_stream(uint8_t *stream, size_t stream_size)
{
    struct bytecode bytecode = {
        .size = stream_size,
        .cap = stream_size
    };

    bytecode.bytes = xcalloc(sizeof (uint8_t), stream_size);
    memcpy(bytecode.bytes, stream, stream_size);
    return bytecode;
}

static void bytecode_check_realloc(struct bytecode *bytecode)
{
    if (bytecode->size >= bytecode->cap)
    {
        bytecode->cap += 20;
        bytecode->bytes = xrealloc(bytecode->bytes, sizeof (uint8_t) * (bytecode->cap));
    }
}

void bytecode_push_byte(struct bytecode *bytecode, uint8_t byte)
{
    bytecode_check_realloc(bytecode);
    bytecode->bytes[bytecode->size++] = byte;
}

void bytecode_push_word(struct bytecode *bytecode, uint16_t word)
{
    bytecode_check_realloc(bytecode);
    memcpy((uint8_t *) (bytecode->bytes + bytecode->size), &word, sizeof (word));
    bytecode->size += sizeof (uint16_t);
}

void bytecode_push_dword(struct bytecode *bytecode, uint32_t dword)
{
    bytecode_check_realloc(bytecode);
    memcpy((uint8_t *) (bytecode->bytes + bytecode->size), &dword, sizeof (dword));
    bytecode->size += sizeof (uint32_t);
}

void bytecode_push_qword(struct bytecode *bytecode, uint64_t qword)
{
    bytecode_check_realloc(bytecode);
    memcpy((uint8_t *) (bytecode->bytes + bytecode->size), &qword, sizeof (qword));
    bytecode->size += sizeof (uint64_t);
}

uint8_t bytecode_get_byte(struct bytecode *bytecode, size_t addr)
{
    assert(addr < bytecode->size && "Address out of range");
    return bytecode->bytes[addr];
}

uint16_t bytecode_get_word(struct bytecode *bytecode, size_t addr)
{
    assert((addr + 1) < bytecode->size && "Address out of range");
    uint64_t word;
    memcpy(&word, bytecode->bytes + addr, sizeof (uint16_t));
    return word;
}

uint32_t bytecode_get_dword(struct bytecode *bytecode, size_t addr)
{
    assert((addr + 3) < bytecode->size && "Address out of range");
    uint64_t dword;
    memcpy(&dword, bytecode->bytes + addr, sizeof (uint32_t));
    return dword;
}

uint64_t bytecode_get_qword(struct bytecode *bytecode, size_t addr)
{
    assert((addr + 7) < bytecode->size && "Address out of range");
    uint64_t qword;
    memcpy(&qword, bytecode->bytes + addr, sizeof (uint64_t));
    return qword;
}

void bytecode_free(struct bytecode *bytecode)
{
    free(bytecode->bytes);
    bytecode->size = 0;
}

bool bytecode_exec(struct bytecode *bytecode)
{
    int i = 0;

    registers[IP] = (uint64_t) bytecode->bytes;
    registers[IS] = (uint64_t) bytecode->bytes;

    while (registers[IP] != 0)
    {
        i++;

        if (i >= 1000)
        {
            fatal_error("Loop exhausted");
        }

        uint8_t *ip = (uint8_t *) registers[IP];

        if (ip >= (bytecode->bytes + bytecode->size))
        {
            bytecode_error = strdup("%ip points to a memory address that is out of range");
            return false;
        }

        if (*ip == OP_HLT)
        {
            puts("System halted");
            return true;
        }

        uint8_t *result = instruction_exec(*ip, bytecode);

        if (bytecode_error != NULL)
        {
            return false;
        }

        if (result == NULL)
        {
            registers[IP]++;
        }
        else
        {
            registers[IP] = (uint64_t) result;
        }
    }

    return true;
}
