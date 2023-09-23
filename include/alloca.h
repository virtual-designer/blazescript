/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_ALLOCA_H
#define BLAZESCRIPT_ALLOCA_H

#include <stddef.h>
#include <stdbool.h>

void *xmalloc(size_t size);
void *xcalloc(size_t n, size_t size);
void *xrealloc(void *old_ptr, size_t new_size);

#endif /* BLAZESCRIPT_ALLOCA_H */
