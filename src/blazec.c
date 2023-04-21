#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "blaze.h"
#include "parser.h"
#include "utils.h"
#include "bytecode.h"
#include "opcode.h"

config_t config = {
    .currentfile = NULL,
    .entryfile = NULL,
    .outfile = NULL,
    .progname = NULL
};

static char *content = NULL;
static size_t filesize = 0;
static bool disassemble = false;

static void cleanup()
{
    if (content != NULL) 
        free(content);

    free(config.currentfile);
    free(config.entryfile);
    free(config.outfile);
}

static void blazec_init(int argc, char **argv)
{
    if (strcmp(argv[1], "--disassemble") == 0)
    {
        config.currentfile = strdup(argv[2]);
        config.entryfile = strdup(argv[2]);
        disassemble = true;
    }
    else 
    {
        config.currentfile = strdup(argv[1]);
        config.entryfile = strdup(argv[1]);
    }

    size_t namelen = strlen(config.entryfile);
    config.outfile = xmalloc(namelen + 1);
    size_t i;

    for (i = 0; i < namelen; i++)
    {
        if ((i + 2) < namelen && 
            config.currentfile[i] == '.' &&
            config.currentfile[i + 1] == 'b' &&
            config.currentfile[i + 2] == 's')
            break;

        config.outfile[i] = config.currentfile[i];
    }

    config.outfile[i] = '\0';

    // config.outfile[i] = '.';
    // config.outfile[i + 1] = 'b';
    // config.outfile[i + 2] = 'v';
    // config.outfile[i + 3] = 'm';
    // config.outfile[i + 4] = '\0';
}

static bool blazec_is_empty_file()
{
    for (size_t i = 0; content != NULL && i < filesize; i++)
    {
        if (content[i] != '\n' && content[i] != ' ' && content[i] != '\r' && content[i] != '\t')
        {
            return false;
        }
    }

    return true;
}

static void blazec_start_compilation()
{
    FILE *output_file = fopen(config.outfile, "wb");
    bool empty = blazec_is_empty_file();    

    // bytecode_t bytecode = BYTECODE_INIT; 

    // uint8_t bytes[] = {
    //     OP_TEST,
    //     OP_HLT
    // };

    // for (size_t i = 0; i < (sizeof (bytes) / sizeof (bytes[0])); i++)
    // {
    //     bytecode.bytes = xrealloc(bytecode.bytes, ++bytecode.size);
    //     bytecode.bytes[i] = bytes[i];
    // }

    bytecode_write_magic_header(output_file);
    bytecode_write_shebang(output_file);

    if (empty)
    {
        uint8_t hlt = OP_HLT;
        fwrite(&hlt, sizeof hlt, 1, output_file);
        fclose(output_file);
        return;
    }

    ast_stmt prog = parser_create_ast(content);
    znfree(content, "Program content");

    bytecode_t bytecode = bytecode_compile(prog);
    
    if (disassemble)
    {
        opcode_init();
        bytecode_disassemble(&bytecode);
    }
    
    bytecode_write(&bytecode, output_file);
    bytecode_free(&bytecode);
    fchmod(fileno(output_file), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH);
    fclose(output_file);
}

static void blazec_read_file()
{
    FILE *input_file = fopen(config.currentfile, "rb");

    if (!input_file)
        utils_error(true, "%s: cannot open input file: %s", config.currentfile, strerror(errno));

    char tmpbuf[1024];
    size_t len = 1;
    bool is_initial_iteration = true;

    while (fgets(tmpbuf, sizeof tmpbuf, input_file) != NULL)
    {
        size_t newlen = strlen(tmpbuf);
        len += newlen;

        content = xrealloc(content, len);

        if (is_initial_iteration) 
        {
            strcpy(content, "");
            is_initial_iteration = false;
        }

        strncat(content, tmpbuf, sizeof tmpbuf);
    }

    fclose(input_file);
    filesize = len;
}

int main(int argc, char **argv)
{
    atexit(cleanup);
    config.progname = basename(argv[0]);

    if (argc < 2)
    {
        utils_error(true, "no input files");
    }

    blazec_init(argc, argv);
    blazec_read_file();    
    blazec_start_compilation();
    
    return 0;
}
