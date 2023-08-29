/*
 * Created by CodeDiseaseDev.
 */

#include "vector.h"
#include "alloca.h"
#include "utils.h"
#include <stdlib.h>

vector_t *vector_init()
{
    vector_t *vector = blaze_malloc(sizeof(vector_t));
    vector->length = 0;
    vector->data = NULL;
    return vector;
}

size_t vector_push(vector_t *vector, void *element)
{
    vector->data = blaze_realloc(vector->data, sizeof(void *) * (++vector->length));
    vector->data[vector->length - 1] = element;
    return vector->length - 1;
}

void *vector_at(vector_t *vector, size_t index)
{
    if (index >= vector->length)
        fatal_error("Index out of bound: trying to access index %lu of a vector with length %lu", index, vector->length);

    return vector->data[index];
}

void vector_free(vector_t *vector)
{
    blaze_free(vector->data);
    blaze_free(vector);
}