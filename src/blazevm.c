/*
* Created by rakinar2 on 8/30/23.
*/

#include "bytecode.h"
#include "disassemble.h"
#include "file.h"
#include "opcode.h"
#include "utils.h"
#include <stdio.h>

static _Noreturn void process_file(const char *filepath)
{
    struct bytecode bytecode;
    struct filebuf filebuf = filebuf_init(filepath);
    filebuf_read(&filebuf);
    filebuf_close(&filebuf);
    bytecode = bytecode_init_from_filebuf(&filebuf);
    filebuf_free(&filebuf);
    disassemble(stdout, &bytecode);

    execution_init();

    if (!bytecode_exec(&bytecode))
    {
        execution_end();
        bytecode_free(&bytecode);
        fatal_error("%s", bytecode_error);
        free(bytecode_error);
        exit(-1);
    }

    execution_end();
    bytecode_free(&bytecode);
    exit(bytecode_exit_code);
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

    /*

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
     */
    
    if (argc < 2)
       fatal_error("no input file specified");

    process_file(argv[1]);
}
