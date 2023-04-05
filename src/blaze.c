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
#include "scope.h"
#include "xmalloc.h"

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

void handle_result(runtime_val_t *result)
{
    if (result->type == VAL_NULL)
        puts("\033[35mnull\033[0m");
    else if (result->type == VAL_BOOLEAN)
        printf("\033[34m%s\033[0m\n", result->boolval ? "true" : "false");
    else if (result->type == VAL_NUMBER)
    {
        if (result->is_float)
            printf("\033[33m%Lf\033[0m\n", result->floatval);
        else
            printf("\033[33m%lld\033[0m\n", result->intval);    
    }
    else 
        printf("Error: %d\n", result->type);
}

scope_t create_global_scope()
{
    scope_t global = scope_init(NULL);  

    runtime_val_t _null_val = {
        .type = VAL_NULL,
    };

    runtime_val_t _true_val = {
        .type = VAL_BOOLEAN,
        .boolval = true
    };

    runtime_val_t _false_val = {
        .type = VAL_BOOLEAN,
        .boolval = false
    };

    runtime_val_t *null_val = xmalloc(sizeof (runtime_val_t)),
                  *true_val = xmalloc(sizeof (runtime_val_t)),
                  *false_val = xmalloc(sizeof (runtime_val_t));

    memcpy(null_val, &_null_val, sizeof _null_val);
    memcpy(true_val, &_true_val, sizeof _true_val);
    memcpy(false_val, &_false_val, sizeof _false_val);

    scope_declare_identifier(&global, "null", null_val, true);
    scope_declare_identifier(&global, "true", true_val, true);
    scope_declare_identifier(&global, "false", false_val, true);

    return global;
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

    scope_t global = create_global_scope();
    runtime_val_t result = eval(prog, &global);
    handle_result(&result);

    // __debug_lex_print_token_array(&lex);
    
    return 0;
}
