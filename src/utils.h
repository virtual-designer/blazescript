/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_UTILS_H
#define BLAZESCRIPT_UTILS_H

#include <stdbool.h>
#include <stdio.h>
#define SYNTAX_ERROR_LINE(token, filename, fmt) \
    syntax_error("\033[0m\033[1m%s\033[0m:%lu:%lu: " fmt, filename, token->line_start, token->column_start)

#define SYNTAX_ERROR_LINE_ARGS(filename, line, column, fmt, ...) \
    syntax_error("\033[0m\033[1m%s\033[0m:%lu:%lu: " fmt, filename, line, column, __VA_ARGS__)

#define RUNTIME_ERROR(filename, line, column, fmt, ...)     \
    do {                                                    \
        log_error("\033[0m\033[1m%s\033[0m:%lu:%lu: " fmt,  \
                  filename, line, column, __VA_ARGS__);     \
                                                            \
        blaze_error_exit();                                 \
    }                                                       \
    while (0)

void fatal_error(const char *fmt, ...);
void syntax_error(const char *fmt, ...);
char *ctos(char c);
void set_repl_mode(bool value);
void blaze_error_exit();

ssize_t blaze_getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

extern bool is_repl;

#endif /* BLAZESCRIPT_UTILS_H */
