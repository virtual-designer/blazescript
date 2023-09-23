/*
 * Created by rakinar2 on 9/23/23.
 */

#ifndef BLAZESCRIPT_ERRMSG_H
#define BLAZESCRIPT_ERRMSG_H

#include <stddef.h>
enum error_type
{
    ERR_SYNTAX,
    ERR_EVAL,
    ERR_COMPILE,
    ERR_FATAL
};

void errmsg_print_formatted(size_t line_start, size_t line_end, size_t col_start,
                       size_t col_end, const char *buf, const char *filename, enum error_type type, const char *fmt, ...);

#endif /* BLAZESCRIPT_ERRMSG_H */
