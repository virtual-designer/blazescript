#ifndef __COMPILE_H__
#define __COMPILE_H__ 

#include "ast.h"
#include "bytecode.h"
#include "runtimevalues.h"

void compile(ast_stmt astnode, bytecode_t *bytecode);
void compile_force_push(ast_stmt astnode, bytecode_t *bytecode);
runtime_valtype_t dt_to_rtval_type(data_type_t type);
data_type_t ast_node_to_dt(ast_stmt node);
bool is_number_dt(data_type_t type);

#endif
