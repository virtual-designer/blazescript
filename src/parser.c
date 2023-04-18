#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "blaze.h"
#include "xmalloc.h"
#include "vector.h"
#include "string.h"

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
    return parser_at().line;
}

static void parser_error(bool should_exit, const char *fmt, ...)
{
    va_list args;
    char fmt_processed[strlen(fmt) + 50];
    va_start(args, fmt);

    sprintf(fmt_processed, COLOR("1", "%s:%lu: ") COLOR("1;31", "parse error") ": %s\n", config.currentfile, parser_line(), fmt);
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
        else if (prog->body[i].type == NODE_STRING) 
            printf(" String: \033[33m%s\033[0m", prog->body[i].strval);
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

        sprintf(fmt_processed, COLOR("1", "%s:%lu: ") COLOR("1;31", "parse error") ": %s\n", config.currentfile, token.line, error_fmt);
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
    
        case T_STRING:
            stmt.type = NODE_STRING;
            stmt.line = parser_line();
            stmt.strval = parser_shift().value;
        break;

        case T_NUMBER: {
            stmt.type = NODE_NUMERIC_LITERAL;
            stmt.line = parser_line();
            stmt.value = (long double) atof(parser_at().value);
            stmt.is_float = strchr(parser_shift().value, '.') != NULL;
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
        case '!':
            return OP_LOGICAL_NOT;
        default:
            fprintf(stderr, "Parse error: Invalid binary operator: %c\n", c);
            exit(-1);
    }
}

static ast_stmt parser_parse_member_expr()
{
    ast_stmt object = parser_parse_primary_expr();

    while (parser_at().type == T_DOT || parser_at().type == T_ARRAY_BRACKET_OPEN)
    {
        lex_tokentype_t operator = parser_shift().type;
        ast_stmt prop;
        bool computed;

        if (operator == T_DOT)
        {
            computed = false;
            prop = parser_parse_primary_expr();

            if (prop.type != NODE_IDENTIFIER)
            {
                parser_error(true, "Cannot use dot operator without an identifier on the right side");
            }
        }
        else 
        {
            computed = true;
            prop = parser_parse_expr();
            parser_expect(T_ARRAY_BRACKET_CLOSE, "Expected closing brackets ']' after computed property access");
        }

        ast_stmt *object_heap = xmalloc(sizeof object);
        ast_stmt *prop_heap = xmalloc(sizeof prop);

        memcpy(object_heap, &object, sizeof object);
        memcpy(prop_heap, &prop, sizeof prop);

        object = (ast_stmt) {
            .type = NODE_EXPR_MEMBER_ACCESS,
            .object = object_heap,
            .prop = prop_heap,
            .computed = computed
        };
    }

    return object;
}

static void parser_parse_argument_list(vector_t *vector)
{
    VEC_PUSH((*vector), parser_parse_assignment_expr(), ast_stmt);

    while (!parser_eof() && parser_at().type == T_COMMA) 
    {
        parser_shift();
        VEC_PUSH((*vector), parser_parse_assignment_expr(), ast_stmt);
    }
}

/* Returns a vector of ast_stmt. */
static vector_t parser_parse_args()
{
    parser_expect(T_PAREN_OPEN, "Expected open parenthesis");
    vector_t vector = VEC_INIT;
    
    if (parser_at().type != T_PAREN_CLOSE)
        parser_parse_argument_list(&vector);

    parser_expect(T_PAREN_CLOSE, "Expected closing parenthesis after function argument list");
    return vector;
}

static ast_stmt parser_parse_call_expr(ast_stmt callee)
{
    ast_stmt call_expr = {
        .type = NODE_EXPR_CALL,
        .args = parser_parse_args()
    };

    call_expr.callee = xmalloc(sizeof callee);

    memcpy(call_expr.callee, &callee, sizeof callee);

    if (parser_at().type == T_PAREN_OPEN)
        call_expr = parser_parse_call_expr(call_expr);

    return call_expr;
}

static ast_stmt parser_parse_call_member_expr()
{
    ast_stmt member = parser_parse_member_expr();

    if (parser_at().type == T_PAREN_OPEN)
        return parser_parse_call_expr(member);

    return member;
}

static ast_stmt parser_parse_unary_expr()
{
    size_t line;

    if (parser_at().type != T_UNARY_OPERATOR || 
        (parser_at().value[0] != '+' && parser_at().value[0] != '-' && parser_at().value[0] != '!')) 
    {
        return parser_parse_call_member_expr();
    }

    char operator = parser_shift().value[0];
    line = parser_line();
    ast_stmt right = parser_parse_call_member_expr();

    ast_stmt ret = {
        .type = NODE_EXPR_UNARY,
        .right = xmalloc(sizeof right),
        .operator = operator == '+' ? OP_PLUS : operator == '-' ? OP_MINUS : OP_LOGICAL_NOT,
        .line = line
    };

    memcpy(ret.right, &right, sizeof right);

    return ret;
}

