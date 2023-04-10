#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <sys/types.h>
#include "xmalloc.h"

#define VEC_INIT { .elements = NULL, .length = 0 }
#define VEC_PUSH(vector, element, type) { \
    vector.elements = xrealloc(vector.elements, (sizeof (type)) * (vector.length + 1)); \
    ((type *) vector.elements)[vector.length++] = (element); \
}
#define VEC_GET(vector, index, type) (((type *) vector.elements)[index])
#define VEC_FREE(vector) { \
    free(vector.elements); \
    vector.elements = NULL; \
    vector.length = 0; \
}
#define VEC_PRINT(vector, format, type) \
    do { \
        for (size_t i = 0; i < vector.length; i++) {\
            printf("[VEC %lu]: %" format "\n", i, VEC_GET(vector, i, type).symbol); \
        } \
    } \
    while (0)

typedef struct {
    void *elements;
    size_t length;
} vector_t;

#endif
