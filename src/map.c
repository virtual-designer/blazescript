#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <md2.h>

#include "map.h"
#include "blaze.h"
#include "xmalloc.h"

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
    // unsigned char digest[16];
    // struct MD2Context context;

    // MD2Init(&context);
    // MD2Update(&context, key, len);
    // MD2Final(digest, &context);

    // printf("\n------ Sum: ");

    for (int i = 0; i < len; i++) {
        // sum += digest[i];
        sum += key[i];
        // printf("%u ", digest[i]);
    }

    // printf("\n");

    return sum % map->size;
}

void map_set(map_t *map, char *key, void *ptr)
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

void map_set_free(map_t *map, char *key, void *_ptr)
{
    void *ptr = map_get(map, key);

    if (ptr)
        free(ptr);

    map_set(map, key, _ptr);
}

void *map_get(map_t *map, char *key)
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
    uint32_t hash = hash_function(map, key);

    while (map->size > hash && map->array[hash] != NULL)
    {
        if (strcmp(map->array[hash]->key, key) == 0)
            return true;

        hash++;
        hash %= map->size;
    }

    return map->array[hash] != NULL;
}

void *map_free(map_t *map, bool __recursive_free)
{  
    for (size_t i = 0; i < map->size; i++)
    {
        if (map->array[i] != NULL)  
        {
            free(map->array[i]->key);

            if (__recursive_free)
                free(map->array[i]->value);

            free(map->array[i]);
        }
    }

    free(map->array);
    map->array = NULL;
}

void __debug_map_print(map_t *map, bool printnull)
{
    printf("Size: %lu\n\n", map->size);

    for (size_t i = 0; i < map->size; i++)
    {
        if (printnull && map->array[i] == NULL)
            printf("[%lu]: NULL\n", i);
        else
            printf("[%lu]: %s => %p\n", i, map->array[i]->key, map->array[i]->value);
    }
}
