#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"

void process_file(const char *name)
{
    struct filebuf buf;
    struct lex *lex;
    struct parser *parser;

    buf = filebuf_init(name);
    filebuf_read(&buf);
    lex = lex_init(buf.content);
    lex_analyze(lex);
    blaze_debug__lex_print(lex);
    parser = parser_init_from_lex(lex);
    parser_free(parser);
    lex_free(lex);
    filebuf_free(&buf);
}

int main(int argc, char **argv)
{
    if (argc < 2)
        fatal_error("no input file specified");

    process_file(argv[1]);
    return 0;
}