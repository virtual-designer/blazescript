/*
 * Created by rakinar2 on 9/5/23.
 */

#include "map.h"
#include "alloca.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VALMAP_DEFAULT_SIZE 4096
#define FNV_OFFSET_BASIS 0xcbf29ce484222325UL
#define FNV_PRIME 0x100000001b3UL

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

map_t map_create()
{
    return (map_t) {
        .capacity = MAP_INIT_SIZE,
        .size = 0,
        .elements = xcalloc(sizeof (map_entry_t), MAP_INIT_SIZE)
    };
}

static void map_realloc(map_t *map, size_t new_capacity)
{
    map_entry_t *old_array = map->elements;
    size_t old_capacity = map->capacity;

    map->elements = xcalloc(1, new_capacity);
    map->capacity = new_capacity;

    for (size_t i = 0; i < old_capacity; i++)
    {
        if (old_array[i].key != NULL)
        {
            map_set(map, old_array[i].key, old_array[i].value, MAP_CREATE | MAP_OVERWRITE);
            free(old_array[i].key);
        }
    }

    free(old_array);
}

static void map_check_realloc(map_t *map)
{
    if ((((long double) (map->size)) * 1.25) >= map->capacity)
    {
        map_realloc(map, map->capacity * 2);
    }
}

static const char *valmap_set_entry(map_entry_t *array,
    size_t capacity, const char *key, void *value, size_t *element_count,
    void **old_element,
    unsigned int flags, unsigned int *result_flags)
{
    size_t index = hash_key(capacity, key);

    while (array[index].key != NULL)
    {
        if (strcmp(array[index].key, key) == 0)
        {
            if ((flags & MAP_OVERWRITE) == MAP_OVERWRITE)
            {
                if (old_element != NULL)
                {
                    *old_element = array[index].value;
                }

                if ((flags & MAP_FREE_ON_OVERWRITE) == MAP_FREE_ON_OVERWRITE)
                {
                    if (result_flags != NULL)
                        *result_flags |= MAP_RESULT_FREED_ON_OVERWRITE;

                    free(array[index].value);
                }

                array[index].value = value;
                return array[index].key;
            }
            else
            {
                if (result_flags != NULL)
                    *result_flags |= MAP_RESULT_NOT_OVERWRITTEN;
                return NULL;
            }
        }

        index++;

        if (index >= capacity)
            index = 0;
    }

    if ((flags & MAP_CREATE) == MAP_CREATE)
    {
        array[index].key = strdup(key);
        array[index].value = value;

        if (element_count != NULL)
            (*element_count)++;

        return array[index].key;
    }

    if (result_flags != NULL)
        *result_flags |= MAP_RESULT_NOT_CREATED;

    return NULL;
}

unsigned int map_set(map_t *map, char *key, void *value, unsigned int flags)
{
    unsigned int result = 0;
    map_check_realloc(map);
    valmap_set_entry(map->elements, map->capacity, key, value, &map->size, NULL, flags, &result);
    return result;
}

unsigned int map_set_ret(map_t *map, char *key, void *value, void **old_element, unsigned int flags)
{
    unsigned int result = 0;
    map_check_realloc(map);
    valmap_set_entry(map->elements, map->capacity, key, value, &map->size, old_element, flags, &result);
    return result;
}

void *map_get(map_t *map, const char *key)
{
    size_t index = hash_key(map->capacity, key);

    while (map->elements[index].key != NULL)
    {
        if (strcmp(map->elements[index].key, key) == 0)
        {
            return map->elements[index].value;
        }

        index++;

        if (index >= map->capacity)
            index = 0;
    }

    return NULL;
}

void map_print(map_t *map)
{
    printf("Map (%zu) {%s", map->size, map->size > 0 ? "\n" : "");

    for (size_t i = 0; i < map->capacity; i++)
    {
        if (map->elements[i].key != NULL)
        {
            printf("    \"%s\" (%zu) => %p\n", map->elements[i].key, i, map->elements[i].value);
        }
    }

    printf("}\n");
}

void map_free(map_t *map)
{
    free(map->elements);
}