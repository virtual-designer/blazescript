/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_AST_H
#define BLAZESCRIPT_AST_H

#include <stddef.h>

typedef enum ast_node_type {
    NODE_NUM_LIT,
    NODE_BINARY_EXPR
} ast_type_t;

typedef enum ast_bin_operator {
    OP_PLUS = '+',
    OP_MINUS = '-',
    OP_DIVIDE = '/',
    OP_TIMES = '*',
    OP_MODULUS = '%'
} ast_bin_operator_t;

typedef struct ast_sub_int_lit {
    long long int intval;
} ast_intlit_t;

typedef struct ast_sub_binexpr {
    struct ast_node *left;
    struct ast_node *right;
    ast_bin_operator_t operator;
} ast_binexpr_t;

typedef struct ast_node
{
    ast_type_t type;

    union {
        ast_intlit_t integer;
        ast_binexpr_t binexpr;
    };
} ast_node_t;

typedef struct ast_root {
    size_t size;
    ast_node_t *nodes;
} ast_root_t;

#endif /* BLAZESCRIPT_AST_H */
