/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_LOG_H
#define BLAZESCRIPT_LOG_H

#include <stdarg.h>

#define INTERNAL__LOG_FUNCTION(name) \
    void __attribute__((format(printf, 1, 2))) log_##name(const char *fmt, ...)

INTERNAL__LOG_FUNCTION(error);
INTERNAL__LOG_FUNCTION(warn);
INTERNAL__LOG_FUNCTION(info);
INTERNAL__LOG_FUNCTION(debug);

void log_error_va_list(const char *fmt, va_list args);

#endif /* BLAZESCRIPT_LOG_H */
