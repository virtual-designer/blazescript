#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "alloca.h"
#include "eval.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"
#include "valalloc.h"
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
    scope_free(scope);
    parser_ast_free_inner(&node);
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

//    struct val_alloc_tbl tbl = val_alloc_tbl_init();
//
//    for (size_t i = 0; i < 10000; i++)
//    {
//        val_t *value = val_alloc(&tbl);
//        value->type = VAL_STRING;
//        value->strval = strdup("KEKW");
//        print_val(value);
//        val_alloc_free(&tbl, value, true);
//    }
//
//    val_alloc_tbl_free(&tbl, true);
//    exit(0);

    atexit(&val_alloc_tbl_global_free);
    val_alloc_tbl_global_init();
    process_file(argv[1]);
    return 0;
}
