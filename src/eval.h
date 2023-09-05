/*
 * Created by rakinar2 on 8/24/23.
 */

#ifndef BLAZESCRIPT_EVAL_H
#define BLAZESCRIPT_EVAL_H

#include "ast.h"
#include "datatype.h"
#include "scope.h"

val_t eval(scope_t *scope, const ast_node_t *node);

extern char *eval_fn_error;

#endif /* BLAZESCRIPT_EVAL_H */
