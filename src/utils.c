#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "blaze.h"

void utils_error(bool _exit, const char *fmt, ...) 
{
    char *fmtadd = COLOR("1", "%s") ": " COLOR("1;31", "error") ": %s\n";
    size_t len = strlen(fmtadd) + strlen(config.progname);
    char str[len];
    va_list args;

    sprintf(str, fmtadd, config.progname, fmt);

    va_start(args, fmt);
    vfprintf(stderr, str, args);
    va_end(args);

    if (_exit)
        exit(EXIT_FAILURE);
}
