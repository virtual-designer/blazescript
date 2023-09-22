/*
 * Created by rakinar2 on 9/21/23.
 */

#include "asm.h"
#include "alloca.h"
#include "arch.h"
#include "compile.h"
#include "utils.h"
#include <string.h>

asm_node_t asm_node_create(asm_node_type_t type)
{
    asm_node_t node = { 0 };
    node.type = type;
    return node;
}

asm_node_t asm_node_empty()
{
    return asm_node_create(ASM_EMPTY);
}

void asm_node_children_init(struct asm_node **nodes, size_t *size)
{
    *size = 0;
    *nodes = NULL;
}

void asm_node_children_push(struct asm_node **nodes, size_t *size, struct asm_node *node)
{
    *nodes = xrealloc(*nodes, sizeof (struct asm_node) * (++(*size)));
    (*nodes)[(*size) - 1] = *node;
}

void asm_node_free_inner(struct asm_node *node)
{
    switch (node->type)
    {
        case ASM_LABEL:
            free(node->label_name);
        break;

        case ASM_INSTRUCTION:
            for (size_t i = 0; i < node->inst_operand_count; i++)
            {
                if (node->inst_operands[i].type == ASM_OPERAND_IMMEDIATE && node->inst_operands[i].immediate_label != NULL)
                    free(node->inst_operands[i].immediate_label);
                if (node->inst_operands[i].type == ASM_OPERAND_IDENTIFIER)
                    free(node->inst_operands[i].identifier);
            }

            free(node->inst_operands);
        break;

        case ASM_ROOT:
            for (size_t i = 0; i < node->root_size; i++)
                asm_node_free_inner(&node->root_children[i]);

            free(node->root_children);
        break;

        case ASM_INSTRUCTION_ARRAY:
            for (size_t i = 0; i < node->array_size; i++)
                asm_node_free_inner(&node->array_children[i]);

            free(node->array_children);
        break;

        case ASM_EMPTY:
        break;

        case ASM_DIRECTIVE:
            free(node->directive_params);
            free(node->directive_name);
        break;

        default:
            fatal_error("unsupported assembly tree node found");
    }
}

void asm_node_free(struct asm_node *node)
{
    asm_node_free_inner(node);
    free(node);
}

asm_node_t asm_create_inst_array()
{
    return (asm_node_t) {
        .type = ASM_INSTRUCTION_ARRAY,
        .array_children = NULL,
        .array_size = 0
    };
}

asm_node_t asm_create_inst_bin_op(asm_node_inst_t inst, asm_inst_suffix_t suffix, asm_node_operand_t operand1, asm_node_operand_t operand2)
{
    asm_node_t node = {
        .type = ASM_INSTRUCTION,
        .inst = inst,
        .inst_suffix = suffix,
        .inst_operand_count = 2,
        .inst_operands = xcalloc(2, sizeof (operand1)),
    };

    node.inst_operands[0] = operand1;
    node.inst_operands[1] = operand2;

    return node;
}

void asm_push_inst_bin_op(asm_node_t *target, asm_node_inst_t inst, asm_inst_suffix_t suffix, asm_node_operand_t operand1, asm_node_operand_t operand2)
{
    asm_node_t instruction = asm_create_inst_bin_op(inst, suffix, operand1, operand2);
    asm_node_children_push(&target->array_children, &target->array_size, &instruction);
}

void asm_push_inst_mono_op(asm_node_t *target, asm_node_inst_t inst, asm_inst_suffix_t suffix, asm_node_operand_t operand1)
{
    asm_node_t instruction = asm_create_inst_mono_op(inst, suffix, operand1);
    asm_node_children_push(&target->array_children, &target->array_size, &instruction);
}

void asm_push_inst_no_op(asm_node_t *asm_node, asm_node_inst_t inst, asm_inst_suffix_t suffix)
{
    asm_node_children_push(&asm_node->root_children, &asm_node->root_size, & (asm_node_t) {
        .type = ASM_INSTRUCTION,
        .inst = inst,
        .inst_operand_count = 0,
        .inst_operands = NULL,
        .inst_suffix = suffix
    });
}

asm_node_t asm_create_inst_mono_op(asm_node_inst_t inst, asm_inst_suffix_t suffix, asm_node_operand_t operand1)
{
    asm_node_t node = {
        .type = ASM_INSTRUCTION,
        .inst = inst,
        .inst_suffix = suffix,
        .inst_operand_count = 1,
        .inst_operands = xcalloc(1, sizeof (operand1)),
    };

    node.inst_operands[0] = operand1;
    return node;
}

void asm_inst_array_push(asm_node_t node)
{
    asm_node_t array = asm_create_inst_array();
    array.array_children = xrealloc(array.array_children, (sizeof (asm_node_t)) * (++array.array_size));
    array.array_children[array.array_size - 1] = node;
}

void asm_data_push(struct compilation_context *ctx, char *lbl, char *directive, char *param)
{
    ctx->asm_data.data = xrealloc(ctx->asm_data.data, (sizeof (asm_data_lbl_t)) * (++ctx->asm_data.data_lbl_count));
    ctx->asm_data.data[ctx->asm_data.data_lbl_count - 1].label = lbl;
    ctx->asm_data.data[ctx->asm_data.data_lbl_count - 1].directive = directive;
    ctx->asm_data.data[ctx->asm_data.data_lbl_count - 1].param = param;
}

