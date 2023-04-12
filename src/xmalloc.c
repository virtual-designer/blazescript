#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#include "xmalloc.h"
#include "blaze.h"

void *xmalloc(size_t size) 
{
    void *ptr = malloc(size);

    if (!ptr) 
        blaze_error(true, "xmalloc: failed to allocate memory");

    return ptr;
}

void *xcalloc(size_t size, size_t blocks)
{
    void *ptr = calloc(size, blocks);

    if (!ptr) 
        blaze_error(true, "xcalloc: failed to allocate memory");

    return ptr;
}

void *xrealloc(void *oldptr, size_t size) 
{
    void *newptr = realloc(oldptr, size);

    if (!newptr) 
        blaze_error(true, "xrealloc: failed to reallocate memory");

    return newptr;
}

void xfree(void *ptr)
{
#ifdef _DEBUG
#ifndef _NODEBUG
    printf("[xfree] freeing: %p\n", ptr);
#endif
#endif

    if (ptr)
        free(ptr);
}

void xnullfree(void **ptr) 
{
#ifdef _DEBUG
#ifndef _NODEBUG
    printf("[xnullfree] freeing: %p\n", ptr);
#endif
#endif

    if (ptr == NULL || *((char **) ptr) == NULL)
        return;

    xfree(*((char **) ptr));
    *((char **) ptr) = NULL;
}

void zfree(void *ptr, const char *fmt, ...)
{
#ifdef _DEBUG
#ifndef _NODEBUG
    va_list args;
    va_start(args, fmt);

    printf("[zfree] freeing: %p\n\t", ptr);
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
#endif
#endif

    if (ptr)
        free(ptr);
}

void znullfree(void **ptr, const char *fmt, ...)
{
#ifdef _DEBUG
#ifndef _NODEBUG
    va_list args;
    va_start(args, fmt);

    printf("[znullfree] freeing: %p\n\t", ptr);
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
#endif
#endif

    if (ptr == NULL || *((char **) ptr) == NULL)
        return;

    free(*((char **) ptr));
    *((char **) ptr) = NULL;
}

void *copy_heap(void *ptr, size_t size)
{
    void *alloc = xmalloc(size);
    memcpy(alloc, ptr, size);
    return alloc;
}
