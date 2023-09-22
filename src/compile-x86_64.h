/*
 * Created by rakinar2 on 9/19/23.
 */

#ifndef BLAZESCRIPT_COMPILE_X86_64_H
#define BLAZESCRIPT_COMPILE_X86_64_H

#include "ast.h"
#include "compile.h"

asm_node_t x86_64_compile(struct compilation_context *context, ast_node_t *node);

#endif /* BLAZESCRIPT_COMPILE_X86_64_H */
