/*
 * Created by rakinar2 on 9/19/23.
 */

#include "blazec.h"
#include "log.h"
#include "utils.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void blazec_context_free(struct blazec_context *context)
{
    free(context->outfile);
    free(context->infile);
}

const char *blazec_generation_mode_to_str(enum blazec_generation_mode mode)
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

int main(int argc, char **argv)
{
    int c;
    struct blazec_context context = { 0 };
    context.mode = GEN_LINK_EXEC;

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
                context.outfile = strdup(optarg);
                break;

            case 'S':
                context.mode = GEN_ASM;
                break;

            case 'c':
                context.mode = GEN_OBJ;
                break;

            case 'x':
                context.mode = GEN_LINK_EXEC;
                break;

            case ':':
                fatal_error("option '%s' requires an argument", argv[optind - 1]);

            case '?':
                fatal_error("Invalid option: '%s'", argv[optind - 1]);

            default:
                fatal_error("invalid option code");
        }
    }

    if (optind >= argc)
        fatal_error("no input files");

    if (optind < argc)
        context.infile = strdup(argv[optind]);

    log_info("Compiling '%s'", context.infile);
    log_info("Generate  '%s'", context.outfile);
    log_info("Mode      '%s'", blazec_generation_mode_to_str(context.mode));

    blazec_context_free(&context);
    return 0;
}