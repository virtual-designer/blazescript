/*
 * Created by rakinar2 on 8/22/23.
 */

#include "utils.h"
#include "alloca.h"
#include "log.h"
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

bool is_repl = false;

void set_repl_mode(bool value)
{
    is_repl = value;
}

void blaze_error_exit()
{
    if (!is_repl)
        exit(EXIT_FAILURE);
}

void fatal_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_error_va_list(fmt, args);
    va_end(args);
    blaze_error_exit();
}

void syntax_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "\033[1;31msyntax error:\033[0m ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    blaze_error_exit();
}

char *ctos(char c)
{
    char *s = blaze_malloc(2);
    s[0] = c;
    s[1] = 0;
    return s;
}