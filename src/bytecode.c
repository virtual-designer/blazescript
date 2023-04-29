#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "compile.h"
#include "opcode.h"
#include "ast.h"
#include "utils.h"
#include "bytecode.h"
#include "config.h"

#if defined(__WIN32__)
#define EOL "\r\n"
#else
#define EOL "\n"
#endif

#if !defined(vasprintf)
int vasprintf(char **ptr, const char *fmt, ...);
#endif

#define __STDC_WANT_LIB_EXT2__ 1
#define _GNU_SOURCE

static uint8_t magic_bytes_start[] = { 0x23, 0x21 }; /* '#', '!' */

bytecode_t bytecode_compile(ast_stmt astnode)
{
    bytecode_t bytecode = BYTECODE_INIT;
    compile(astnode, &bytecode);
    return bytecode;
}

void bytecode_write_magic_header(FILE *file)
{
    size_t bytes_count = sizeof (magic_bytes_start) / sizeof (magic_bytes_start[0]);

    for (size_t i = 0; i < bytes_count; i++)
        fwrite(&magic_bytes_start[i], sizeof magic_bytes_start[0], 1, file);
}

void bytecode_write_shebang(FILE *file)
{
    const char *path = utils_blazevm_full_path();
    const char *newline = EOL;
    fwrite(path, sizeof (char), strlen(path), file);
    fwrite(newline, sizeof (char), strlen(newline), file);
}

void bytecode_write(bytecode_t *bytecode, FILE *file)
{
    for (size_t i = 0; i < bytecode->size; i++)
    {
        fwrite(&bytecode->bytes[i], sizeof (uint8_t), 1, file);
    }
}

void bytecode_push(bytecode_t *bytecode, uint8_t byte)
{
    bytecode->bytes = xrealloc(bytecode->bytes, ++bytecode->size);
    bytecode->bytes[bytecode->size - 1] = byte;
}

void bytecode_push_bytes(bytecode_t *bytecode, uint8_t *bytes, size_t len)
{
    bytecode->bytes = xrealloc(bytecode->bytes, len + bytecode->size);

    for (size_t i = bytecode->size, j = 0; i < len; i++, j++)
        bytecode->bytes[i] = bytes[j];

    bytecode->size += len;
}

void bytecode_free(bytecode_t *bytecode)
{
    xnfree(bytecode->bytes);
    bytecode->size = 0;
}

void bytecode_trim_shebang(FILE *file)
{
    uint8_t last_read = 0x00;

    fread(&last_read, sizeof (last_read), 1, file);

    if (last_read == magic_bytes_start[0])
    {
        fread(&last_read, sizeof (last_read), 1, file);

        if (last_read != magic_bytes_start[1])
            fseek(file, 0, SEEK_SET);
        else 
        {
            while (last_read != '\n')
            {
                if (feof(file))
                    utils_error(true, "Unexpected end of bytecode file");

                fread(&last_read, sizeof (last_read), 1, file);
            }
        }
    }
    else 
        fseek(file, 0, SEEK_SET);
}

void bytecode_read_from_file(bytecode_t *bytecode, FILE *file)
{
    bytecode_trim_shebang(file);

    while (!feof(file))
    {
        bytecode->bytes = xrealloc(bytecode->bytes, ++bytecode->size);
        fread(&bytecode->bytes[bytecode->size - 1], sizeof (uint8_t), 1, file);
    }
}

char *bytecode_error(bytecode_t *bytecode)
{
    return bytecode->error;
}

void bytecode_set_error(bytecode_t *bytecode, const char *error, ...)
{
    va_list args;
    char *alloc = NULL;

    va_start(args, error);
    vasprintf(&alloc, error, args);
    va_end(args);

    assert(alloc != NULL);
    bytecode->error = alloc;
}

void bytecode_exec(bytecode_t *bytecode)
{
    uint8_t *ip = bytecode->bytes;

    while (*ip != OP_HLT)
    {
        if (*ip == OP_NOP)
            continue;

        opcode_handler_t handler = opcode_get_handler(*ip);

        if (!handler)
        {
            bytecode_set_error(bytecode, "invalid opcode: %zu", *ip);
            return;
        }

        ip = handler(ip, bytecode);

        if (bytecode->error != NULL)
            return;
        
        if (*ip == OP_HLT)
            opcode_get_handler(*ip)(ip, bytecode);
    }
}

void bytecode_disassemble(bytecode_t *bytecode)
{
    uint8_t *ip = bytecode->bytes;

    while (*ip != OP_HLT)
    {
        opcode_handler_t handler = opcode_get_handler(*ip);

        if (!handler)
        {
            utils_error(true, "invalid opcode: %02x", *ip);
            return;
        }

        printf("%p ", ip);

        switch (*ip)
        {
            case OP_NOP:
                puts("nop");
                break;

            case OP_ADD:
                puts("add");
                break;

            case OP_SUB:
                puts("sub");
                break;

            case OP_MUL:
                puts("mul");
                break;

            case OP_DIV:
                puts("div");
                break;

            case OP_MODULUS:
                puts("mod");
                break;

            case OP_PUSH:
                printf("push %u\n", (*++ip));
                break;

            case OP_BUILTIN_FN_CALL:
                {
                    printf("call %s", (++ip));

                    while (*ip != '\0')
                        ip++;

                    ip++;

                    printf(", %u", *ip);

                    uint8_t len = *ip;

                    if (len != 0)
                        printf(", ");
                
                    for (uint8_t i = 0; i < len; i++)
                    {
                        data_type_t type = *++ip;

                        if (type == DT_INT)
                            printf("%u", *++ip);
                        else if (type == DT_FLOAT)
                            printf("%hhu", *++ip);
                        else if (type == DT_STRING)
                        {
                            printf("\"");
                            size_t str_len = *++ip;

                            for (size_t i2 = 0; i2 < str_len; i2++)
                                putchar(*++ip);

                            printf("\"");
                        }

                        if (i != (len - 1))
                            printf(", ");
                    }

                    printf("\n");
                }
                break;

            case OP_POP:
                puts("pop");
                break;

            case OP_DUMP:
                puts("dump");
                break;
        }

        ip++;
    }
}
