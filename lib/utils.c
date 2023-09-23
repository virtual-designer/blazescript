/*
 * Created by rakinar2 on 9/22/23.
 */
#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "utils.h"
#include "alloca.h"
#include "log.h"

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
    fprintf(stderr, "\033[1;31mfatal\033[0m ");
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
    exit(-1);
}

char *ctos(char c)
{
    char *s = xmalloc(2);
    s[0] = c;
    s[1] = 0;
    return s;
}

void libblaze_fatal_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    exit(-1);
    va_end(args);
}

ssize_t blaze_getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
#if defined _POSIX_C_SOURCE && _POSIX_C_SOURCE >= 200809L
    ssize_t ret = getline(lineptr, n, stream);

    return ret;
#else
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
#endif
}