/*
 * Created by rakinar2 on 8/28/23.
 */

#ifndef BLAZESCRIPT_VECTOR_H
#define BLAZESCRIPT_VECTOR_H

#include <stddef.h>

#define VECTOR_FOREACH(vector) \
    for (unsigned long int i = 0; i < vector->length; i++)

typedef struct vector
{
    size_t length;
    void **data;
} vector_t;

vector_t *vector_init();
size_t vector_push(vector_t *vector, void *element);
void *vector_at(vector_t *vector, size_t index);
void vector_free(vector_t *vector);

#endif /* BLAZESCRIPT_VECTOR_H */
