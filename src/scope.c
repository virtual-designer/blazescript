/*
 * Created by rakinar2 on 8/25/23.
 */

#include <stdlib.h>

#include "alloca.h"
#include "eval.h"
#include "scope.h"
#include "valmap.h"

struct scope *scope_init(struct scope *parent)
{
    struct scope *scope = xcalloc(1, sizeof (struct scope));
    scope->valmap = valmap_init_default();
    scope->parent = parent;

    if (parent == NULL)
    {
        scope->null = val_create(VAL_NULL);
        scope->null->nofree = true;
    }
    else
    {
        struct scope *tmp_scope = scope;

        while (tmp_scope->parent != NULL)
        {
            tmp_scope = tmp_scope->parent;
        }

        scope->null = tmp_scope->null;
    }

    return scope;
}

void scope_free(struct scope *scope)
{
    valmap_free(scope->valmap, true);

    if (scope->parent == NULL)
        val_free_force(scope->null);

    free(scope);
}

bool scope_assign_identifier(struct scope *scope, const char *name, val_t *val)
{
    return valmap_set_no_create(scope->valmap, name, val, true);
}

bool scope_declare_identifier(struct scope *scope, const char *name, val_t *val)
{
    return valmap_set_no_overwrite(scope->valmap, name, val, true);
}

val_t *scope_resolve_identifier(struct scope *scope, const char *name)
{
    return valmap_get(scope->valmap, name);
}