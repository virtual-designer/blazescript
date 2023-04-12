#ifndef __XMALLOC_H__
#define __XMALLOC_H__

#include <stddef.h>
#include <stdlib.h>

#define xnfree(ptr) do { if (ptr) xfree(ptr); ptr = NULL; } while (0)
#define znfree(ptr, ...) do { if (ptr) zfree(ptr, __VA_ARGS__); ptr = NULL; } while (0)  
#define xmemcpy(ptr, type) (type *) copy_heap(ptr, sizeof (type)) 

void *xcalloc(size_t size, size_t blocks);
void *xmalloc(size_t size);
void *xrealloc(void *oldptr, size_t size);
void xfree(void *ptr);
void zfree(void *ptr, const char *fmt, ...);
void xnullfree(void **ptr);
void znullfree(void **ptr, const char *fmt, ...);
void *copy_heap(void *ptr, size_t size);

#endif
