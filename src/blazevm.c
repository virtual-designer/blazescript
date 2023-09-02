/*
* Created by rakinar2 on 8/30/23.
*/

#include "bytecode.h"
#include "disassemble.h"
#include "file.h"
#include "opcode.h"
#include "utils.h"
#include "register.h"
#include <stdio.h>
#include <string.h>

static void process_file(const char *filepath)
{
    struct bytecode bytecode;
    struct filebuf filebuf = filebuf_init(filepath);
    filebuf_read(&filebuf);
    filebuf_close(&filebuf);
    bytecode = bytecode_init_from_stream((uint8_t *) filebuf.content, filebuf.size);
    bytecode_free(&bytecode);
    filebuf_free(&filebuf);
}

static bool is_little_endian()
{
    uint32_t num = 0xCAFEBABE;
    uint8_t *first_byte = (uint8_t *) &num;

    return *first_byte == 0xBE;
}

int main(int argc, char **argv)
{
    if (!is_little_endian())
        fatal_error("blaze vm can only run on a system with an LE CPU");

    uint8_t bytes[] = {
        OP_MOV_IR, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        OP_MOV_IR, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        OP_ADD_RR, 0x00, 0x01,
        OP_MOV_IR, 0x00, SYS_REGDUMP, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //    OP_MOV_IR, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //    OP_REGDUMP,
        OP_SYSCALL,
        OP_MOV_IR, 0x00, SYS_PRINT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        OP_MOV_IR, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        OP_MOV_IR, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        OP_SYSCALL,
        OP_HLT
    };

    size_t size = sizeof (bytes) / sizeof (bytes[0]);

    /* struct bytecode bytecode = {
       .bytes = bytes,
       .size = size,
       .cap = size
       };*/

    struct bytecode bytecode = bytecode_init();

    bytecode_push_byte(&bytecode, OP_MOV_IR);
    bytecode_push_byte(&bytecode, R0);
    bytecode_push_qword(&bytecode, SYS_PRINT);

    bytecode_push_byte(&bytecode, OP_MOV_IR);
    bytecode_push_byte(&bytecode, R1);
    bytecode_push_qword(&bytecode, 0x02);

    bytecode_push_byte(&bytecode, OP_MOV_IR);
    bytecode_push_byte(&bytecode, R2);
    char *str = "Hello world!";
    bytecode_push_qword(&bytecode, (uintptr_t) str);
    bytecode_push_byte(&bytecode, OP_SYSCALL);

    bytecode_push_byte(&bytecode, OP_MOV_IR);
    bytecode_push_byte(&bytecode, R2);
    str = "LMAO!";
    bytecode_push_qword(&bytecode, (uintptr_t) str);

    bytecode_push_byte(&bytecode, OP_SYSCALL);
    bytecode_push_byte(&bytecode, OP_HLT);
    
    disassemble(stdout, &bytecode);

    if (!bytecode_exec(&bytecode))
    {
        bytecode_free(&bytecode);
        fatal_error("%s", bytecode_error);
        free(bytecode_error);
        exit(-1);
    }

    bytecode_free(&bytecode);
    exit(bytecode_exit_code);
    
    if (argc < 2)
       fatal_error("no input file specified");

    process_file(argv[1]);
    return 0;
}
