#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "xmalloc.h"

typedef struct {
    lex_t lexer_data;
} parser_conf_t;

static parser_conf_t conf;

static void parser_error(bool should_exit, const char *fmt, ...)
{
    va_list args;
    char fmt_processed[strlen(fmt) + 50];
    va_start(args, fmt);

    sprintf(fmt_processed, "Parser error: %s\n", fmt);
    vfprintf(stderr, fmt_processed, args);

    va_end(args);

    if (should_exit)
        exit(EXIT_FAILURE);
}

bool parser_eof() 
{
    return conf.lexer_data.size == 0 || conf.lexer_data.tokens[0].type == T_EOF;
}

lex_token_t parser_shift()
{
    lex_token_t token;

    if (!lex_token_array_shift(&conf.lexer_data, &token))
    {
        parser_error(true, "array shift returned NULL");
    }

    return token;
}

static char __debug_parser_print_ast_stmt_binop_operator_to_str(ast_operator_t operator)
{
    if (operator == OP_PLUS)
        return '+';
    else if (operator == OP_MINUS)
        return '-';
    else if (operator == OP_TIMES)
        return '*';
    else if (operator == OP_DIVIDE)
        return '/';
    else if (operator == OP_MOD)
        return '%';
    else 
        return operator;
}

static void __debug_parser_print_ast_stmt_binop(ast_stmt *binop)
{
    putchar('(');

    ast_stmt *left = binop->left;

    if (left->type == NODE_NUMERIC_LITERAL)
        printf("\033[33m%lld\033[0m", left->value);
    else if (left->type == NODE_IDENTIFIER)
        printf("%s", left->symbol);
    else if (left->type == NODE_EXPR_BINARY)
        __debug_parser_print_ast_stmt_binop(left);
    else 
        printf("[Unknown %d]", left->type);

    printf(" %c ", __debug_parser_print_ast_stmt_binop_operator_to_str(binop->operator));

    ast_stmt *right = binop->right;

    if (right->type == NODE_NUMERIC_LITERAL)
        printf("\033[33m%lld\033[0m", right->value);
    else if (right->type == NODE_IDENTIFIER)
        printf("%s", right->symbol);
    else if (right->type == NODE_EXPR_BINARY)
        __debug_parser_print_ast_stmt_binop(right);
    else 
        printf("[Unknown %d]", right->type);
    
    putchar(')');
}

void __debug_parser_print_ast_stmt(ast_stmt *prog)
{
    assert(prog->type == NODE_PROGRAM);

    for (size_t i = 0; i < prog->size; i++)
    {
        printf("[%lu] - {%d}", i, prog->body[i].type);

        if (prog->body[i].type == NODE_IDENTIFIER) 
            printf(" Identifier: '%s'", prog->body[i].symbol);
        else if (prog->body[i].type == NODE_NUMERIC_LITERAL) 
            printf(" Number: %lld", prog->body[i].value);
        else if (prog->body[i].type == NODE_EXPR_BINARY) 
        {
            printf(" Binary expression: ");
            __debug_parser_print_ast_stmt_binop(&prog->body[i]);
            printf("\n");
        }
        else
            printf("Unknown node");

        printf("\n");
    }
}

static inline lex_token_t parser_at() 
{
    return conf.lexer_data.tokens[0];
}

ast_stmt parser_parse_primary_expr()
{
    ast_stmt stmt;
    lex_token_t token = conf.lexer_data.tokens[0];

    switch (token.type)
    {
        case T_IDENTIFIER:
            stmt.type = NODE_IDENTIFIER;
            stmt.symbol = parser_shift().value;
        break;

        case T_NUMBER:
            stmt.type = NODE_NUMERIC_LITERAL;
            stmt.value = atoll(parser_shift().value);
        break;

        case T_VAR:
            stmt.type = T_VAR;
            parser_shift();
        break;

        case T_ASSIGNMENT:
            stmt.type = T_ASSIGNMENT;
            parser_shift();
        break;

        default:
            parser_error(true, "unexpected token found");
            parser_shift();
        break;
    } 

    return stmt;
}

static ast_stmt parser_parse_multiplicative_expr()
{
    ast_stmt left = parser_parse_primary_expr();

    while (conf.lexer_data.size > 0 && conf.lexer_data.tokens[0].type == T_BINARY_OPERATOR &&
        (conf.lexer_data.tokens[0].value[0] == '/' || 
            conf.lexer_data.tokens[0].value[0] == '*' || 
                conf.lexer_data.tokens[0].value[0] == '%')) 
    {
        char operator = parser_shift().value[0];
        ast_stmt right = parser_parse_primary_expr();

        ast_stmt binop = {
            .type = NODE_EXPR_BINARY,
            .operator = operator,
        };

        binop.left = xmalloc(sizeof (ast_stmt));
        memcpy(binop.left, &left, sizeof left);

        binop.right = xmalloc(sizeof (ast_stmt));
        memcpy(binop.right, &right, sizeof right);

        left = binop;
    } 

    return left;
}

static ast_stmt parser_parse_additive_expr()
{
    ast_stmt left = parser_parse_multiplicative_expr();

    while (conf.lexer_data.size > 0 && conf.lexer_data.tokens[0].type == T_BINARY_OPERATOR &&
        (conf.lexer_data.tokens[0].value[0] == '+' || 
            conf.lexer_data.tokens[0].value[0] == '-')) 
    {
        char operator = parser_shift().value[0];
        ast_stmt right = parser_parse_multiplicative_expr();

        ast_stmt binop = {
            .type = NODE_EXPR_BINARY,
            .operator = operator,
        };

        binop.left = xmalloc(sizeof (ast_stmt));
        memcpy(binop.left, &left, sizeof left);

        binop.right = xmalloc(sizeof (ast_stmt));
        memcpy(binop.right, &right, sizeof right);

        left = binop;
    } 

    return left;
}

ast_stmt parser_parse_expr()
{
    return parser_parse_additive_expr();
}

ast_stmt parser_parse_stmt()
{
    return parser_parse_expr();
}

ast_stmt parser_create_ast(char *code)
{
    ast_stmt prog = {
        .type = NODE_PROGRAM,
        .body = NULL,
        .size = 0
    };

    conf.lexer_data = (lex_t) LEX_INIT;
    lex_tokenize(&conf.lexer_data, code);

    __debug_lex_print_token_array(&conf.lexer_data);

    while (!parser_eof())
    {
        ast_stmt stmt = parser_parse_stmt();
        prog.body = xrealloc(prog.body, sizeof (ast_stmt) * (prog.size + 1));
        prog.body[prog.size++] = stmt;
    }

    lex_free(&conf.lexer_data);

    return prog;
}
