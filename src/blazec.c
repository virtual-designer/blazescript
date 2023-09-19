/*
 * Created by rakinar2 on 9/19/23.
 */

#define _GNU_SOURCE

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "lexer.h"
#include "log.h"
#include "parser.h"
#include "utils.h"

static struct option const long_options[] = {
    { "output",      required_argument, NULL, 'o' },
    { "assembly",    no_argument,       NULL, 'S' },
    { "compile",     no_argument,       NULL, 'c' },
    { "executable",  no_argument,       NULL, 'x' },
    { 0,             0,                 0,    0  }
};

enum blazec_generation_mode
{
    GEN_ASM,
    GEN_OBJ,
    GEN_LINK_EXEC
};

struct blazec_context
{
    char *outfile;
    char *infile;
    enum blazec_generation_mode mode;
};

static void blazec_context_free(struct blazec_context *context)
{
    free(context->outfile);
    free(context->infile);
}

static const char *blazec_generation_mode_to_str(enum blazec_generation_mode mode)
{
    const char *translate[] = {
        [GEN_ASM] = "ASM",
        [GEN_LINK_EXEC] = "EXECUTABLE",
        [GEN_OBJ] = "OBJECT"
    };

    if ((sizeof (translate) / sizeof (translate[0])) <= mode)
        fatal_error("invalid generation mode");

    return translate[mode];
}

static void blazec_process_options(int argc, char **argv, struct blazec_context *context)
{
    int c;

    opterr = 0;

    while (true)
    {
        int option_index = 1;
        c = getopt_long(argc, argv, ":o:Scx", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 'o':
                context->outfile = strdup(optarg);
                break;

            case 'S':
                context->mode = GEN_ASM;
                break;

            case 'c':
                context->mode = GEN_OBJ;
                break;

            case 'x':
                context->mode = GEN_LINK_EXEC;
                break;

            case ':':
                fatal_error("option '%s' requires an argument", argv[optind - 1]);
                break;

            case '?':
                fatal_error("Invalid option: '%s'", argv[optind - 1]);
                break;

            default:
                fatal_error("invalid option code");
                break;
        }
    }
}

static void blazec_compile_file(int argc, char **argv, struct blazec_context *context)
{
    struct filebuf buf = filebuf_init(context->infile);
    filebuf_read(&buf);
    struct lex lex = lex_init(context->infile, buf.content);
    lex_analyze(&lex);
    struct parser parser = parser_init_from_lex(&lex);
    ast_node_t node = parser_create_ast_node(&parser);
#ifndef NDEBUG
    blaze_debug__print_ast(&node);
#endif
    parser_ast_free_inner(&node);
    parser_free(&parser);
    lex_free(&lex);
    filebuf_close(&buf);
    filebuf_free(&buf);
}

static char *blazec_determine_outfile_name(const char *restrict infile, const enum blazec_generation_mode mode)
{
    char *ret = NULL;
    char *outfile = strdup(infile);
    bool ext_changed = false;
    ssize_t len = (ssize_t) strlen(outfile);
    ssize_t index = len == 0 ? 0 : len - 1;

    if ((index - 3) >= 0 && outfile[index] == 'l'
        && outfile[index - 1] == 'b'
        && outfile[index - 2] == '.')
    {
        outfile[index - (mode == GEN_LINK_EXEC ? 2 : 1)] = 0;
        ext_changed = true;
    }

    if (!ext_changed && mode == GEN_LINK_EXEC)
        ret = strdup("a.out");
    else
        asprintf(&ret, "%s%s%s", outfile,
                 ext_changed ? "" : ".",
                 mode == GEN_LINK_EXEC ? "" :
                 mode == GEN_OBJ ? "o" : mode == GEN_ASM ? "s" : "out");
    free(outfile);
    return ret;
}

int main(int argc, char **argv)
{
    struct blazec_context context = { 0 };
    context.mode = GEN_LINK_EXEC;

    blazec_process_options(argc, argv, &context);

    if (optind >= argc)
        fatal_error("no input files");

    if (optind < argc)
        context.infile = strdup(argv[optind]);

    if (context.outfile == NULL)
        context.outfile = blazec_determine_outfile_name(context.infile, context.mode);

    log_info("Compiling '%s'", context.infile);
    log_info("Generate  '%s'", context.outfile);
    log_info("Mode      '%s'", blazec_generation_mode_to_str(context.mode));

    blazec_compile_file(argc, argv, &context);
    blazec_context_free(&context);
    return 0;
}