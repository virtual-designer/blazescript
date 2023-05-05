#ifndef __SCOPE_H__
#define __SCOPE_H__

#include "map.h"
#include "runtimevalues.h"

#define IDENTIFIER_MAX 256
#define ARGS_MAX 32

typedef struct scope {
    struct scope *parent;
    map_t identifiers;
    char *name;
    bool is_broken;
    bool is_continued;
} scope_t;

scope_t scope_init(scope_t *parent_scope);
identifier_t *scope_declare_identifier(scope_t *scope, char *name, runtime_val_t *value, bool is_const);
void scope_free(scope_t *scope);
runtime_val_t *scope_assign_identifier(scope_t *scope, char *name, runtime_val_t *value);
identifier_t *scope_resolve_identifier(scope_t *scope, char *name);
void scope_runtime_val_free(runtime_val_t *val);

#endif
