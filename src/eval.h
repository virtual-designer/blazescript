#ifndef __EVAL_H__
#define __EVAL_H__

#include "ast.h"
#include "runtimevalues.h"

runtime_val_t eval(ast_stmt astnode);

#endif
