/*
 * Created by rakinar2 on 8/30/23.
 */

#ifndef BLAZESCRIPT_BYTECODE_H
#define BLAZESCRIPT_BYTECODE_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "alloca.h"
#include "file.h"

#define BYTECODE_INIT { .bytes = xcalloc(10, sizeof (uint8_t)), .size = 0, .cap = 10 };

struct bytecode
{
    uint8_t *bytes;
    size_t size;
    size_t cap;
};

struct bytecode bytecode_init();
void bytecode_free(struct bytecode *bytecode);
struct bytecode bytecode_init_from_stream(uint8_t *stream, size_t stream_size);
struct bytecode bytecode_init_from_filebuf(struct filebuf *filebuf);

void bytecode_push_byte(struct bytecode *bytecode, uint8_t byte);
void bytecode_push_word(struct bytecode *bytecode, uint16_t word);
void bytecode_push_dword(struct bytecode *bytecode, uint32_t dword);
void bytecode_push_qword(struct bytecode *bytecode, uint64_t qword);

uint8_t bytecode_get_byte(struct bytecode *bytecode, size_t addr);
uint16_t bytecode_get_word(struct bytecode *bytecode, size_t addr);
uint32_t bytecode_get_dword(struct bytecode *bytecode, size_t addr);
uint64_t bytecode_get_qword(struct bytecode *bytecode, size_t addr);

bool bytecode_exec(struct bytecode *bytecode);

extern char *bytecode_error;
extern uint8_t bytecode_exit_code;

#endif /* BLAZESCRIPT_BYTECODE_H */
