/*
 * Created by rakinar2 on 8/25/23.
 */

#ifndef BLAZESCRIPT_SCOPE_H
#define BLAZESCRIPT_SCOPE_H

#include "datatype.h"
#include "valmap.h"

struct scope
{
    struct scope *parent;
    valmap_t *valmap;
    val_t *null;
};

typedef struct scope scope_t;

struct scope *scope_init(struct scope *parent);
void scope_free(struct scope *scope);
bool scope_assign_identifier(struct scope *scope, const char *name, val_t *val);
bool scope_declare_identifier(struct scope *scope, const char *name, val_t *val);
val_t *scope_resolve_identifier(struct scope *scope, const char *name);

#endif /* BLAZESCRIPT_SCOPE_H */
