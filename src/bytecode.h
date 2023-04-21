#ifndef __COMPILE_H__
#define __COMPILE_H__

#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>

#include "ast.h"
#include "config.h"

#define BYTECODE_INIT { .bytes = NULL, .size = 0, .error = NULL }

typedef struct {
    uint8_t *bytes;
    size_t size;
    char *error;
} bytecode_t;

bytecode_t bytecode_compile(ast_stmt astnode);
void bytecode_write(bytecode_t *bytecode, FILE *file);
void bytecode_read_from_file(bytecode_t *bytecode, FILE *file);
void bytecode_write_magic_header(FILE *file);
void bytecode_write_shebang(FILE *file);
void bytecode_free(bytecode_t *bytecode);
void bytecode_exec(bytecode_t *bytecode);
char *bytecode_error(bytecode_t *bytecode);
void bytecode_set_error(bytecode_t *bytecode, const char *error, ...);
void bytecode_push(bytecode_t *bytecode, uint8_t byte);
void bytecode_push_bytes(bytecode_t *bytecode, uint8_t *byte, size_t len);
void bytecode_disassemble(bytecode_t *bytecode);

#endif
