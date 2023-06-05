#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>

#include "blaze.h"
#include "config.h"

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

void utils_info(const char *fmt, ...) 
{
    char *fmtadd = COLOR("1", "%s") ": " COLOR("1;38", "info") ": %s\n";
    size_t len = strlen(fmtadd) + strlen(config.progname);
    char str[len];
    va_list args;

    sprintf(str, fmtadd, config.progname, fmt);

    va_start(args, fmt);
    vfprintf(stderr, str, args);
    va_end(args);
}

void utils_warn(const char *fmt, ...) 
{
    char *outstr = NULL;
    va_list args;

    va_start(args, fmt);
    vasprintf(&outstr, fmt, args);
    fprintf(stderr, COLOR("1", "%s") ": " COLOR("1;35", "warning") ": %s\n", config.progname, outstr);
    free(outstr);
    va_end(args);
}

const char *utils_blazevm_full_path()
{
    const char *path = getenv("BLAZE_INTERPRETER");
    return path != NULL ? path : BLAZE_VM_FULL_PATH;
}
