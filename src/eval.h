/*
 * Created by rakinar2 on 8/24/23.
 */

#ifndef BLAZESCRIPT_EVAL_H
#define BLAZESCRIPT_EVAL_H

#include "ast.h"
#include "datatype.h"
#include "scope.h"

void val_free(val_t *val);
val_t *eval(scope_t *scope, const ast_node_t *node);
void print_val(val_t *val);
const char *val_type_to_str(val_type_t type);
val_t *val_create(val_type_t type);
void val_free_global();
void val_free_force(val_t *val);
void print_val_internal(val_t *val, bool quote_strings);
val_t *val_copy_deep(val_t *orig);

extern char *eval_fn_error;

#endif /* BLAZESCRIPT_EVAL_H */
