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
#include "map.h"

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

char *content = NULL;

void cleanup()
{
    if (content != NULL) 
        free(content);
}

void handle_result(runtime_val_t *result, bool newline, int tabs, bool quote_strings)
{
    if (result->type == VAL_NULL)
        puts("\033[35mnull\033[0m");
    else if (result->type == VAL_BOOLEAN)
        printf("\033[34m%s\033[0m", result->boolval ? "true" : "false");
    else if (result->type == VAL_STRING)
        printf("%s" COLOR("%s", "%s") "%s", quote_strings ? COLOR("2", "\"") : "", quote_strings ? "35" : "0", result->strval, quote_strings ? COLOR("2", "\"") : "");
    else if (result->type == VAL_NUMBER)
    {
        if (result->is_float)
            printf("\033[33m%Lf\033[0m", result->floatval);
        else
            printf("\033[33m%lld\033[0m", result->intval);    
    }
    else if (result->type == VAL_OBJECT)
    {
        printf("Object {\n");

        for (size_t i = 0; i < result->properties.size; i++)
        {
            if (result->properties.array[i] == NULL)
                continue;
            
            for (int i = 0; i < tabs; i++)
                putchar('\t');

            printf("%s: ", result->properties.array[i]->key);
            handle_result(result->properties.array[i]->value->value, false, tabs + 1, true);
            printf(",\n");
        }

        for (int i = 0; i < (tabs - 1); i++)
            putchar('\t');
        
        printf("}");
    }
    else 
        printf("[Unknown Type: %d]", result->type);

    if (newline)
        printf("\n");
}

static runtime_val_t __native_null()
{
    return (runtime_val_t) { .type = VAL_NULL };
}

static runtime_val_t __native_println_fn(vector_t args, scope_t *scope)
{
    for (size_t i = 0; i < args.length; i++)
    {
        runtime_val_t arg = VEC_GET(args, i, runtime_val_t);
        handle_result(&arg, true, 1, false);
    }
    
    VEC_FREE(args);
    return __native_null();
}

static runtime_val_t __native_print_fn(vector_t args, scope_t *scope)
{
    for (size_t i = 0; i < args.length; i++)
    {
        runtime_val_t arg = VEC_GET(args, i, runtime_val_t);
        handle_result(&arg, false, 1, false);
    }
    
    VEC_FREE(args);
    return __native_null();
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

    runtime_val_t _system_val = {
        .type = VAL_OBJECT,
    };

    runtime_val_t _println_val = {
        .type = VAL_NATIVE_FN,
        .fn = &__native_println_fn
    };

    runtime_val_t _print_val = {
        .type = VAL_NATIVE_FN,
        .fn = &__native_print_fn
    };

    runtime_val_t *null_val = xmalloc(sizeof (runtime_val_t)),
                  *true_val = xmalloc(sizeof (runtime_val_t)),
                  *false_val = xmalloc(sizeof (runtime_val_t)),
                  *system_val = xmalloc(sizeof (runtime_val_t)),
                  *print_val = xmalloc(sizeof (runtime_val_t)),
                  *println_val = xmalloc(sizeof (runtime_val_t));

    memcpy(null_val, &_null_val, sizeof _null_val);
    memcpy(true_val, &_true_val, sizeof _true_val);
    memcpy(false_val, &_false_val, sizeof _false_val);
    memcpy(system_val, &_system_val, sizeof _system_val);
    memcpy(println_val, &_println_val, sizeof _println_val);
    memcpy(print_val, &_print_val, sizeof _print_val);

    system_val->properties = (map_t) MAP_INIT(identifier_t *, 1);

    runtime_val_t _version_val = { .type = VAL_STRING, .strval = strdup(VERSION) };
    runtime_val_t *version_val = xmalloc(sizeof _version_val);
    memcpy(version_val, &_version_val, sizeof _version_val);

    identifier_t _version = { .is_const = true, .name = "version", .value = version_val };
    identifier_t *version = xmalloc(sizeof _version);
    memcpy(version, &_version, sizeof _version);

    map_set(&system_val->properties, "version", version);

    scope_declare_identifier(&global, "null", null_val, true);
    scope_declare_identifier(&global, "true", true_val, true);
    scope_declare_identifier(&global, "false", false_val, true);
    scope_declare_identifier(&global, "system", system_val, true);
    scope_declare_identifier(&global, "println", println_val, true);
    scope_declare_identifier(&global, "print", print_val, true);

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

        config.entryfile = argv[1];
        config.currentfile = argv[1];
    }

    assert(fp != NULL);

    char tmpbuf[1024];
    size_t len = 1;
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

        strncat(content, tmpbuf, sizeof tmpbuf);
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
    znfree(content, "Program content");

#ifndef _NODEBUG
#ifdef _DEBUG
    __debug_parser_print_ast_stmt(&prog);
#endif 
#endif 

    scope_t global = create_global_scope();
    runtime_val_t result = eval(prog, &global);
    scope_runtime_val_free(&result);
    scope_free(&global);    
    return 0;
}
