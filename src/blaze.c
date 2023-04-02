#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define _DEBUG 1

#include "blaze.h"
#include "lexer.h"
#include "parser.h"
#include "eval.h"
#include "runtimevalues.h"

#define _GNU_SOURCE

config_t config = { 0 };

void blaze_error(bool shouldexit, char *format, ...)
{
    char *fmtadd = "%s: %s: %s\n";
    char *error = strerror(errno);
    size_t len = strlen(format) + strlen(fmtadd) + strlen(config.progname) + strlen(error);
    char str[len];
    va_list args;

    sprintf(str, fmtadd, config.progname, format, error);

    va_start(args, format);
    vprintf(str, args);
    va_end(args);

    if (shouldexit)
        exit(EXIT_FAILURE);
}

// lex_t lex = LEX_INIT;
char *content = NULL;

void cleanup()
{
    // lex_free(&lex);
    free(content);
}

int main(int argc, char **argv) 
{
    atexit(cleanup);
    config.progname = argv[0];
    FILE *fp = NULL;

    if (argc >= 2) {
        fp = fopen(argv[1], "r");

        if (fp == NULL) {
            blaze_error(true, "%s", argv[1]);
        }
    }

    assert(fp != NULL);

    char tmpbuf[1024];
    size_t len = 0;
    bool is_initial_iteration = true;

    while (fgets(tmpbuf, sizeof tmpbuf, fp) != NULL)
    {
        size_t newlen = strlen(tmpbuf);

        len += newlen;

        content = realloc(content, len);

        if (content == NULL) 
            blaze_error(true, "failed to reallocate memory");

        if (is_initial_iteration) 
        {
            strcpy(content, "");
            is_initial_iteration = false;
        }

        strcat(content, tmpbuf);
    }

    bool empty = true;

    for (size_t i = 0; i < len; i++)
    {
        if (content[i] != '\n' && content[i] != ' ' && content[i] != '\r' && content[i] != '\t')
        {
            empty = false;
            break;
        }
    }

    if (empty)
        return 0;

    ast_stmt prog = parser_create_ast(content);
    __debug_parser_print_ast_stmt(&prog);

    runtime_val_t result = eval(prog);

    if (result.type == VAL_NULL)
        puts("null");
    else if (result.type == VAL_NUMBER)
    {
        if (result.is_float)
            printf("%Lf\n", result.floatval);
        else
            printf("%lld\n", result.intval);    
    }
    else 
        puts("Error");

    // __debug_lex_print_token_array(&lex);
    
    return 0;
}
