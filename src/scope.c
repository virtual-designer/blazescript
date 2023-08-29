/*
 * Created by rakinar2 on 8/25/23.
 */

#include <stdlib.h>

#include "alloca.h"
#include "eval.h"
#include "include/lib.h"
#include "scope.h"
#include "valmap.h"

static val_t *true_val = NULL,
             *false_val = NULL;

static struct scope **allocated_scopes = NULL;
static size_t scope_count = 0;

struct scope *scope_init(struct scope *parent)
{
    struct scope *scope = blaze_calloc(1, sizeof(struct scope));
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

    allocated_scopes = blaze_realloc(
        allocated_scopes, (sizeof(struct scope *)) * (++scope_count));
    allocated_scopes[scope_count - 1] = scope;

    return scope;
}

struct scope *scope_create_global()
{
    struct scope *scope = scope_init(NULL);

    true_val = val_create(VAL_BOOLEAN);
    false_val = val_create(VAL_BOOLEAN);
    true_val->boolval->value = true;
    false_val->boolval->value = false;
    true_val->nofree = true;
    false_val->nofree = true;

    scope_declare_identifier(scope, "true", true_val, true);
    scope_declare_identifier(scope, "false", false_val, true);
    scope_declare_identifier(scope, "null", scope->null, true);

    for (size_t i = 0; i < (sizeof builtin_functions) / (sizeof builtin_functions[0]); i++)
    {
        val_t *fn_val = val_create(VAL_FUNCTION);
        fn_val->fnval->type = FN_BUILT_IN;
        fn_val->fnval->built_in_callback = builtin_functions[i].callback;
        scope_declare_identifier(scope, builtin_functions[i].name, fn_val, true);
    }

    return scope;
}

void scope_free(struct scope *scope)
{
    if (scope == NULL)
        return;

    bool found = false;

    for (size_t i = 0; i < scope_count; i++)
    {
        if (allocated_scopes[i] == scope)
        {
            found = true;
            allocated_scopes[i] = NULL;
        }
    }

    if (!found)
    {
        return;
    }

    if (scope->parent == NULL)
        valmap_free_builtin_fns(scope->valmap);

    valmap_free(scope->valmap, true);

    if (scope->parent == NULL)
    {
        val_free_force(scope->null);
        val_free_force(true_val);
        val_free_force(false_val);
    }

    blaze_free(scope);
}

enum valmap_set_status scope_assign_identifier(struct scope *scope, const char *name, val_t *val)
{
    enum valmap_set_status status = valmap_set_no_create(scope->valmap, name, val, false, true);

    if (status == VAL_SET_NOT_FOUND && scope->parent != NULL)
    {
        return scope_assign_identifier(scope->parent, name, val);
    }

    return status;
}

enum valmap_set_status scope_declare_identifier(struct scope *scope, const char *name, val_t *val, bool is_const)
{
    return valmap_set_no_overwrite(scope->valmap, name, val, is_const, true);
}

val_t *scope_resolve_identifier(struct scope *scope, const char *name)
{
    val_t *val = valmap_get(scope->valmap, name);

    if (val == NULL && scope->parent != NULL)
        return scope_resolve_identifier(scope->parent, name);

    return val;
}

void scope_destroy_all()
{
    for (size_t i = 0; i < scope_count; i++)
    {
        scope_free(allocated_scopes[i]);
    }

    blaze_free(allocated_scopes);
    allocated_scopes = NULL;
    scope_count = 0;
}