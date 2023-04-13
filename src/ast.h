#ifndef __AST_H__
#define __AST_H__

#include <sys/types.h>
#include <stdbool.h>
#include "vector.h"

typedef enum {
    NODE_PROGRAM,
    NODE_EXPR_BINARY,
    NODE_NUMERIC_LITERAL,
    NODE_IDENTIFIER,
    NODE_EXPR_CALL,
    NODE_EXPR_UNARY,
    NODE_DECL_FUNCTION,
    NODE_DECL_VAR,
    NODE_EXPR_ASSIGNMENT,
    NODE_OBJECT_LITERAL,
    NODE_PROPERTY_LITERAL,
    NODE_EXPR_MEMBER_ACCESS,
    NODE_STRING
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
    size_t line;

    union {
        /* if (type == NODE_PROGRAM || NODE_DECL_FUNCTION) */  
        struct {
            struct ast_stmt *body;                  /* Array of statements. */
            size_t size;                            /* Size of the array. */
            char *fn_name;                          /* Function name. */
            vector_t argnames;
        };
        /* endif */                

        /* if (type == NODE_EXPR_BINARY) || (type == NODE_EXPR_UNARY) */
        struct {
            struct ast_stmt *left;                  /* Pointer to the statement at left. */
            struct ast_stmt *right;                 /* Pointer to the statement at right. */
            ast_operator_t operator;                /* The operator type. */
        };
        /* endif */                
        
        /* if (type == NODE_IDENTIFIER) */
        char *symbol;                               /* The identifier symbol. */
        /* endif */                
        
        /* if (type == NODE_STRING) */
        char *strval;                               /* The string value. */
        /* endif */                
        
        /* if (type == NODE_NUMERIC_LITERAL) */
        struct {
            long double value;                          /* The actual value of the numeric literal. */
            bool is_float;
        };
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
        
        /* if (type == NODE_EXPR_CALL) */
        struct {
            struct ast_stmt *callee;
            vector_t args; /* Vector of struct ast_stmt */
        };                          
        /* endif */                
        
        /* if (type == NODE_PROPERTY_LITERAL) */
        struct {
            char *key;
            struct ast_stmt *propval;       
        };                          
        /* endif */                
        
        /* if (type == NODE_OBJECT_LITERAL) */
        struct {
            vector_t properties; /* Vector of struct ast_stmt */
        };                          
        /* endif */         

        /* if (type == NODE_EXPR_MEMBER_ACCESS) */
        struct {
            struct ast_stmt *object;
            struct ast_stmt *prop;
            bool computed;
        };                          
        /* endif */                
    };
} ast_stmt;

#endif
