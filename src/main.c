#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "eval.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"
#include "valmap.h"
#include "log.h"

#define REPL_FILENAME "<stdin>"

#define BLAZE_ERROR(...)                                    \
    do {                                                    \
        log_error(__VA_ARGS__);                             \
        exit(EXIT_FAILURE);                                 \
    }                                                       \
    while (0)

static struct lex *lex = NULL;
static struct parser *parser = NULL;
static ast_node_t *root_node = NULL;
static scope_t *global_scope = NULL;

#if !defined(getline)
ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
    *lineptr = NULL;
    *n = 0;
    int c;

    while (!feof(stream) && (c = fgetc(stream)) != '\n')
    {
        *lineptr = xrealloc(*lineptr, ++(*n));
        (*lineptr)[(*n) - 1] = (char) c;
    }

    *lineptr = xrealloc(*lineptr, (*n) + 2);
    (*lineptr)[(*n)++] = '\n';
    (*lineptr)[(*n)++] = 0;
    return (ssize_t) (*n);
}
#endif

static void process_file(const char *name)
{
    struct filebuf buf;

    buf = filebuf_init(name);
    filebuf_read(&buf);
    lex = lex_init((char *) name, buf.content);
    lex_analyze(lex);
#ifndef NDEBUG
    blaze_debug__lex_print(lex);
#endif
    parser = parser_init_from_lex(lex);
    ast_node_t *node = parser_create_ast_node(parser);
#ifndef NDEBUG
    blaze_debug__print_ast(node);
#endif
    scope_t *scope = scope_create_global();
    val_t *val = eval(scope, node);
    print_val(val);
    scope_free(scope);
    val_free_global();
    parser_ast_free(node);
    parser_free(parser);
    lex_free(lex);
    filebuf_free(&buf);
}

static char *get_input()
{
    char *line = NULL;
    size_t n = 0;

    printf("> ");

    if (getline(&line, &n, stdin) == -1)
    {
        BLAZE_ERROR("failed to read input from stdin: %s", strerror(errno));
    }

    return line;
}

static void repl_free()
{
    if (root_node != NULL)
        parser_ast_free(root_node);

    if (parser != NULL)
        parser_free(parser);

    if (lex != NULL)
        lex_free(lex);

    root_node = NULL;
    parser = NULL;
    lex = NULL;
}

static void process_input(const char *input)
{
    lex = lex_init(REPL_FILENAME, (char *) input);

    if (lex_analyze(lex))
    {
        parser = parser_init_from_lex(lex);
        root_node = parser_create_ast_node(parser);

        if (root_node != NULL)
        {
            val_t *val = eval(global_scope, root_node);

            if (val != NULL)
            {
                print_val(val);
            }
        }
    }

    repl_free();
}

static void handle_exit()
{
    if (global_scope != NULL)
        scope_free(global_scope);

    val_free_global();
}

_Noreturn static void start_repl()
{
    atexit(handle_exit);

    filebuf_set_current_file(REPL_FILENAME);
    global_scope = scope_create_global();

    while (true)
    {
        char *input = get_input();

        if (strcmp(input, ".exit\n") == 0)
            exit(EXIT_SUCCESS);

        process_input(input);
        free(input);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        set_repl_mode(true);
        start_repl();
    }

    process_file(argv[1]);
    return 0;
}