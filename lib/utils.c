/*
 * Created by rakinar2 on 9/22/23.
 */
#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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