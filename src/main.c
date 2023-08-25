#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"

void process_file(const char *name)
{
    struct filebuf buf;
    struct lex *lex;
    struct parser *parser;

    buf = filebuf_init(name);
    filebuf_read(&buf);
    lex = lex_init((char *) name, buf.content);
    lex_analyze(lex);
    blaze_debug__lex_print(lex);
    parser = parser_init_from_lex(lex);
    ast_node_t *node = parser_create_ast_node(parser);
    blaze_debug__print_ast(node);
    val_t *val = eval(node);
    print_val(val);
    val_free(val);
    parser_ast_free(node);
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