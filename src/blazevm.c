#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include "utils.h"
#include "opcode.h"
#include "bytecode.h"

config_t config = {
    .currentfile = NULL,
    .entryfile = NULL,
    .outfile = NULL,
    .progname = NULL
};

static bytecode_t bytecode;

static void cleanup()
{
    bytecode_free(&bytecode);
}

int main(int argc, char **argv)
{
    config.progname = basename(argv[0]);
    atexit(&cleanup);

    if (argc < 2)
        utils_error(true, "no input file");
    
    FILE *file = fopen(argv[1], "rb");

    if (!file)
        utils_error(true, "%s: failed to open file: %s", argv[1], strerror(errno));

    config.currentfile = argv[1];
    config.entryfile = argv[1];

    opcode_init();
    bytecode_read_from_file(&bytecode, file);
    bytecode_exec(&bytecode);

    if (bytecode.error != NULL)
        utils_error(true, "%s", bytecode.error);

    return 0;
}
