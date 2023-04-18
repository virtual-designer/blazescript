#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdbool.h>

#define COLOR(codes, text) "\033[" codes "m" text "\033[0m"

void __attribute__((format(printf, 2, 3))) utils_error(bool _exit, const char *fmt, ...);

#endif
