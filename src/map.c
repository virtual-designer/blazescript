#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "map.h"
#include "blaze.h"
#include "xmalloc.h"
#include "scope.h"

static int callnumber = 1;

map_t map_init(size_t type_size, size_t max_elements) 
{
    return (map_t) {
        .array = xcalloc(sizeof (map_entry_t *), max_elements),
        .count = 0,
        .size = max_elements,
        .typesize = type_size
    };
}

static uint32_t hash_function(map_t *map, char *key)
{
    uint32_t sum = 1;
    size_t len = strlen(key);

    for (size_t i = 0; i < len; i++) {
        sum += key[i] * i;
    }

    return sum % map->size;
}

void map_set(map_t *map, char *key, identifier_t *ptr)
{
    if (map->count >= map->size)
        blaze_error(true, "map_set(): overflow detected");

    uint32_t hash = hash_function(map, key);

    while (map->size > hash && map->array[hash] != NULL && strcmp(map->array[hash]->key, key) != 0)
    {
        hash++;
        hash %= map->size;
    }

    if (map->array[hash] == NULL)
        map->count++;
    else 
        printf("Overwriting: %s\n", key);

    map->array[hash] = xmalloc(sizeof (map_entry_t));
    map->array[hash]->key = strdup(key);
    map->array[hash]->value = ptr;

    callnumber++;
}

void map_delete(map_t *map, char *key, bool _free)
{
    uint32_t hash = hash_function(map, key);

    while (map->size > hash && map->array[hash] != NULL)
    {
        if (strcmp(map->array[hash]->key, key) == 0)
        {
            if (_free)
                free(map->array[hash]->value);

            free(map->array[hash]->key);
            free(map->array[hash]);

            map->array[hash] = NULL;

            map->count--;
            return;
        }

        hash++;
        hash %= map->size;
    }
}

void map_set_free(map_t *map, char *key, identifier_t *_ptr)
{
    void *ptr = map_get(map, key);

    if (ptr)
        free(ptr);

    map_set(map, key, _ptr);
}

identifier_t *map_get(map_t *map, char *key)
{
    uint32_t hash = hash_function(map, key);

    while (map->size > hash && map->array[hash] != NULL)
    {
        if (strcmp(map->array[hash]->key, key) == 0)
            return map->array[hash]->value;

        hash++;
        hash %= map->size;
    }

    return map->array[hash] == NULL ? NULL : map->array[hash]->value;
}

bool map_has(map_t *map, char *key)
{
    return map_get(map, key) != NULL;
}

void map_free(map_t *map, bool __recursive_free)
{  
    for (size_t i = 0; i < map->size; i++)
    {
        if (map->array[i] != NULL)  
        {
            xnfree(map->array[i]->key);

            if (__recursive_free)
            {
                scope_runtime_val_free(map->array[i]->value->value);
                znfree(map->array[i]->value, "Identifier");
            }

            xnfree(map->array[i]);
        }
    }

    xnfree(map->array);
}

void __debug_map_print(map_t *map, bool printnull)
{
    printf("Size: %lu\n\n", map->size);

    for (size_t i = 0; i < map->size; i++)
    {
        if (map->array[i] == NULL)
        {
            if (printnull)
                printf("[%lu]: NULL\n", i);
        }
        else if (map->array[i]->value->value->type == VAL_NUMBER)
        {
            printf("[%lu]: %s => %lld\n", i, map->array[i]->key, map->array[i]->value->value->intval);
        }
        else
            printf("[%lu]: %s => %p\n", i, map->array[i]->key, map->array[i]->value);
    }
}
