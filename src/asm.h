/*
 * Created by rakinar2 on 9/21/23.
 */

#ifndef BLAZESCRIPT_ASM_H
#define BLAZESCRIPT_ASM_H

#include "arch.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define IMM64(imm) ((asm_node_operand_t) { .type = ASM_OPERAND_IMMEDIATE, .immediate = imm })
#define IMM_LBL(lbl) ((asm_node_operand_t) { .type = ASM_OPERAND_IMMEDIATE, .immediate_label = strdup(lbl) })
#define REGISTER(reg) ((asm_node_operand_t) { .type = ASM_OPERAND_REGISTER, .cpu_register = reg })
#define REGISTER_OFFSET(value, reg) ((asm_node_operand_t) { .type = ASM_OPERAND_ADDR_OFFSET_REG, .offset_register = (reg), .offset = (value) })
#define REGISTER_X64(reg) ((asm_node_operand_t) { .type = ASM_OPERAND_REGISTER, .cpu_register = ASM_INTEL_##reg })
#define IDENTIFIER(id) ((asm_node_operand_t) { .type = ASM_OPERAND_IDENTIFIER, .identifier = strdup(id) })

struct compilation_context;

typedef struct {
    char *label;
    char *directive;
    char *param;
} asm_data_lbl_t;

typedef struct {
    asm_data_lbl_t *data;
    size_t data_lbl_count;
} asm_data_t;

typedef enum
{
    ASM_INSTRUCTION,
    ASM_INSTRUCTION_ARRAY,
    ASM_LABEL,
    ASM_ROOT,
    ASM_EMPTY,
    ASM_DIRECTIVE
} asm_node_type_t;

typedef enum
{
    ASM_INST_MOV,
    ASM_INST_LEA,
    ASM_INST_RET,
    ASM_INST_PUSH,
    ASM_INST_POP,
    ASM_INST_SYSCALL,
    ASM_INST_CALL,
    ASM_INST_ADD,
    ASM_INST_SUB
} asm_node_inst_t;

typedef enum
{
    ASM_SUF_BYTE = 'b',
    ASM_SUF_WORD = 'w',
    ASM_SUF_DWORD = 'l',
    ASM_SUF_QWORD = 'q',
    ASM_SUF_NONE = 0
} asm_inst_suffix_t;

typedef enum
{
    ASM_OPERAND_IMMEDIATE,
    ASM_OPERAND_REGISTER,
    ASM_OPERAND_ADDR_REG,
    ASM_OPERAND_ADDR_OFFSET_REG,
    ASM_OPERAND_IDENTIFIER
} asm_node_operand_type_t;

typedef enum
{
    /* Registers for Intel i386 and x86_64 CPUs. */
    ASM_INTEL_RAX,
    ASM_INTEL_RCX,
    ASM_INTEL_RBX,
    ASM_INTEL_RDX,
    ASM_INTEL_RDI,
    ASM_INTEL_RSI,
    ASM_INTEL_RSP,
    ASM_INTEL_RBP,
    ASM_INTEL_R8,
    ASM_INTEL_R9,

    /* Registers for ARM CPUs. */
    ASM_ARM_R0,
    ASM_ARM_R1,
    ASM_ARM_R2,
    ASM_ARM_R3,

    /* Registers for AArch64 (arm64) CPUs. */
    ASM_ARM64_X0,
    ASM_ARM64_X1,
    ASM_ARM64_X2,
    ASM_ARM64_X3,
} asm_node_register_t;

typedef struct
{
    asm_node_operand_type_t type;

    union
    {
        struct
        {
            int64_t immediate;
            char *immediate_label;
        };

        char *identifier;

        asm_node_register_t addr_register;
        asm_node_register_t cpu_register;

        struct
        {
            asm_node_register_t offset_register;
            int64_t offset;
        };
    };
} asm_node_operand_t;

typedef struct asm_node
{
    asm_node_type_t type;

    union
    {
        struct
        {
            struct asm_node *root_children;
            size_t root_size;
        };

        struct
        {
            struct asm_node *array_children;
            size_t array_size;
        };

        struct
        {
            char *label_name;
        };

        struct
        {
            asm_node_inst_t inst;
            asm_inst_suffix_t inst_suffix;
            asm_node_operand_t *inst_operands;
            size_t inst_operand_count;
        };

        struct
        {
            char *directive_name;
            char *directive_params;
        };
    };
} asm_node_t;

asm_node_t asm_node_create(asm_node_type_t type);
void asm_node_children_init(struct asm_node **nodes, size_t *size);
void asm_node_children_push(struct asm_node **nodes, size_t *size, struct asm_node *node);
void asm_node_free_inner(struct asm_node *node);
void asm_node_free(struct asm_node *node);
asm_node_t asm_node_empty();
void asm_inst_array_push(asm_node_t node);
asm_node_t asm_create_inst_array();
asm_node_t asm_create_inst_bin_op(asm_node_inst_t inst, asm_inst_suffix_t suffix, asm_node_operand_t operand1, asm_node_operand_t operand2);
void asm_print(FILE *file, enum blazec_arch arch, asm_node_t *node);
asm_node_t asm_create_inst_mono_op(asm_node_inst_t inst, asm_inst_suffix_t suffix, asm_node_operand_t operand1);
void asm_data_push(struct compilation_context *ctx, char *lbl, char *directive, char *param);
void asm_data_free(asm_data_t *data);
void asm_print_header(FILE *file, asm_data_t *data);
void asm_push_inst_bin_op(asm_node_t *target, asm_node_inst_t inst, asm_inst_suffix_t suffix, asm_node_operand_t operand1, asm_node_operand_t operand2);
void asm_push_inst_mono_op(asm_node_t *target, asm_node_inst_t inst, asm_inst_suffix_t suffix, asm_node_operand_t operand1);
void asm_push_inst_no_op(asm_node_t *asm_node, asm_node_inst_t inst, asm_inst_suffix_t suffix);

#endif /* BLAZESCRIPT_ASM_H */
