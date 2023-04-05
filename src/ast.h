#ifndef __AST_H__
#define __AST_H__

#include <sys/types.h>
#include <stdbool.h>

typedef enum {
    NODE_PROGRAM,
    NODE_EXPR_BINARY,
    NODE_NUMERIC_LITERAL,
    NODE_IDENTIFIER,
    NODE_EXPR_CALL,
    NODE_EXPR_UNARY,
    NODE_DECL_FUNCTION,
    NODE_DECL_VAR,
    NODE_EXPR_ASSIGNMENT
} ast_nodetype_t;

typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIVIDE,
    OP_MOD
} ast_operator_t;

typedef struct ast_stmt {
    ast_nodetype_t type;                            /* Type of this statement. */

    union {
        /* if (type == NODE_PROGRAM) */  
        struct {
            struct ast_stmt *body;                  /* Array of statements. */
            size_t size;                            /* Size of the array. */
        };
        /* endif */                

        /* if (type == NODE_EXPR_BINARY) */
        struct {
            struct ast_stmt *left;                  /* Pointer to the statement at left. */
            struct ast_stmt *right;                 /* Pointer to the statement at right. */
            ast_operator_t operator;                /* The operator type. */
        };
        /* endif */                
        
        /* if (type == NODE_IDENTIFIER) */
        char *symbol;                               /* The identifier symbol. */
        /* endif */                
        
        /* if (type == NODE_NUMERIC_LITERAL) */
        long double value;                          /* The actual value of the numeric literal. */
        /* endif */                
        
        /* if (type == NODE_DECL_VAR) */
        struct {
            char *identifier;
            bool is_const;
            struct ast_stmt *varval;
            bool has_val;
        };                          
        /* endif */                
        
        /* if (type == NODE_EXPR_ASSIGNMENT) */
        struct {
            struct ast_stmt *assignee;
            struct ast_stmt *assignment_value;
        };                          
        /* endif */                
    };
} ast_stmt;

#endif
