/*
 * Created by rakinar2 on 9/23/23.
 */

#include "errmsg.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define IS_NEWLINE(c) ((c) == '\r' || (c) == '\n')

const char *
error_type_to_str(enum error_type type)
{
    const char *translate[] = {
        [ERR_SYNTAX] = "syntax error",
        [ERR_EVAL] = "error",
        [ERR_COMPILE] = "error",
        [ERR_FATAL] = "fatal",
    };

    assert((sizeof (translate) / sizeof (translate[0])) > type && type >= 0);
    return translate[type];
}

static void
errmsg_print_msg_internal(enum error_type type, const char *filename, size_t line_start,
                          size_t col_start, const char *fmt, va_list _args)
{
    va_list args;
    va_copy(args, _args);
    fprintf(stderr, "\033[1;31m%s:\033[0m \033[1m%s:%zu:%zu:\033[0m ",
            error_type_to_str(type), filename, line_start, col_start);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void
errmsg_print_msg(enum error_type type, const char *filename, size_t line_start,
                 size_t col_start, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    errmsg_print_msg_internal(type, filename, line_start, col_start, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

static void
errmsg_print_formatted_src_lines(size_t line_start, size_t line_end, size_t col_start,
                                 size_t col_end, const char *buf)
{
    size_t length = strlen(buf);
    bool skip = true;
    int64_t line = 1;
    int64_t max_spaces = 3 + (int64_t) log10l(line_end);

    for (size_t i = 0; i < length; i++)
    {
        if (line == line_end)
        {
            int64_t spaces = max_spaces + 2;

            while (spaces --> -1)
                fputc(' ', stderr);

            fputs("\033[1;31m", stderr);

            for (size_t column_char_index = 0; column_char_index < col_end; column_char_index++)
            {
                if (column_char_index >= col_start)
                    fputc('^', stderr);
                else
                    fputc(' ', stderr);
            }

            fputs("\033[0m", stderr);
            fputc('\n', stderr);
            fflush(stderr);
            break;
        }

        if (!skip || line == line_start)
        {
            int64_t spaces = max_spaces - (int64_t) log10l(line);

            while (spaces --> 0)
                fputc(' ', stderr);

            fprintf(stderr, "%zu | ", line);

            while (i < length && !IS_NEWLINE(buf[i]))
                fputc(buf[i++], stderr);

            fputc('\n', stderr);
            fflush(stderr);
            skip = false;
        }

        if (IS_NEWLINE(buf[i]))
            line++;
    }
}

void
errmsg_print_formatted(size_t line_start, size_t line_end, size_t col_start,
             size_t col_end, const char *buf, const char *filename, enum error_type type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    errmsg_print_msg_internal(type, filename, line_start, col_start, fmt, args);
    va_end(args);
    errmsg_print_formatted_src_lines(line_start, line_end, col_start, col_end,
                                     buf);
    exit(EXIT_FAILURE);
}