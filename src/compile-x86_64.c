/*
 * Created by rakinar2 on 9/19/23.
 */

#define _GNU_SOURCE

#include "compile-x86_64.h"
#include "compile.h"
#include "log.h"
#include "utils.h"
#include "include/lib.h"
#include <assert.h>
#include <string.h>
#include <string.h>

#define RSP REGISTER_X64(RSP)
#define RBP REGISTER_X64(RBP)
#define RAX REGISTER_X64(RAX)
#define RDI REGISTER_X64(RDI)
#define RSI REGISTER_X64(RSI)
#define RDX REGISTER_X64(RDX)
#define RCX REGISTER_X64(RCX)
#define R8 REGISTER_X64(R8)
#define R9 REGISTER_X64(R9)

static struct builtin_function_def const builtin_function_defs[] = {
    { "println", "libblaze_fn_println", true }
};
static size_t builtin_function_def_count = sizeof (builtin_function_defs) / sizeof (builtin_function_defs[0]);
static uint64_t lbl_count = 0;

static asm_node_t x86_64_compile_call_expr(struct compilation_context *context, ast_node_t *node);
static asm_node_t x86_64_compile_expr(struct compilation_context *context, ast_node_t *node)
{
    asm_node_t asm_node = asm_create_inst_array();

    switch (node->type)
    {
        case NODE_STRING:
        {
            char *label = NULL;
            asprintf(&label, ".DL%lu", lbl_count++);
            char *string = strdup("\"");
            size_t len = 1;

            for (size_t i = 0; i < strlen(node->string->strval); i++)
            {
                const char c = node->string->strval[i];

                if (c == '"')
                {
                    len += 2;
                    string = xrealloc(string, len);
                    string[len - 2] = '\\';
                }
                else
                    string = xrealloc(string, ++len);

                string[len - 1] = c;
            }

            len += 2;
            string = xrealloc(string, len);
            string[len - 2] = '"';
            string[len - 1] = 0;

            asm_data_push(context, label, strdup("string"), string);
            asm_push_inst_bin_op(&asm_node, ASM_INST_MOV, ASM_SUF_QWORD,
                                 IMM_LBL(label), REGISTER_X64(RDI));
            asm_push_inst_mono_op(&asm_node, ASM_INST_CALL, ASM_SUF_QWORD,
                                  IDENTIFIER("libblaze_val_create_strval"));
        }
        break;

        default:
            fatal_error("complex expressions aren't supported yet");
    }

    return asm_node;
}

static void x86_64_compile_call_expr_args_load(struct compilation_context *context, size_t argc, asm_node_t *asm_node, size_t stack_argc_ptr, bool variadic)
{
    for (size_t i = 0; i < argc; i++)
    {
        if (i <= 5)
        {
            asm_node_operand_t operand;

            if (i == 0)
                operand = RDI;
            else if (i == 1)
                operand = RSI;
            else if (i == 2)
                operand = RDX;
            else if (i == 3)
                operand = RCX;
            else if (i == 4)
                operand = R8;
            else if (i == 5)
                operand = R9;

            asm_push_inst_bin_op(asm_node, ASM_INST_MOV, ASM_SUF_QWORD, REGISTER_OFFSET(-8 * (i + 1), ASM_INTEL_RBP), operand);
        }
    }
}

static void x86_64_compile_call_expr_args(struct compilation_context *context, size_t argc, ast_node_t *args, asm_node_t *asm_node, size_t *stack_argc_ptr, bool variadic)
{
    if (variadic)
    {
        asm_push_inst_bin_op(asm_node, ASM_INST_MOV, ASM_SUF_QWORD,
                             IMM64(argc), RDI);
        asm_push_inst_mono_op(asm_node, ASM_INST_PUSH, ASM_SUF_QWORD, RDI);
    }

    size_t stack_argc = 0;

    for (size_t i = 0, c = variadic ? 1 : 0; i < argc; i++, c++)
    {
        ast_node_t arg = args[i];
        asm_node_t instructions = x86_64_compile_expr(context, &arg);
        asm_node_children_push(&asm_node->array_children, &asm_node->array_size, &instructions);
        asm_push_inst_mono_op(asm_node, ASM_INST_PUSH, ASM_SUF_QWORD, RAX);

        if (c > 5)
            stack_argc++;
    }

    *stack_argc_ptr = stack_argc;
}

