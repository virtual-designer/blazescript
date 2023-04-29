#ifndef __COMPILE_H__
#define __COMPILE_H__ 

#include "ast.h"
#include "bytecode.h"
#include "runtimevalues.h"

void compile(ast_stmt astnode, bytecode_t *bytecode);
inline runtime_valtype_t dt_to_rtval_type(data_type_t type);
inline data_type_t ast_node_to_dt(ast_stmt node);

#endif
