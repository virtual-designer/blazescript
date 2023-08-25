/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stdio.h>
#include <stdarg.h>
#include "log.h"

#define LOG_FUNCTION_START \
    va_list args; \
    va_start(args, fmt)

#define LOG_FUNCTION_END \
    va_end(args)

INTERNAL__LOG_FUNCTION(error)
{
    LOG_FUNCTION_START;
    log_error_va_list(fmt, args);
    LOG_FUNCTION_END;
}

void log_error_va_list(const char *fmt, va_list args)
{
    fprintf(stderr, "\033[1;31merror:\033[0m ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

INTERNAL__LOG_FUNCTION(warn)
{
    LOG_FUNCTION_START;
    fprintf(stderr, "\033[1;33mwarning:\033[0m ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    LOG_FUNCTION_END;
}

INTERNAL__LOG_FUNCTION(info)
{
    LOG_FUNCTION_START;
    fprintf(stdout, "\033[36minfo:\033[0m ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    LOG_FUNCTION_END;
}

INTERNAL__LOG_FUNCTION(debug)
{
#ifndef NDEBUG
    LOG_FUNCTION_START;
    fprintf(stdout, "\033[2mdebug:\033[0m ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    LOG_FUNCTION_END;
#endif
}