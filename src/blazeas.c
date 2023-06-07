#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "utils.h"
#include "xmalloc.h"
#include "opcode.h"
#include "bytecode.h"
#include "assemble.h"

config_t config = {
    .currentfile = NULL,
    .entryfile = NULL,
    .outfile = NULL,
    .progname = NULL
};

static char *file_content = NULL;
static size_t file_size = 0;

static void cleanup()
{
    free(file_content);
}

static void initialize(int argc, char **argv)
{
    if (argc < 2)
        utils_error(true, "No input files");

    config.entryfile = strdup(argv[1]);
    config.currentfile = strdup(argv[1]);
}

static void read_file() 
{
    FILE *file = fopen(config.currentfile, "rb");

    if (file == NULL)
        utils_error(true, "Cannot open '%s': %s", config.currentfile, strerror(errno));

    struct stat st;

    if (fstat(fileno(file), &st) == -1)
        utils_error(true, "Cannot stat '%s': %s", config.currentfile, strerror(errno));

    file_content = xmalloc(st.st_size);
    file_size = st.st_size;

    fread(file_content, sizeof (char), st.st_size, file);

    if (file_content == NULL)
        utils_error(true, "Failed to read '%s': %s", config.currentfile, strerror(errno));

    fclose(file);
}

static char *make_output_file_name()
{
    char *input_file_name = strdup(config.currentfile);
    size_t len = strlen(input_file_name);

    if (input_file_name[len - 1] == 's' &&
        input_file_name[len - 2] == 'a' &&
        input_file_name[len - 3] == 'l' &&
        input_file_name[len - 4] == 'b' &&
        input_file_name[len - 5] == '.')
    {
        input_file_name[len - 5] = '\0';
        return input_file_name;
    }

    char *output_file_name = xmalloc(len + 5);
    strncpy(output_file_name, input_file_name, len);
    free(input_file_name);

    output_file_name[len] = '.';
    output_file_name[len + 1] = 'b';
    output_file_name[len + 2] = 'v';
    output_file_name[len + 3] = 'm';
    output_file_name[len + 4] = '\0';

    return output_file_name;
}

static void write_output(bytecode_t *bytecode)
{
    char *filename = make_output_file_name();
    config.outfile = strdup(filename);
    FILE *file = fopen(filename, "w");

    if (file == NULL)
        utils_error(true, "Cannot open '%s' for writing: %s", filename, strerror(errno));

    bytecode_write_magic_header(file);
    bytecode_write_shebang(file);
    bytecode_write(bytecode, file);

    if (fchmod(fileno(file), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
        utils_warn("Cannot change permissions of output file '%s': %s", config.outfile, strerror(errno));

    fclose(file);
}

static void start_assembling()
{
    bytecode_t bytecode = BYTECODE_INIT;
    assemble(file_content, &bytecode);
    write_output(&bytecode);
    bytecode_free(&bytecode);
}

int main(int argc, char **argv) 
{
    config.progname = basename(argv[0]);

    atexit(cleanup);
    initialize(argc, argv);
    read_file();
    start_assembling();

    return 0;
}
