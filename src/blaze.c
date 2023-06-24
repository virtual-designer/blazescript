#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <stdbool.h>
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
#include "functions.h"

#define _GNU_SOURCE

config_t config = { 0 };

static function_t __native_functions[] = {
    { "println", NATIVE_FN_REF(println) },
    { "print", NATIVE_FN_REF(print) },
    { "sleep", NATIVE_FN_REF(sleep) },
    { "pause", NATIVE_FN_REF(pause) },
    { "typeof", NATIVE_FN_REF(typeof) },
    { "read", NATIVE_FN_REF(read) }
};

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

    runtime_val_t *null_val = xmalloc(sizeof (runtime_val_t)),
                  *true_val = xmalloc(sizeof (runtime_val_t)),
                  *false_val = xmalloc(sizeof (runtime_val_t)),
                  *system_val = xmalloc(sizeof (runtime_val_t));

    memcpy(null_val, &_null_val, sizeof _null_val);
    memcpy(true_val, &_true_val, sizeof _true_val);
    memcpy(false_val, &_false_val, sizeof _false_val);
    memcpy(system_val, &_system_val, sizeof _system_val);

    system_val->properties = (map_t) MAP_INIT(identifier_t *, 2);

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

    for (size_t i = 0; i < (sizeof (__native_functions) / sizeof (__native_functions[0])); i++)
    {
        runtime_val_t _fnval = {
            .type = VAL_NATIVE_FN,
            .fn = __native_functions[i].callback
        };

        runtime_val_t *fnval = xmalloc(sizeof _fnval);
        memcpy(fnval, &_fnval, sizeof _fnval);

        scope_declare_identifier(&global, __native_functions[i].name, fnval, true);
    }

    return global;
}

int main(int argc, char **argv) 
{
    atexit(cleanup);
    config.progname = argv[0];
    FILE *fp = NULL;

    if (argc >= 2) {
#if defined(__WIN32__)
        fp = fopen(argv[1], "rb");
#else
        fp = fopen(argv[1], "r");
#endif
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
        if (content == NULL)
            break;
        
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
    scope_free(&global);    
    return 0;
}
