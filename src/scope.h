#ifndef __SCOPE_H__
#define __SCOPE_H__

#include "map.h"
#include "runtimevalues.h"

typedef struct scope {
    struct scope *parent;
    map_t identifiers;
} scope_t;

scope_t scope_init(scope_t *parent_scope);
runtime_val_t *scope_declare_identifier(scope_t *scope, char *name, runtime_val_t *value);
void scope_free(scope_t *scope);
runtime_val_t *scope_resolve_identifier(scope_t *scope, char *name);

#endif
