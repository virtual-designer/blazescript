/*
 * Created by rakinar2 on 9/22/23.
 */

#ifndef BLAZESCRIPT_LIB_UTILS_H
#define BLAZESCRIPT_LIB_UTILS_H

#include <stdio.h>
#include <stdlib.h>

void  __attribute__((format(printf, 1, 2))) libblaze_fatal_error(const char *fmt, ...);
ssize_t blaze_getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

#endif /* BLAZESCRIPT_LIB_UTILS_H */
