/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "utils.h"
#include "log.h"
#include "alloca.h"

void fatal_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_error_va_list(fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void syntax_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "\033[1;31msyntax error:\033[0m ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

char *ctos(char c)
{
    char *s = xmalloc(2);
    s[0] = c;
    s[1] = 0;
    return s;
}