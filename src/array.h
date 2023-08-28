/*
 * Created by rakinar2 on 8/28/23.
 */

#ifndef BLAZESCRIPT_ARRAY_H
#define BLAZESCRIPT_ARRAY_H

#include <stddef.h>

typedef struct array {
    size_t length;
    void **elements;
} array_t;

array_t *array_init();
size_t array_push(array_t *array, void *element);
void *array_get(array_t *array, size_t index);
void array_free(array_t *array);

#endif /* BLAZESCRIPT_ARRAY_H */
