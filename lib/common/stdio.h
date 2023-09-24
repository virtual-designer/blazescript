#ifndef BLAZE_LIB_COMMON_STDIO_H
#define BLAZE_LIB_COMMON_STDIO_H

#include "common.h"

#if defined I386
#include "../i386/stdio.h"
#elif defined X86_64
#include "../x86_64/stdio.h"
#elif defined ARM
#include "../arm/stdio.h"
#elif defined AARCH64
#include "../aarch64/stdio.h"
#else
#error "Unsupported architecture"
#endif

void __attribute__((always_inline)) libblaze_putchar(char c);
void __attribute__((always_inline)) libblaze_puts(char *s);
void __attribute__((always_inline)) libblaze_putstr(char *s);

#endif
