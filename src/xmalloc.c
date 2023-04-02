#include <malloc.h>
#include <stddef.h>
#include <stdbool.h>

#include "xmalloc.h"
#include "blaze.h"

void *xmalloc(size_t size) 
{
    void *ptr = malloc(size);

    if (!ptr) 
        blaze_error(true, "xmalloc: failed to allocate memory");

    return ptr;
}

void *xrealloc(void *oldptr, size_t size) 
{
    void *newptr = realloc(oldptr, size);

    if (!newptr) 
        blaze_error(true, "xrealloc: failed to reallocate memory");

    return newptr;
}
