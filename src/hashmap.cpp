#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdatomic.h>
#include <string.h>

#include "blaze.h"
#include "hashmap.h"
#include "xmalloc.h"
#include "runtimevalues.h"

struct map 
{
    std::map<char *, runtime_val_t *> array;
    size_t count;
};

extern "C" {
    map_t *map_init()
    {
        map_t *map = (map_t *) malloc(sizeof (map_t));

        if (!map) 
        {
            printf("malloc() failed: %s\n", strerror(errno));
            exit(-1);
        }

        std::map<char *, runtime_val_t *> inner_map;

        map->count = 0;
        map->array = inner_map;

        return map;
    }

    void map_free(map_t *map)
    {
        if (map)    
            free(map);
    }

    void map_set(map_t *map, char *key, runtime_val_t *valptr)
    {
        if (!(map->array.count(key) > 0))
            map->count++;
        
        map->array[key] = valptr;
    }

    void map_delete(map_t *map, char *key, bool _free)
    {
        map->count--;
    
        if (_free)
            free(map->array[key]);
        
        map->array.erase(key);
    }

    runtime_val_t *map_get(map_t *map, char *key)
    {
        return map->array[key];
    }
}
