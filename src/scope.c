/*
 * Created by rakinar2 on 8/25/23.
 */

#include <stdlib.h>

#include "alloca.h"
#include "include/lib.h"
#include "scope.h"
#include "valmap.h"

static val_t true_val = { .type = VAL_BOOLEAN, .boolval = true, .nofree = true },
             false_val = { .type = VAL_BOOLEAN, .boolval = false, .nofree = true },
             null_val = { .type = VAL_NULL, .nofree = true };

struct scope *scope_init(struct scope *parent)
{
    struct scope *scope = xcalloc(1, sizeof(struct scope));
    scope->valmap = valmap_init_default();
    scope->parent = parent;
    scope->allow_redecl = false;

    scope->null = &null_val;
    return scope;
}

val_t *blaze_null()
{
    return &null_val;
}

struct scope *scope_create_global()
{
    struct scope *scope = scope_init(NULL);

    scope_declare_identifier(scope, "true", true_val, true);
    scope_declare_identifier(scope, "false", false_val, true);
    scope_declare_identifier(scope, "null", null_val, true);

    for (size_t i = 0; i < (sizeof builtin_functions) / (sizeof builtin_functions[0]); i++)
    {
        val_t *fn_val = val_create_heap(VAL_FUNCTION);
        fn_val->nofree = true;
        fn_val->fnval->type = FN_BUILT_IN;
        fn_val->fnval->built_in_callback = builtin_functions[i].callback;
        scope_declare_identifier(scope, builtin_functions[i].name, *fn_val, true);
    }

    return scope;
}

void scope_free(struct scope *scope)
{
    if (scope == NULL)
        return;

    if (scope->parent == NULL)
        valmap_free_builtin_fns(scope->valmap);

    valmap_free(scope->valmap, true);
    free(scope);
}

enum valmap_set_status scope_assign_identifier(struct scope *scope, const char *name, val_t val)
{
    enum valmap_set_status status = valmap_set_no_create(scope->valmap, name, val, false, false);

    if (status == VAL_SET_NOT_FOUND && scope->parent != NULL)
    {
        return scope_assign_identifier(scope->parent, name, val);
    }

    return status;
}

enum valmap_set_status scope_declare_identifier(struct scope *scope, const char *name, val_t val, bool is_const)
{
    return scope->allow_redecl ?
       valmap_set_default(scope->valmap, name, val, is_const, true)
       : valmap_set_no_overwrite(scope->valmap, name, val, is_const, true);
}

val_t *scope_resolve_identifier(struct scope *scope, const char *name)
{
    val_t *val = valmap_get(scope->valmap, name);

    if (val == NULL && scope->parent != NULL)
        return scope_resolve_identifier(scope->parent, name);

    return val;
}