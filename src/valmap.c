/*
 * Created by rakinar2 on 8/25/23.
 */

#include "valmap.h"
#include "alloca.h"
#include "datatype.h"
#include "eval.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define VALMAP_DEFAULT_SIZE 4096
#define FNV_OFFSET_BASIS 0xcbf29ce484222325UL
#define FNV_PRIME 0x100000001b3UL

struct valmap
{
    struct valmap_entry *array;
    size_t capacity;
    size_t elements;
};

struct valmap_entry
{
    char *key;
    val_t *value;
};

struct valmap *valmap_init(size_t size)
{
    struct valmap *valmap = xcalloc(1, sizeof (struct valmap));
    valmap->elements = 0;
    valmap->capacity = size;
    valmap->array = xcalloc(size, sizeof (struct valmap_entry));
    return valmap;
}

struct valmap *valmap_init_default()
{
    return valmap_init(VALMAP_DEFAULT_SIZE);
}

static size_t hash_key(size_t capacity, const char *key)
{
    uint64_t hash = FNV_OFFSET_BASIS;

    for (size_t i = 0; i < strlen(key); i++)
    {
        hash ^= key[i];
        hash *= FNV_PRIME;
    }

    return (size_t) (hash & (uint64_t)(capacity - 1));
}

val_t *valmap_get(struct valmap *valmap, const char *key)
{
    size_t index = hash_key(valmap->capacity, key);

    while (valmap->array[index].key != NULL)
    {
        if (strcmp(valmap->array[index].key, key) == 0)
        {
            return valmap->array[index].value;
        }

        index++;

        if (index >= valmap->capacity)
            index = 0;
    }

    return NULL;
}

bool valmap_has(struct valmap *valmap, const char *key)
{
    return valmap_get(valmap, key) != NULL;
}

enum overwrite_mode {
    OW_NO_OVERWRITE,
    OW_NO_CREATE,
    OW_DEFAULT
};

static const char *valmap_set_entry(struct valmap_entry *array,
            size_t capacity, const char *key, val_t *value, size_t *element_count,
            bool attempt_free, enum overwrite_mode overwrite, bool *result)
{
    size_t index = hash_key(capacity, key);

    if (result != NULL)
        *result = false;

    while (array[index].key != NULL)
    {
        if (strcmp(array[index].key, key) == 0)
        {
            if (overwrite == OW_NO_OVERWRITE && result != NULL)
            {
                *result = true;
                return NULL;
            }

            if (attempt_free) {
                free(array[index].value);
                array[index].value = NULL;
            }

            array[index].value = value;
            return array[index].key;
        }

        index++;

        if (index >= capacity)
            index = 0;
    }

    if (overwrite == OW_NO_CREATE && result != NULL)
    {
        *result = true;
        return NULL;
    }

    array[index].key = strdup(key);
    array[index].value = value;

    if (element_count != NULL)
        (*element_count)++;

    return array[index].key;
}

static void valmap_realloc(struct valmap *valmap, size_t new_capacity)
{
    struct valmap_entry *old_array = valmap->array;
    size_t old_capacity = valmap->capacity;

    valmap->array = xcalloc(new_capacity, sizeof (struct valmap_entry));
    valmap->capacity = new_capacity;

    for (size_t i = 0; i < old_capacity; i++)
    {
        if (old_array[i].key != NULL)
        {
            valmap_set_entry(valmap->array, valmap->capacity, old_array[i].key,
                 old_array[i].value, NULL, false, true, NULL);
            free(old_array[i].key);
        }
    }

    free(old_array);
}

static void valmap_check_realloc(struct valmap *valmap)
{
    if ((((long double) (valmap->elements)) * 1.25) >= valmap->capacity)
    {
        valmap_realloc(valmap, valmap->capacity * 2);
    }
}

void valmap_set(struct valmap *valmap, const char *key, val_t *value, bool attempt_free)
{
    valmap_check_realloc(valmap);
    valmap_set_entry(valmap->array, valmap->capacity, key, value,
 &valmap->elements, attempt_free, OW_DEFAULT, NULL);
}

bool valmap_set_no_overwrite(struct valmap *valmap, const char *key, val_t *value, bool attempt_free)
{
    bool need_to_overwrite = false;
    valmap_check_realloc(valmap);
    valmap_set_entry(valmap->array, valmap->capacity, key, value,
 &valmap->elements, attempt_free, OW_NO_OVERWRITE, &need_to_overwrite);
    return !need_to_overwrite;
}

bool valmap_set_no_create(struct valmap *valmap, const char *key, val_t *value, bool attempt_free)
{
    bool does_not_exist = false;
    valmap_check_realloc(valmap);
    valmap_set_entry(valmap->array, valmap->capacity, key, value,
 &valmap->elements, attempt_free, OW_NO_CREATE, &does_not_exist);
    return !does_not_exist;
}

void valmap_set_no_free(struct valmap *valmap, const char *key, val_t *value)
{
    valmap_set(valmap, key, value, false);
}

void valmap_free(struct valmap *valmap, bool free_values)
{
    for (size_t i = 0; i < valmap->capacity; i++)
    {
        if (valmap->array[i].key != NULL)
        {
            free(valmap->array[i].key);
        }

        if (free_values && valmap->array[i].value != NULL)
        {
            val_free(valmap->array[i].value);
        }
    }

    free(valmap->array);
    free(valmap);
}

size_t valmap_get_capacity(struct valmap *valmap)
{
    return valmap->capacity;
}

size_t valmap_get_count(struct valmap *valmap)
{
    return valmap->elements;
}