char *asm_inst_to_str(asm_node_inst_t inst, asm_inst_suffix_t suffix)
{
    const char *translate[] = {
        [ASM_INST_ADD] = "add",
        [ASM_INST_SUB] = "sub",
        [ASM_INST_MOV] = "mov",
        [ASM_INST_LEA] = "lea",
        [ASM_INST_CALL] = "call",
        [ASM_INST_PUSH] = "push",
        [ASM_INST_POP] = "pop",
        [ASM_INST_RET] = "ret",
        [ASM_INST_SYSCALL] = "syscall",
    };

    if (inst >= (sizeof (translate) / sizeof (translate[0])))
        fatal_error("invalid instruction code");

    size_t len = strlen(translate[inst]);
    char final[len + 2];
    strncpy(final, translate[inst], len);
    final[len] = suffix;
    final[len + 1] = 0;
    return strdup(final);
}

const char *asm_register_to_string(asm_node_register_t regcode)
{
    const char *translate[] = {
        [ASM_INTEL_RAX] = "rax",
        [ASM_INTEL_RCX] = "rcx",
        [ASM_INTEL_RBX] = "rbx",
        [ASM_INTEL_RDX] = "rdx",
        [ASM_INTEL_RDI] = "rdi",
        [ASM_INTEL_RSI] = "rsi",
        [ASM_INTEL_RSP] = "rsp",
        [ASM_INTEL_RBP] = "rbp",
        [ASM_INTEL_R8] = "r8",
        [ASM_INTEL_R9] = "r9",
        [ASM_ARM64_X0] = "x0",
        [ASM_ARM64_X1] = "x1",
        [ASM_ARM64_X2] = "x2",
        [ASM_ARM64_X3] = "x3",
        [ASM_ARM_R0] = "r0",
        [ASM_ARM_R1] = "r1",
        [ASM_ARM_R2] = "r2",
        [ASM_ARM_R3] = "r3",
    };

    if (regcode >= (sizeof (translate) / sizeof (translate[0])))
        fatal_error("invalid register code");

    return translate[regcode];
}

void asm_operand_print(FILE *file, enum blazec_arch arch, asm_node_operand_t *operand)
{
    switch (operand->type)
    {
        case ASM_OPERAND_IMMEDIATE:
            if (operand->immediate_label != NULL)
                fprintf(file, "%s%s", arch == ARCH_X86_64 || arch == ARCH_I386 ? "$" : "", operand->immediate_label);
            else
                fprintf(file, "%s%li", arch == ARCH_X86_64 || arch == ARCH_I386 ? "$" : "", operand->immediate);

            break;

        case ASM_OPERAND_IDENTIFIER:
            fprintf(file, "%s", operand->identifier);
            break;

        case ASM_OPERAND_REGISTER:
            fprintf(file, "%s%s",
                    arch == ARCH_X86_64 || arch == ARCH_I386 ? "%" : "", asm_register_to_string(operand->cpu_register));
            break;
        case ASM_OPERAND_ADDR_REG:
            fprintf(file, "(%s%s)",
                    arch == ARCH_X86_64 || arch == ARCH_I386 ? "%" : "", asm_register_to_string(operand->addr_register));
            break;
        case ASM_OPERAND_ADDR_OFFSET_REG:
            if (operand->offset == 0)
                fprintf(file, "(%s%s)",
                        arch == ARCH_X86_64 || arch == ARCH_I386 ? "%" : "", asm_register_to_string(operand->offset_register));
            else
                fprintf(file, "%li(%s%s)",
                        operand->offset,
                        arch == ARCH_X86_64 || arch == ARCH_I386 ? "%" : "", asm_register_to_string(operand->offset_register));
            break;

        default:
            fatal_error("unsupported operand type");
    }
}

void asm_print(FILE *file, enum blazec_arch arch, asm_node_t *node)
{
    if (node->type == ASM_ROOT && node->root_size > 0)
        fprintf(file, ".text\n");

    switch (node->type)
    {
        case ASM_ROOT:
            for (size_t i = 0; i < node->root_size; i++)
                asm_print(file, arch, &node->root_children[i]);

#if defined(__linux__) && defined(__ELF__)
            fprintf(file, "\n.section .note.GNU-stack,\"\",%%progbits\n\n");
#endif
        break;

        case ASM_EMPTY:
        break;

        case ASM_INSTRUCTION_ARRAY:
            for (size_t i = 0; i < node->array_size; i++)
                asm_print(file, arch, &node->array_children[i]);
        break;

        case ASM_LABEL:
            fprintf(file, "%s:\n", node->label_name);
        break;

        case ASM_DIRECTIVE:
            fprintf(file, ".%s %s\n", node->directive_name, node->directive_params);
        break;

        case ASM_INSTRUCTION:
            {
                char *inst_str = asm_inst_to_str(node->inst, node->inst_suffix);
                fprintf(file, "    %s    ", inst_str);
                free(inst_str);

                for (size_t i = 0; i < node->inst_operand_count; i++)
                {
                    asm_operand_print(file, arch, &node->inst_operands[i]);

                    if (i != node->inst_operand_count - 1)
                        fprintf(file, ", ");
                }

                fprintf(file, "\n");
            }
        break;

        default:
            fatal_error("invalid node");
    }
}

void asm_print_header(FILE *file, asm_data_t *data)
{
    if (data->data_lbl_count != 0)
        fprintf(file, ".data\n");

    for (size_t i = 0; i < data->data_lbl_count; i++)
    {
        fprintf(file, "%s:\n", data->data[i].label);
        fprintf(file, "    .%s %s\n", data->data[i].directive, data->data[i].param);
    }
}

void asm_data_free(asm_data_t *data)
{
    for (size_t i = 0; i < data->data_lbl_count; i++)
    {
        free(data->data[i].label);
        free(data->data[i].param);
        free(data->data[i].directive);
    }

    free(data->data);
}