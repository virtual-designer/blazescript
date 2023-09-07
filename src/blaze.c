#define _GNU_SOURCE

#include <stdlib.h>

#include "alloca.h"
#include "eval.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"
#include "valmap.h"

static struct lex lex;
static struct parser parser;

static void process_file(const char *name)
{
    struct filebuf buf = filebuf_init(name);
    filebuf_read(&buf);
    filebuf_close(&buf);
    lex = lex_init((char *) name, buf.content);
    lex_analyze(&lex);
#ifndef NDEBUG
    blaze_debug__lex_print(&lex);
#endif
    parser = parser_init_from_lex(&lex);
    ast_node_t node = parser_create_ast_node(&parser);
#ifndef NDEBUG
    blaze_debug__print_ast(&node);
#endif
    scope_t *scope = scope_create_global();
    eval(scope, &node);
    scope_destroy_all();
    val_free_global();
    parser_ast_free(&node);
    parser_free(&parser);
    lex_free(&lex);
    filebuf_free(&buf);
}

int main(int argc, char **argv)
{
    /* We've temporarily removed the REPL support. */
    if (argc < 2)
    {
        fatal_error("No input files");
    }

    atexit(&blaze_alloca_tbl_free);
    blaze_alloca_tbl_init();

    process_file(argv[1]);
    return 0;
}
