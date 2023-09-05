/*
 * Created by rakinar2 on 9/5/23.
 */

#ifndef BLAZESCRIPT_MAP_H
#define BLAZESCRIPT_MAP_H

#include <stddef.h>
#include <stdint.h>

#define MAP_INIT_SIZE 4096

enum map_set_flags {
    MAP_OVERWRITE = 0b00001,
    MAP_CREATE = 0b00010,
    MAP_FREE_ON_OVERWRITE = 0b00100
};

enum map_set_result_flags {
    MAP_RESULT_NOT_OVERWRITTEN = 0b00001,
    MAP_RESULT_NOT_CREATED = 0b00010,
    MAP_RESULT_FREED_ON_OVERWRITE = 0b00100
};

typedef struct {
    char *key;
    void *value;
} map_entry_t;

typedef struct {
    size_t capacity;
    size_t size;
    map_entry_t *elements;
} map_t;

map_t map_create();
unsigned int map_set(map_t *map, char *key, void *value, unsigned int flags);
void map_print(map_t *map);
void map_free(map_t *map);
void *map_get(map_t *map, const char *key);
unsigned int map_set_ret(map_t *map, char *key, void *value, void **old_element, unsigned int flags);

#endif /* BLAZESCRIPT_MAP_H */
