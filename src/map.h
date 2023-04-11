#ifndef __MAP_H__
#define __MAP_H__

#include <stdbool.h>
#include <sys/types.h>

#define MAP_INIT(type, max_elements) map_init(sizeof (type), (max_elements))

typedef struct {
    bool is_const;
    char *name;
    struct runtime_val_t *value;
} identifier_t;

typedef struct {
    char *key;
    identifier_t *value;
} map_entry_t;

typedef struct map {
    map_entry_t **array;
    size_t count;
    size_t size;
    size_t typesize;
} map_t;

map_t map_init(size_t type_size, size_t max_elements);
void map_set(map_t *map, char *key, identifier_t *ptr);
void map_set(map_t *map, char *key, identifier_t *ptr);
identifier_t *map_get(map_t *map, char *key);
void map_free(map_t *map, bool __recursive_free);
void map_delete(map_t *map, char *key, bool _free);
bool map_has(map_t *map, char *key);
map_t map_copy(map_t *map, bool __recursive);

void __debug_map_print(map_t *map, bool printnull);

#endif
