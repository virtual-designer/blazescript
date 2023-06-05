/*
 * This file is a part of BlazeScript.
 *
 * Copyright (C) 2023  OSN Developers.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>

#include "utils.h"
#include "xmalloc.h"
#include "ast.h"
#include "parser.h"
#include "opcode.h"
#include "bytecode.h"
#include "compile.h"

config_t config = {
    .currentfile = NULL,
    .entryfile = NULL,
    .outfile = NULL,
    .progname = NULL
};

char *content = NULL;
size_t content_length = 0;
FILE *input_file = NULL;

static void cleanup()
{
    if (content != NULL)
    {
        free(content);
        content = NULL;
    }

    if (input_file != NULL)
    {
        fclose(input_file);
        input_file = NULL;
    }
}

static ssize_t get_file_size(FILE *file)
{
    struct stat st;

    if (fstat(fileno(file), &st) == -1)
        return -1;

    return st.st_size;
}

static void read_input_file()
{
    ssize_t file_size = get_file_size(input_file);

    if (file_size == -1)
        utils_error(true, "Cannot stat '%s': %s", config.currentfile, strerror(errno));

    content = xmalloc(file_size);

    fread(content, sizeof (char), file_size, input_file);
    content_length = file_size;

    if (content == NULL)
        utils_error(true, "Failed to read file '%s': %s", config.currentfile, strerror(errno));
}

static void init(int argc, char **argv)
{
    if (argc < 2)
        utils_error(true, "No input files");

    input_file = fopen(argv[1], "r");

    if (input_file == NULL)
        utils_error(true, "Cannot open file '%s': %s", argv[1], strerror(errno));

    config.currentfile = basename(argv[1]);
    config.entryfile = config.currentfile;
}

static ast_stmt make_ast_node()
{
    return parser_create_ast(content);
}

static char *make_output_file_name()
{
    char *input_file_name = strdup(config.currentfile);
    size_t len = strlen(input_file_name);

    if (input_file_name[len - 1] == 'l' &&
        input_file_name[len - 2] == 'b' &&
        input_file_name[len - 3] == '.')
    {
        input_file_name[len - 3] = '\0';
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

static void write_compiled_bytecode(bytecode_t *bytecode)
{
    config.outfile = make_output_file_name();

    FILE *output_file = fopen(config.outfile, "w");

    if (output_file == NULL)
        utils_error(true, "Failed to create output file: %s", strerror(errno));

    bytecode_write_magic_header(output_file);
    bytecode_write_shebang(output_file);
    bytecode_write(bytecode, output_file);

    if (fchmod(fileno(output_file), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
        utils_warn("Cannot change permissions of output file '%s': %s", config.outfile, strerror(errno));

    fclose(output_file);
}

static void begin_compilation()
{
    bytecode_t bytecode;

    if (content_length == 0)
    {
        bytecode_push(&bytecode, OP_HLT);
    }
    else
    {
        ast_stmt ast_node = make_ast_node();
        bytecode = bytecode_compile(ast_node);
        bytecode_disassemble(&bytecode);
    }

    write_compiled_bytecode(&bytecode);
    bytecode_free(&bytecode);
}

int main(int argc, char **argv)
{
    atexit(cleanup);
    config.progname = basename(argv[0]);

    init(argc, argv);
    read_input_file();
    begin_compilation();

    return 0;
}