static void x86_64_compile_call_expr_free_values(asm_node_t *asm_node, size_t argc, bool variadic)
{
    for (size_t i = variadic ? 1 : 0; i < argc; i++)
    {
        asm_push_inst_mono_op(asm_node, ASM_INST_POP, ASM_SUF_QWORD, RDI);
        asm_push_inst_mono_op(asm_node, ASM_INST_CALL, ASM_SUF_QWORD, IDENTIFIER("libblaze_val_free"));
    }

    if (variadic)
        asm_push_inst_bin_op(asm_node, ASM_INST_ADD, ASM_SUF_QWORD, IMM64(8), RSP);
}

static asm_node_t x86_64_compile_call_expr(struct compilation_context *context, ast_node_t *node)
{
    assert(node->type == NODE_EXPR_CALL);
    assert(node->fn_call->argc < 3);

    size_t argc = node->fn_call->argc;
    size_t stack_args = 0;
    asm_node_t asm_node = asm_create_inst_array();

    bool found = false;

    for (size_t i = 0; i < builtin_function_def_count; i++)
    {
        if (strcmp(builtin_function_defs[i].name, node->fn_call->identifier->symbol) == 0)
        {
            found = true;
            x86_64_compile_call_expr_args(context, argc, node->fn_call->args, &asm_node, &stack_args, builtin_function_defs[i].variadic);
            x86_64_compile_call_expr_args_load(context, argc + 1, &asm_node, stack_args, builtin_function_defs[i].variadic);
            asm_push_inst_mono_op(&asm_node, ASM_INST_CALL, ASM_SUF_QWORD, IDENTIFIER(builtin_function_defs->actual_name));
             x86_64_compile_call_expr_free_values(&asm_node, argc + 1, builtin_function_defs[i].variadic);
            break;
        }
    }

    if (!found)
        fatal_error("call to undefined function '%s()'", node->fn_call->identifier->symbol);

    if (stack_args > 0)
        asm_push_inst_bin_op(&asm_node, ASM_INST_ADD, ASM_SUF_QWORD, IMM64(8 * stack_args), RSP);

    return asm_node;
}

static void x86_64_compile_root_start(asm_node_t *asm_node)
{
    asm_node_children_push(&asm_node->root_children, &asm_node->root_size, & (asm_node_t) {
        .type = ASM_DIRECTIVE,
        .directive_name = strdup("globl"),
        .directive_params = strdup("_start")
    });
    asm_node_children_push(&asm_node->root_children, &asm_node->root_size, & (asm_node_t) {
        .type = ASM_LABEL,
        .label_name = strdup("_start")
    });

    asm_push_inst_mono_op(asm_node, ASM_INST_CALL, ASM_SUF_QWORD, IDENTIFIER("main"));
    asm_push_inst_bin_op(asm_node, ASM_INST_MOV, ASM_SUF_QWORD, IMM64(0), REGISTER_X64(RDI));
    asm_push_inst_mono_op(asm_node, ASM_INST_CALL, ASM_SUF_QWORD, IDENTIFIER("exit"));
}

static void x86_64_setup_frame(asm_node_t *asm_node)
{
    asm_push_inst_mono_op(asm_node, ASM_INST_PUSH, ASM_SUF_QWORD, RBP);
    asm_push_inst_bin_op(asm_node, ASM_INST_MOV, ASM_SUF_QWORD, RSP, RBP);
}

static void x86_64_restore_frame(asm_node_t *asm_node)
{
    asm_push_inst_mono_op(asm_node, ASM_INST_POP, ASM_SUF_QWORD, RBP);
}

static asm_node_t x86_64_compile_root(struct compilation_context *context, ast_node_t *node)
{
    asm_node_t asm_node = asm_node_create(ASM_ROOT);
    asm_node_children_init(&asm_node.root_children, &asm_node.root_size);

    x86_64_compile_root_start(&asm_node);

    asm_node_children_push(&asm_node.root_children, &asm_node.root_size, & (asm_node_t) {
        .type = ASM_LABEL,
        .label_name = strdup("main")
    });

    x86_64_setup_frame(&asm_node);

    for (size_t i = 0; i < node->root->size; i++)
    {
        asm_node_t ret = x86_64_compile(context, &node->root->nodes[i]);

        if (ret.type != ASM_EMPTY)
            asm_node_children_push(&asm_node.root_children, &asm_node.root_size, &ret);
    }

    x86_64_restore_frame(&asm_node);
    asm_push_inst_no_op(&asm_node, ASM_INST_RET, ASM_SUF_QWORD);
    return asm_node;
}

asm_node_t x86_64_compile(struct compilation_context *context, ast_node_t *node)
{
    switch (node->type)
    {
        case NODE_ROOT:
            return x86_64_compile_root(context, node);

        case NODE_EXPR_CALL:
            return x86_64_compile_call_expr(context, node);

        default:
            fatal_error("invalid or unsupported AST type");
    }

    return asm_node_empty();
}