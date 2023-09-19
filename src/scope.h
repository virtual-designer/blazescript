/*
 * Created by rakinar2 on 8/25/23.
 */

#ifndef BLAZESCRIPT_SCOPE_H
#define BLAZESCRIPT_SCOPE_H

#include <stdatomic.h>
#include <stdint.h>
#include "datatype.h"
#include "valmap.h"

enum scope_mode
{
    SC_DEFAULT,
    SC_MODE_REUSE
};

struct scope
{
    struct scope *parent;
    valmap_t *valmap;
    val_t *null;
    bool allow_redecl;
    enum scope_mode mode;
    _Atomic uint64_t unique_id;
    _Atomic uint64_t prev_unique_id;
};

typedef struct scope scope_t;

struct scope *scope_init(struct scope *parent);
void scope_free(struct scope *scope);
enum valmap_set_status scope_assign_identifier(struct scope *scope, const char *name, val_t val);
enum valmap_set_status scope_declare_identifier(struct scope *scope, const char *name, val_t val, bool is_const);
val_t *scope_resolve_identifier(struct scope *scope, const char *name);
struct scope *scope_create_global();
val_t *blaze_null();

#endif /* BLAZESCRIPT_SCOPE_H */
