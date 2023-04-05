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

static inline lex_token_t parser_at() 
{
    return conf.lexer_data.tokens[0];
}

static inline size_t parser_line()
{
    return parser_at().line + 1;
}

static void parser_error(bool should_exit, const char *fmt, ...)
{
    va_list args;
    char fmt_processed[strlen(fmt) + 50];
    va_start(args, fmt);

    sprintf(fmt_processed, "\033[1;31mParse error\033[0m: %s at line %lu\n", fmt, parser_line());
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
        parser_error(true, "Unexpected end of file");
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
        printf("\033[33m%Lf\033[0m", left->value);
    else if (left->type == NODE_IDENTIFIER)
        printf("%s", left->symbol);
    else if (left->type == NODE_EXPR_BINARY)
        __debug_parser_print_ast_stmt_binop(left);
    else 
        printf("[Unknown %d]", left->type);

    printf(" %c ", __debug_parser_print_ast_stmt_binop_operator_to_str(binop->operator));

    ast_stmt *right = binop->right;

    if (right->type == NODE_NUMERIC_LITERAL)
        printf("\033[33m%Lf\033[0m", right->value);
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
            printf(" Number: %Lf", prog->body[i].value);
        else if (prog->body[i].type == NODE_DECL_VAR)
            printf(" Variable declared: %s", prog->body[i].identifier);
        else if (prog->body[i].type == NODE_EXPR_ASSIGNMENT)
            printf(" Assignment: %s = %p", prog->body[i].assignee->symbol, prog->body[i].assignment_value);
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

static lex_token_t parser_expect(lex_tokentype_t tokentype, const char *error_fmt, ...)
{
    lex_token_t token = parser_shift();

    if (token.type != tokentype) {
        va_list args;
        char fmt_processed[strlen(error_fmt) + 50];

        va_start(args, error_fmt);

        sprintf(fmt_processed, "\033[1;31mParse error\033[0m: %s at line %lu\n", error_fmt, token.line);
        vfprintf(stderr, fmt_processed, args);

        va_end(args);
        exit(EXIT_FAILURE);
    }

    return token;
} 

ast_stmt parser_parse_primary_expr()
{
    ast_stmt stmt;
    lex_token_t token = parser_at();

    switch (token.type)
    {
        case T_IDENTIFIER:
            stmt.type = NODE_IDENTIFIER;
            stmt.line = parser_line();
            stmt.symbol = parser_shift().value;
        break;

        case T_NUMBER: {
            stmt.type = NODE_NUMERIC_LITERAL;
            stmt.line = parser_line();
            stmt.value = (long double) atof(parser_shift().value);
        }
        break;

        case T_PAREN_OPEN:
        {
            parser_shift();
            ast_stmt stmt = parser_parse_expr();
            stmt.line = parser_line();
            parser_expect(T_PAREN_CLOSE, "Unexpcted token found. Expecting ')' (T_PAREN_CLOSE)\n");
            return stmt;
        }
        break;

        default:
            parser_error(true, "Unexpected token '%s' found", token.value);
            parser_shift();
        break;
    } 

    return stmt;
}

static ast_operator_t parser_char_to_operator(char c)
{
    switch (c) 
    {
        case '+':
            return OP_PLUS;
        case '-':
            return OP_MINUS;
        case '*':
            return OP_TIMES;
        case '/':
            return OP_DIVIDE;
        case '%':
            return OP_MOD;
        default:
            fprintf(stderr, "Parse error: Invalid binary operator: %c\n", c);
            exit(-1);
    }
}

static ast_stmt parser_parse_multiplicative_expr()
{
    ast_stmt left = parser_parse_primary_expr();
    size_t line;

    while (conf.lexer_data.size > 0 && conf.lexer_data.tokens[0].type == T_BINARY_OPERATOR &&
        (conf.lexer_data.tokens[0].value[0] == '/' || 
            conf.lexer_data.tokens[0].value[0] == '*' || 
                conf.lexer_data.tokens[0].value[0] == '%')) 
    {
        line = parser_line();

        char operator = parser_shift().value[0];
        ast_stmt right = parser_parse_primary_expr();

        ast_stmt binop = {
            .type = NODE_EXPR_BINARY,
            .operator = parser_char_to_operator(operator),
            .line = line
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
    size_t line;

    while (conf.lexer_data.size > 0 && conf.lexer_data.tokens[0].type == T_BINARY_OPERATOR &&
        (conf.lexer_data.tokens[0].value[0] == '+' || 
            conf.lexer_data.tokens[0].value[0] == '-')) 
    {
        line = parser_line();

        char operator = parser_shift().value[0];
        ast_stmt right = parser_parse_multiplicative_expr();

        ast_stmt binop = {
            .type = NODE_EXPR_BINARY,
            .operator = parser_char_to_operator(operator),
            .line = line
        };

        binop.left = xmalloc(sizeof (ast_stmt));
        memcpy(binop.left, &left, sizeof left);

        binop.right = xmalloc(sizeof (ast_stmt));
        memcpy(binop.right, &right, sizeof right);

        left = binop;
    } 

    return left;
}

ast_stmt parser_parse_assignment_expr()
{
    ast_stmt left = parser_parse_additive_expr(); // FIXME: use objexpr instead of this
    size_t line = parser_line();

    if (parser_at().type == T_ASSIGNMENT) 
    {
        parser_shift();
        ast_stmt val = parser_parse_assignment_expr();

        if (val.type != NODE_EXPR_ASSIGNMENT)
            parser_expect(T_SEMICOLON, "Expected semicolon (T_SEMICOLON) after assignment");

        ast_stmt *val_heap = xmalloc(sizeof (ast_stmt)),
                 *left_heap = xmalloc(sizeof (ast_stmt));

        memcpy(val_heap, &val, sizeof val);
        memcpy(left_heap, &left, sizeof left);

        return (ast_stmt) {
            .type = NODE_EXPR_ASSIGNMENT,
            .assignee = left_heap,
            .assignment_value = val_heap,
            .line = line
        };
    }

    return left;
}

ast_stmt parser_parse_expr()
{
    return parser_parse_assignment_expr();
}

ast_stmt parser_parse_var_decl()
{
    bool is_const = parser_shift().type == T_CONST;
    char *identifier = parser_expect(T_IDENTIFIER, "Expected identifier after %s (%s)\n", is_const ? "const" : "var", is_const ? "T_CONST" : "T_VAR").value;
    size_t line = parser_line();

    if (parser_at().type == T_SEMICOLON)
    {
        parser_shift();

        if (is_const)
            parser_error(true, "Unexpected semicolon, expected an assignment to constant declaration");

        return (ast_stmt) {
            .type = NODE_DECL_VAR,
            .is_const = is_const,
            .identifier = identifier,
            .has_val = false,
            .line = line
        };
    }

    parser_expect(T_ASSIGNMENT, "Expected assignment operator '=' (T_ASSIGNMENT)%s after %s (%s) name", 
        !is_const ? " or ';' (T_SEMICOLON)" : "", is_const ? "constant" : "variable", is_const ? "T_CONST" : "T_VAR");
    
    ast_stmt vardecl = {
        .type = NODE_DECL_VAR,
        .is_const = is_const,
        .identifier = identifier,
        .has_val = true,
        .line = line
    };

    ast_stmt val = parser_parse_expr();
    
    parser_expect(T_SEMICOLON, "Expected semicolon (T_SEMICOLON) after %s (%s) declaration", is_const ? "constant" : "variable", is_const ? "T_CONST" : "T_VAR");
    
    vardecl.varval = xmalloc(sizeof (ast_stmt));
    memcpy(vardecl.varval, &val, sizeof val);

    return vardecl;
}

ast_stmt parser_parse_stmt()
{
    switch (parser_at().type)
    {
        case T_VAR:
        case T_CONST:
            return parser_parse_var_decl();
        break;

        default:
            return parser_parse_expr();
    }
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

#ifndef _NODEBUG
#ifdef _DEBUG
    __debug_lex_print_token_array(&conf.lexer_data);
#endif
#endif

    while (!parser_eof())
    {
        ast_stmt stmt = parser_parse_stmt();
        prog.body = xrealloc(prog.body, sizeof (ast_stmt) * (prog.size + 1));
        prog.body[prog.size++] = stmt;
    }

    lex_free(&conf.lexer_data);

    return prog;
}
