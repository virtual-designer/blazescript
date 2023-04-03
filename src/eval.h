#ifndef __EVAL_H__
#define __EVAL_H__

#include "ast.h"
#include "scope.h"
#include "runtimevalues.h"

runtime_val_t eval(ast_stmt astnode, scope_t *scope);
void eval_error(bool should_exit, const char *fmt, ...);

#endif
