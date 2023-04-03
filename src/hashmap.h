#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdint.h>
#include <sys/types.h>
#include <stdatomic.h>

#include "runtimevalues.h"

#define MAP_INIT_SIZE 3
#define MAP_INIT { .count = 0 }

#ifdef __cplusplus
extern "C" {
#endif

struct map;

typedef struct map map_t;

map_t *map_init();
void map_free(map_t *map);
void map_set(map_t *map, char *key, runtime_val_t *valptr);
void map_delete(map_t *map, char *key, bool _free);
runtime_val_t *map_get(map_t *map, char *key);

#ifdef __cplusplus
}
#endif

#endif
