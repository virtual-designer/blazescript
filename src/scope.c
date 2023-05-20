#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "map.h"
#include "runtimevalues.h"
#include "scope.h"
#include "eval.h"
#include "xmalloc.h"

#define SCOPE_STACK_SIZE 4096

scope_t scope_init(scope_t *parent_scope)
{
    scope_t scope = { .parent = parent_scope, .is_broken = false, .is_continued = false };
    scope.identifiers = MAP_INIT(identifier_t *, SCOPE_STACK_SIZE);
    scope.name = strdup("Something: %d  ");
    sprintf(scope.name, "Something: %d", rand() % 9);
    return scope;
}

identifier_t *scope_declare_identifier(scope_t *scope, char *name, runtime_val_t *value, bool is_const)
{
    if (map_has(&scope->identifiers, name)) 
        eval_error(true, "Cannot redeclare identifier '%s' in this scope", name);

    identifier_t identifier = {
        .is_const = is_const,
        .name = name,
        .value = value
    };

    identifier_t *identifier_heap = xmalloc(sizeof (identifier_t));
    memcpy(identifier_heap, &identifier, sizeof identifier);
    map_set(&scope->identifiers, name, identifier_heap);

    return identifier_heap;
}

scope_t *scope_resolve_identifier_scope(scope_t *scope, char *name)
{
    identifier_t *i = map_get(&scope->identifiers, name);

    if (scope->parent == NULL && i == NULL) 
        eval_error(true, "Undefined identifier '%s' in the current scope", name);
    else if (scope->parent != NULL && i == NULL)
        return scope_resolve_identifier_scope(scope->parent, name);

    return scope;
}

runtime_val_t *scope_assign_identifier(scope_t *scope, char *name, runtime_val_t *value)
{
    scope_t *foundscope = scope_resolve_identifier_scope(scope, name);
    identifier_t *identifier = scope_resolve_identifier(scope, name);

    if (scope->parent == NULL && identifier == NULL) 
        eval_error(true, "Undefined identifier '%s' in the current scope", name);

    if (identifier->is_const) 
        eval_error(true, "Cannot modify constant identifier '%s' in the current scope", name);

    identifier_t copy = *identifier;
    runtime_val_t *val_heap = xmalloc(sizeof (runtime_val_t));
    memcpy(val_heap, value, sizeof (runtime_val_t));

    map_delete(&foundscope->identifiers, name, true);
    
    return scope_declare_identifier(foundscope, name, val_heap, copy.is_const)->value;
}

identifier_t *scope_resolve_identifier(scope_t *scope, char *name)
{
    scope_t *found_scope = scope_resolve_identifier_scope(scope, name);
    return map_get(&found_scope->identifiers, name);
}

void scope_runtime_val_free(runtime_val_t *val)
{
    if (!val || val->literal != true)
        return;

    if (val->type == VAL_STRING)
    {
        zfree(val->strval, "String");
        val->strval = NULL;
    }
    else if (val->type == VAL_OBJECT)
    {
        for (size_t i = 0; i < val->properties.size; i++)
        {
            if (val->properties.array[i] != NULL)
            {
                if (val->properties.array[i]->value != NULL && val->properties.array[i]->value->value != NULL)
                {
                    scope_runtime_val_free(val->properties.array[i]->value->value);
                    val->properties.array[i]->value->value = NULL;
                }

                if (val->properties.array[i]->key != NULL && strlen(val->properties.array[i]->key) != 0)
                {
                    zfree(val->properties.array[i]->key, "Freeing key: %s", val->properties.array[i]->key);
                    val->properties.array[i]->key = NULL;
                }

                xfree(val->properties.array[i]);
                val->properties.array[i] = NULL;
            }
        }

        if (val->properties.array != NULL)
            zfree(val->properties.array, "Properties Array");
        
        val->properties.array = NULL;
    }
}

void scope_free(scope_t *scope)
{
    map_free(&scope->identifiers, true);
}
