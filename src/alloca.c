/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "alloca.h"
#include "utils.h"

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
        fatal_error("could not allocate memory: %s", strerror(errno));

    return ptr;
}


void *xcalloc(size_t n, size_t size)
{
    void *ptr = calloc(n, size);

    if (ptr == NULL)
        fatal_error("could not allocate memory: %s", strerror(errno));

    return ptr;
}

void *xrealloc(void *old_ptr, size_t new_size)
{
    void *ptr = realloc(old_ptr, new_size);

    if (ptr == NULL)
        fatal_error("could not allocate memory: %s", strerror(errno));

    return ptr;
}