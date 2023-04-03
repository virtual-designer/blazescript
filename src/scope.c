#include <stdlib.h>
#include <stdbool.h>

#include "map.h"
#include "runtimevalues.h"
#include "scope.h"
#include "eval.h"

#define SCOPE_STACK_SIZE 4096

scope_t scope_init(scope_t *parent_scope)
{
    scope_t scope = { .parent = parent_scope };
    scope.identifiers = MAP_INIT(runtime_val_t *, SCOPE_STACK_SIZE);
    return scope;
}

runtime_val_t *scope_declare_identifier(scope_t *scope, char *name, runtime_val_t *value)
{
    if (map_has(&scope->identifiers, name)) 
        eval_error(true, "Cannot redeclare identifier '%s' in this scope as it is already defined", name);

    map_set(&scope->identifiers, name, value);
    return value;
}

scope_t *scope_resolve_identifier_scope(scope_t *scope, char *name)
{
    bool has = map_has(&scope->identifiers, name);

    if (scope->parent == NULL && !has) 
        eval_error(true, "Undefined identifier '%s' in the current scope", name);
    else if (scope->parent != NULL && !has)
        return scope_resolve_identifier_scope(scope->parent, name);

    return scope;
}

runtime_val_t *scope_resolve_identifier(scope_t *scope, char *name)
{
    scope_t *found_scope = scope_resolve_identifier_scope(scope, name);
    return map_get(&found_scope->identifiers, name);
}

void scope_free(scope_t *scope)
{
    map_free(&scope->identifiers, false);
}
