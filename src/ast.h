/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_AST_H
#define BLAZESCRIPT_AST_H

#include <stddef.h>

typedef enum ast_node_type {
    NODE_ROOT,
    NODE_INT_LIT,
    NODE_BINARY_EXPR,
    NODE_IDENTIFIER,
    NODE_STRING
} ast_type_t;

typedef enum ast_bin_operator {
    OP_PLUS = '+',
    OP_MINUS = '-',
    OP_DIVIDE = '/',
    OP_TIMES = '*',
    OP_MODULUS = '%'
} ast_bin_operator_t;

typedef struct ast_int_lit {
    long long int intval;
} ast_intlit_t;

typedef struct ast_str_lit {
    char *strval;
} ast_string_t;

typedef struct ast_binexpr {
    struct ast_node *left;
    struct ast_node *right;
    ast_bin_operator_t operator;
} ast_binexpr_t;

typedef struct ast_identifier {
    char *symbol;
} ast_identifier_t;

typedef struct ast_root {
    size_t size;
    struct ast_node *nodes;
} ast_root_t;

typedef struct ast_node
{
    ast_type_t type;

    union {
        ast_intlit_t *integer;
        ast_binexpr_t *binexpr;
        ast_identifier_t *identifier;
        ast_root_t *root;
        ast_string_t *string;
    };
} ast_node_t;

#endif /* BLAZESCRIPT_AST_H */
