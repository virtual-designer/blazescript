#ifndef __COMPILE_H__
#define __COMPILE_H__ 

#include "ast.h"
#include "bytecode.h"

void compile(ast_stmt astnode, bytecode_t *bytecode);

#endif