static ast_stmt parser_parse_multiplicative_expr()
{
    ast_stmt left = parser_parse_unary_expr();
    size_t line;

    while (conf.lexer_data.size > 0 && conf.lexer_data.tokens[0].type == T_BINARY_OPERATOR &&
        (conf.lexer_data.tokens[0].value[0] == '/' || 
            conf.lexer_data.tokens[0].value[0] == '*' || 
                conf.lexer_data.tokens[0].value[0] == '%')) 
    {
        line = parser_line();

        char operator = parser_shift().value[0];
        ast_stmt right = parser_parse_unary_expr();

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

static ast_stmt parser_parse_comparison_expr()
{
    ast_stmt left = parser_parse_additive_expr();
    size_t line;

    while (conf.lexer_data.size > 0 && conf.lexer_data.tokens[0].type == T_BINARY_OPERATOR &&
        (STREQ(parser_at().value, "=="))) 
    {
        line = parser_line();

        char *operator = parser_shift().value;
        ast_stmt right = parser_parse_additive_expr();

        ast_stmt binop = {
            .type = NODE_EXPR_BINARY,
            .operator = STREQ(operator, "==") ? OP_CMP_EQUALS : OP_CMP_EQUALS,
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

static ast_stmt parser_parse_bin_logical_expr()
{
    ast_stmt left = parser_parse_comparison_expr();
    size_t line;

    while (conf.lexer_data.size > 0 && conf.lexer_data.tokens[0].type == T_BINARY_OPERATOR &&
        (conf.lexer_data.tokens[0].value[0] == '&' || 
            conf.lexer_data.tokens[0].value[0] == '|')) 
    {
        line = parser_line();

        char operator = parser_shift().value[0];
        ast_stmt right = parser_parse_comparison_expr();

        ast_stmt binop = {
            .type = NODE_EXPR_BINARY,
            .operator = operator == '|' ? OP_LOGICAL_OR : OP_LOGICAL_AND,
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

ast_stmt parser_parse_object_expr()
{
    if (parser_at().type != T_BLOCK_BRACE_OPEN)
        return parser_parse_bin_logical_expr();

    parser_shift();
    
    vector_t properties = VEC_INIT; /* Vector of ast_stmt */

    while (!parser_eof() && parser_at().type != T_BLOCK_BRACE_CLOSE)
    {
        char *key = parser_expect(T_IDENTIFIER, "Expected object key identifier").value;

        if (parser_at().type == T_COMMA || parser_at().type == T_BLOCK_BRACE_CLOSE)
        {
            if (parser_at().type == T_COMMA)
                parser_shift();

            VEC_PUSH(properties, ((ast_stmt) {
                .type = NODE_PROPERTY_LITERAL,
                .key = key,
                .propval = NULL
            }), ast_stmt);

            continue;
        }

        parser_expect(T_COLON, "Expected colon after object property name");

        ast_stmt value = parser_parse_expr();
        ast_stmt *value_heap = xmalloc(sizeof value);

        memcpy(value_heap, &value, sizeof value);

        VEC_PUSH(properties, ((ast_stmt) {
            .type = NODE_PROPERTY_LITERAL,
            .key = key,
            .propval = value_heap
        }), ast_stmt);
    
        if (parser_at().type != T_BLOCK_BRACE_CLOSE)
            parser_expect(T_COMMA, "Expected ending comma ',' or closing braces '}' after object literal");
    }

    parser_expect(T_BLOCK_BRACE_CLOSE, "Expected closing brace '}' after object literal");

    return (ast_stmt) {
        .type = NODE_OBJECT_LITERAL,
        .properties = properties
    };
}

ast_stmt parser_parse_assignment_expr()
{
    ast_stmt left = parser_parse_object_expr();
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
    
    parser_expect(T_SEMICOLON, "Unexpected %s after %s (%s) declaration", lex_token_stringify(parser_at(), true), is_const ? "constant" : "variable", is_const ? "T_CONST" : "T_VAR");
    
    vardecl.varval = xmalloc(sizeof (ast_stmt));
    memcpy(vardecl.varval, &val, sizeof val);

    return vardecl;
}

ast_stmt parser_parse_function_decl()
{
    parser_shift();

    char *name = parser_expect(T_IDENTIFIER, "Expected identifier after function keyword").value;
    vector_t args = parser_parse_args(); /* Vector of ast_stmt. */
    vector_t argnames = VEC_INIT; /* Vector of (char *). */
    ast_stmt *body = NULL; 
    size_t size = 0;

    for (size_t i = 0; i < args.length; i++)
    {
        if (VEC_GET(args, i, ast_stmt).type != NODE_IDENTIFIER)
            parser_error(true, "Expected identifiers inside parenthesis");

        VEC_PUSH(argnames, VEC_GET(args, i, ast_stmt).symbol, char *);
    }

    VEC_FREE(args);

    parser_expect(T_BLOCK_BRACE_OPEN, "Expected open braces after function name and parameters");

    while (!parser_eof() && parser_at().type != T_BLOCK_BRACE_CLOSE)
    {
        ast_stmt stmt = parser_parse_stmt();
        
        if (stmt.type == NODE_EXPR_CALL && parser_at().type == T_SEMICOLON)
            parser_shift();

        body = xrealloc(body, sizeof (ast_stmt) * (++size));
        body[size - 1] = stmt;
    }

    parser_expect(T_BLOCK_BRACE_CLOSE, "Missing close braces after function body");
    
    return (ast_stmt) {
        .type = NODE_DECL_FUNCTION,
        .fn_name = name,
        .argnames = argnames,
        .size = size,
        .body = body
    };
}

ast_stmt parser_parse_codeblock()
{
    ast_stmt *body = NULL; 
    size_t size = 0;

    parser_expect(T_BLOCK_BRACE_OPEN, "Expected open braces to start block");

    while (!parser_eof() && parser_at().type != T_BLOCK_BRACE_CLOSE)
    {
        ast_stmt stmt = parser_parse_stmt();
        
        if (stmt.type == NODE_EXPR_CALL && parser_at().type == T_SEMICOLON)
            parser_shift();

        body = xrealloc(body, sizeof (ast_stmt) * (++size));
        body[size - 1] = stmt;
    }

    parser_expect(T_BLOCK_BRACE_CLOSE, "Missing close braces to end block");

    return (ast_stmt) {
        .type = NODE_BLOCK,
        .body = body,
        .size = size,
        .line = parser_at().line
    };
}

ast_stmt parser_parse_control_if()
{
    parser_shift();
    parser_expect(T_PAREN_OPEN, "Expected open parenthesis after if keyword");
    ast_stmt cond = parser_parse_expr();
    parser_expect(T_PAREN_CLOSE, "Expected close parenthesis before if body");
    
    bool is_if_block = parser_at().type == T_BLOCK_BRACE_OPEN;
    ast_stmt if_block = is_if_block ? parser_parse_codeblock() : parser_parse_stmt();

    while (!is_if_block && parser_at().type == T_SEMICOLON)
        parser_shift();

    ast_stmt if_cond = {
        .type = NODE_CTRL_IF,
        .if_body = xmalloc(sizeof (ast_stmt)),
        .if_cond = xmalloc(sizeof (ast_stmt)),
        .else_body = NULL,
        .line = parser_at().line
    };

    memcpy(if_cond.if_cond, &cond, sizeof (ast_stmt));
    memcpy(if_cond.if_body, &if_block, sizeof (ast_stmt));

    if (parser_at().type == T_ELSE)
    {        
        parser_shift();
        bool is_else_block = parser_at().type == T_BLOCK_BRACE_OPEN;
        ast_stmt else_block = parser_at().type == T_BLOCK_BRACE_OPEN ? parser_parse_codeblock() : parser_parse_stmt();
        if_cond.else_body = xmalloc(sizeof (ast_stmt));
        memcpy(if_cond.else_body, &else_block, sizeof (ast_stmt));

        while (!is_else_block && parser_at().type == T_SEMICOLON)
            parser_shift();
    }

    return if_cond;
}

ast_stmt parser_parse_stmt()
{
    switch (parser_at().type)
    {
        case T_VAR:
        case T_CONST:
            return parser_parse_var_decl();

        case T_FUNCTION:
            return parser_parse_function_decl();

        case T_IF:
            return parser_parse_control_if();

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
        if (parser_at().type == T_SEMICOLON)
        {
            parser_shift();
            continue;
        }
        
        ast_stmt stmt = parser_parse_stmt();
        prog.body = xrealloc(prog.body, sizeof (ast_stmt) * (prog.size + 1));
        prog.body[prog.size++] = stmt;
    }

    lex_free(&conf.lexer_data);

    return prog;
}
