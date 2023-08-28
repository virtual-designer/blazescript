/*
 * Created by rakinar2 on 8/28/23.
 */

#include <stdlib.h>
#include "array.h"
#include "alloca.h"
#include "utils.h"

array_t *array_init()
{
    array_t *array = xmalloc(sizeof (array_t));
    array->length = 0;
    array->elements = NULL;
    return array;
}

size_t array_push(array_t *array, void *element)
{
    array->elements = xrealloc(array->elements, sizeof (void *) * (++array->length));
    array->elements[array->length - 1] = element;
    return array->length - 1;
}

void *array_get(array_t *array, size_t index)
{
    if (index >= array->length)
        fatal_error("Index out of bound: trying to access index %lu of an array with length %lu", index, array->length);

    return array->elements[index];
}

void array_free(array_t *array)
{
    free(array->elements);
    free(array);
}