#ifndef __XMALLOC_H__
#define __XMALLOC_H__

#include <stddef.h>

void *xmalloc(size_t size);
void *xrealloc(void *oldptr, size_t size);

#endif
