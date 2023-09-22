/*
 * Created by rakinar2 on 9/19/23.
 */

#ifndef BLAZESCRIPT_COMPILE_H
#define BLAZESCRIPT_COMPILE_H

#include "ast.h"
#include "arch.h"
#include "asm.h"

#define COMPILE(fn_name, arch) arch##_##fn_name

struct compilation_context
{
    asm_data_t asm_data;
    enum blazec_arch arch;
};

struct builtin_function_def
{
    char *name;
    char *actual_name;
    bool variadic;
};

typedef struct compilation_context compilation_ctx_t;

struct compilation_context compilation_context_create();
void compilation_context_destroy(struct compilation_context *context);
asm_node_t compile(struct compilation_context *context, ast_node_t *node);

#endif /* BLAZESCRIPT_COMPILE_H */
