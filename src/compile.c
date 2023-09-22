/*
 * Created by rakinar2 on 9/19/23.
 */

#define _GNU_SOURCE

#include "compile.h"
#include "compile-x86_64.h"
#include "datatype.h"
#include "utils.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct compilation_context compilation_context_create()
{
    return (struct compilation_context) {
        .asm_data = {
            .data = NULL,
            .data_lbl_count = 0
        }
    };
}

void compilation_context_destroy(struct compilation_context *context)
{
    asm_data_free(&context->asm_data);
}

asm_node_t compile(struct compilation_context *context, ast_node_t *node)
{
    if (context->arch == ARCH_X86_64)
        return x86_64_compile(context, node);
    else
        fatal_error("unsupported architecture: %s", arch_to_str(context->arch));
}