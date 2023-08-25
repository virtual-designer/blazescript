/*
 * Created by rakinar2 on 8/25/23.
 */

#ifndef BLAZESCRIPT_VALMAP_H
#define BLAZESCRIPT_VALMAP_H

#include "datatype.h"
#include <stddef.h>

typedef struct valmap valmap_t;

struct valmap *valmap_init(size_t size);
struct valmap *valmap_init_default();
val_t *valmap_get(valmap_t *valmap, const char *key);
bool valmap_has(struct valmap *valmap, const char *key);
void valmap_set(struct valmap *valmap, const char *key, val_t *value, bool attempt_free);
void valmap_set_no_free(struct valmap *valmap, const char *key, val_t *value);
void valmap_free(struct valmap *valmap, bool free_values);
size_t valmap_get_capacity(struct valmap *valmap);
size_t valmap_get_count(struct valmap *valmap);
bool valmap_set_no_overwrite(struct valmap *valmap, const char *key, val_t *value, bool attempt_free);
bool valmap_set_no_create(struct valmap *valmap, const char *key, val_t *value, bool attempt_free);

#endif /* BLAZESCRIPT_VALMAP_H */
