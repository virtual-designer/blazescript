/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "alloca.h"

#include "ast.h"
#include "log.h"
#include "utils.h"

#define PARSER_ERROR_ARGS(parser, fmt, ...) \
    {                                    \
        SYNTAX_ERROR_LINE_ARGS(parser->filename, parser_at(parser).line_start, parser_at(parser).column_start, fmt, __VA_ARGS__); \
        exit(0);                                                \
    }

static ast_node_t parser_parse_stmt(struct parser *parser);
static ast_node_t parser_parse_expr(struct parser *parser);
static ast_node_t parser_parse_primary_expr(struct parser *parser);
static ast_node_t parser_parse_binexp_additive(struct parser *parser);
static ast_node_t parser_parse_binexp_multiplicative(struct parser *parser);
static ast_node_t parser_parse_var_decl(struct parser *parser);
static ast_node_t parser_parse_assignment_expr(struct parser *parser);
static ast_node_t parser_parse_fn_decl(struct parser *parser);
static ast_node_t parser_parse_array_lit(struct parser *parser);
static ast_node_t parser_parse_binexp_comparison(struct parser *parser);
static ast_node_t parser_parse_block(struct parser *parser);
static ast_node_t parser_parse_if(struct parser *parser);
static ast_node_t parser_parse_loop_stmt(struct parser *parser);

struct parser parser_init()
{
    struct parser parser;
    parser.token_count = 0;
    parser.index = 0;
    parser.tokens = NULL;
    parser.filename = NULL;
    return parser;
}

struct parser parser_init_from_lex(struct lex *lex)
{
    struct parser parser = parser_init();
    parser.token_count = lex_get_token_count(lex);
    parser.tokens = lex_get_tokens(lex);
    parser.filename = strdup(lex_get_filename(lex));
    return parser;
}

void parser_set_tokens(struct parser *parser, struct lex_token *tokens, size_t count)
{
    parser->token_count = count;
    parser->tokens = tokens;
}

void parser_set_filename(struct parser *parser, const char *filename)
{
    parser->filename = strdup(filename);
}

static inline struct lex_token parser_at(struct parser *parser)
{
    assert(parser->index < parser->token_count && "No more token to return");
    return parser->tokens[parser->index];
}

static inline struct lex_token parser_ret_forward(struct parser *parser)
{
    assert(parser->index < parser->token_count && "No more token to return");
    return parser->tokens[parser->index++];
}

static inline bool parser_is_eof(struct parser *parser)
{
    return parser->index >= parser->token_count || parser_at(parser).type == T_EOF;
}

static inline struct lex_token parser_expect(struct parser *parser, enum lex_token_type type)
{
    if (parser_is_eof(parser))
    {
        PARSER_ERROR_ARGS(parser, "unexpected end of file, expecting %s", lex_token_to_str(type));
    }
    else if (parser_at(parser).type != type)
    {
        PARSER_ERROR_ARGS(parser, "unexpected token '%s' (%s), expecting %s",
              parser_at(parser).value,
              lex_token_to_str(parser_at(parser).type),
              lex_token_to_str(type));
    }

    return parser->tokens[parser->index++];
}

ast_node_t parser_create_ast_node(struct parser *parser)
{
    ast_root_t *root = xcalloc(1, sizeof(ast_node_t));

    root->size = 0;
    root->nodes = NULL;

    while (!parser_is_eof(parser)) {
        root->nodes =
            xrealloc(root->nodes, (++root->size) * sizeof(ast_node_t));
        root->nodes[root->size - 1] = parser_parse_stmt(parser);
    }

    ast_node_t node;

    node.type = NODE_ROOT;
    node.root = root;

    return node;
}

ast_node_t *parser_ast_deep_copy(ast_node_t *node)
{
    ast_node_t *copy = xcalloc(1, sizeof(ast_node_t));
    memcpy(copy, node, sizeof (ast_node_t));

    switch (node->type) {
        case NODE_ROOT:
            copy->root = xcalloc(1, sizeof(ast_root_t));
            copy->root->size = 0;
            copy->root->nodes = NULL;

            for (size_t i = 0; i < node->root->size; i++)
            {
                copy->root->nodes =
                    xrealloc(copy->root->nodes,
                                  sizeof(ast_node_t) * (++copy->root->size));
                copy->root->nodes[copy->root->size - 1] = *parser_ast_deep_copy(&node->root->nodes[i]);
            }

            break;

        case NODE_IDENTIFIER:
            copy->identifier = xcalloc(1, sizeof(ast_identifier_t));
            copy->identifier->symbol = strdup(node->identifier->symbol);
            break;

        case NODE_STRING:
            copy->string = xcalloc(1, sizeof(ast_string_t));
            copy->string->strval = strdup(node->string->strval);
            break;

        case NODE_INT_LIT:
            copy->integer = xcalloc(1, sizeof(ast_intlit_t));
            copy->integer->intval = node->integer->intval;
            break;

        case NODE_BINARY_EXPR:
            copy->binexpr = xcalloc(1, sizeof(ast_binexpr_t));
            copy->binexpr->left = parser_ast_deep_copy(node->binexpr->left);
            copy->binexpr->right = parser_ast_deep_copy(node->binexpr->right);
            copy->binexpr->operator = node->binexpr->operator;
            break;

        case NODE_ASSIGNMENT:
            copy->assignment_expr =
                xcalloc(1, sizeof(ast_assignment_expr_t));
            copy->assignment_expr->assignee = parser_ast_deep_copy(node->assignment_expr->assignee);
            copy->assignment_expr->value = parser_ast_deep_copy(node->assignment_expr->value);
            break;

        case NODE_EXPR_CALL:
            copy->fn_call = xcalloc(1, sizeof(ast_call_t));
            copy->fn_call->argc = 0;
            copy->fn_call->args = NULL;

            for (size_t i = 0; i < node->fn_call->argc; i++)
            {
                copy->fn_call->args = xrealloc(
                    copy->fn_call->args,
                    sizeof(ast_node_t) * (++copy->fn_call->argc));
                copy->fn_call->args[copy->fn_call->argc - 1] = *parser_ast_deep_copy(&node->fn_call->args[i]);
            }

            copy->fn_call->identifier =
                xcalloc(1, sizeof(ast_identifier_t));
            copy->fn_call->identifier->symbol = strdup(node->fn_call->identifier->symbol);
            break;

        case NODE_VAR_DECL:
            copy->var_decl = xcalloc(1, sizeof(ast_var_decl_t));
            copy->var_decl->name = strdup(node->var_decl->name);

            if (node->var_decl->value != NULL)
                copy->var_decl->value = parser_ast_deep_copy(node->var_decl->value);
            else
                copy->var_decl->value = NULL;
            break;

        case NODE_FN_DECL:
            copy->fn_decl = xcalloc(1, sizeof(ast_fn_decl_t));
            copy->fn_decl->body = NULL;
            copy->fn_decl->size = 0;

            for (size_t i = 0; i < node->fn_decl->size; i++)
            {
                copy->fn_decl->body = xrealloc(
                    copy->fn_decl->body,
                    sizeof(ast_node_t) * (++copy->fn_decl->size));
                copy->fn_decl->body[copy->fn_decl->size - 1] = *parser_ast_deep_copy(&node->fn_decl->body[i]);
            }

            copy->fn_decl->param_names = NULL;
            copy->fn_decl->param_count = 0;

            for (size_t i = 0; i < node->fn_decl->param_count; i++)
            {
                copy->fn_decl->param_names = xrealloc(
                    copy->fn_decl->param_names,
                    sizeof(char *) * (++copy->fn_decl->param_count));
                copy->fn_decl->param_names[copy->fn_decl->param_count - 1] = strdup(node->fn_decl->param_names[i]);
            }

            copy->fn_decl->identifier =
                xcalloc(1, sizeof(ast_identifier_t));
            copy->fn_decl->identifier->symbol = strdup(node->fn_decl->identifier->symbol);
            break;

        case NODE_BLOCK:
            copy->block = xcalloc(1, sizeof (ast_block_t));
            copy->block->size = node->block->size;
            copy->block->children = node->block->children;
            break;

        default:
            fatal_error("%s(): AST type not recognized: (%d)", __func__, node->type);
    }

    return copy;
}

void parser_free(struct parser *parser)
{
    free(parser->filename);
}

static ast_node_t *create_node()
{
    return xcalloc(1, sizeof(ast_node_t));
}

static ast_node_t parser_parse_loop_stmt(struct parser *parser)
{
    ast_loop_stmt_t *loop_stmt = xcalloc(1, sizeof(ast_loop_stmt_t));
    ast_node_t node = {
        .type = NODE_LOOP_STMT,
    };

    loop_stmt->iter_varname = NULL;
    loop_stmt->iter_count = NULL;

    parser_expect(parser, T_LOOP);

    if (parser_at(parser).type == T_PAREN_OPEN)
    {
        parser_expect(parser, T_PAREN_OPEN);
        ast_node_t iter_count = parser_parse_expr(parser);

        if (parser_at(parser).type != T_PAREN_CLOSE)
        {
            parser_expect(parser, T_AS);
            char *iter_varname = parser_expect(parser, T_IDENTIFIER).value;
            loop_stmt->iter_varname = strdup(iter_varname);
        }

        loop_stmt->iter_count = create_node();
        memcpy(loop_stmt->iter_count, &iter_count, sizeof iter_count);
        parser_expect(parser, T_PAREN_CLOSE);
    }

    if (parser_at(parser).type == T_IDENTIFIER)
    {
        char *iter_varname = parser_expect(parser, T_IDENTIFIER).value;
        loop_stmt->iter_varname = strdup(iter_varname);
    }

    ast_node_t body = parser_parse_stmt(parser);

    loop_stmt->body = create_node();
    memcpy(loop_stmt->body, &body, sizeof body);
    node.loop_stmt = loop_stmt;
    return node;
}

static ast_node_t parser_parse_if(struct parser *parser)
{
    ast_if_stmt_t *if_node = xcalloc(1, sizeof (ast_if_stmt_t));
    ast_node_t node = {
        .type = NODE_IF_STMT,
    };

    parser_expect(parser, T_IF);
    parser_expect(parser, T_PAREN_OPEN);
    ast_node_t condition = parser_parse_expr(parser);
    parser_expect(parser, T_PAREN_CLOSE);
    ast_node_t if_body = parser_parse_stmt(parser);

    if (parser_at(parser).type == T_ELSE)
    {
        ast_node_t else_body;
        parser_expect(parser, T_ELSE);
        else_body = parser_parse_stmt(parser);
        if_node->else_block = create_node();
        memcpy(if_node->else_block, &else_body, sizeof else_body);
    }
    else
    {
        if_node->else_block = NULL;
    }

    if_node->condition = create_node();
    if_node->if_block = create_node();
    memcpy(if_node->condition, &condition, sizeof condition);
    memcpy(if_node->if_block, &if_body, sizeof if_body);
    node.if_stmt = if_node;
    return node;
}

static ast_node_t parser_parse_block(struct parser *parser)
{
    parser_expect(parser, T_BLOCK_BRACE_OPEN);

    ast_node_t node = {
        .type = NODE_BLOCK,
        .block = xcalloc(1, sizeof (ast_block_t))
    };

    size_t size = 0;
    node.block->children = NULL;

    while (!parser_is_eof(parser) && parser_at(parser).type != T_BLOCK_BRACE_CLOSE)
    {
        ast_node_t child_node = parser_parse_stmt(parser);
        node.block->children = xrealloc(node.block->children, (++size) * (sizeof (ast_node_t)));
        node.block->children[size - 1] = child_node;
    }

    parser_expect(parser, T_BLOCK_BRACE_CLOSE);
    node.block->size = size;
    return node;
}

static ast_node_t parser_parse_array_lit(struct parser *parser)
{
    if (parser_at(parser).type != T_ARRAY)
        return parser_parse_binexp_comparison(parser);

    parser_expect(parser, T_ARRAY);
    parser_expect(parser, T_SQUARE_BRACE_OPEN);

    ast_node_t node;
    node.type = NODE_ARRAY_LIT;
    node.array_lit = xcalloc(1, sizeof(*node.array_lit));
    node.array_lit->elements = vector_init();

    while (!parser_is_eof(parser) &&
           parser_at(parser).type != T_SQUARE_BRACE_CLOSE)
    {
        ast_node_t expr_orig = parser_parse_expr(parser);
        ast_node_t *expr = create_node();
        memcpy(expr, &expr_orig, sizeof expr_orig);

        vector_push(node.array_lit->elements, expr);

        if (parser_at(parser).type == T_SQUARE_BRACE_CLOSE)
            break;

        parser_expect(parser, T_COMMA);
    }

    parser_expect(parser, T_SQUARE_BRACE_CLOSE);
    return node;
}

static ast_node_t parser_parse_stmt(struct parser *parser)
{
    ast_node_t stmt;
    bool semicolon_is_expected = false;

    switch (parser_at(parser).type)
    {
        case T_VAR:
        case T_CONST:
            stmt = parser_parse_var_decl(parser);
            semicolon_is_expected = true;
            break;

        case T_FUNCTION:
            stmt = parser_parse_fn_decl(parser);
            break;

        case T_BLOCK_BRACE_OPEN:
            stmt = parser_parse_block(parser);
            break;

        case T_IF:
            stmt = parser_parse_if(parser);
            break;

        case T_LOOP:
            stmt = parser_parse_loop_stmt(parser);
            break;

        default:
            stmt = parser_parse_expr(parser);
            semicolon_is_expected = true;
    }

    if (semicolon_is_expected)
        parser_expect(parser, T_SEMICOLON);

    while (parser_at(parser).type == T_SEMICOLON)
        parser_ret_forward(parser);

    return stmt;
}

static ast_node_t parser_parse_fn_decl(struct parser *parser)
{
    struct lex_token first_token = parser_expect(parser, T_FUNCTION);
    struct lex_token fn_name_token = parser_expect(parser, T_IDENTIFIER);
    parser_expect(parser, T_PAREN_OPEN);

    ast_node_t node;

    node.type = NODE_FN_DECL;
    node.filename = parser->filename;
    node.fn_decl = xcalloc(1, sizeof(ast_fn_decl_t));
    node.fn_decl->identifier = xcalloc(1, sizeof(ast_identifier_t));
    node.fn_decl->identifier->symbol = strdup(fn_name_token.value);
    node.fn_decl->param_names = NULL;
    node.fn_decl->param_count = 0;
    node.fn_decl->body = NULL;
    node.fn_decl->size = 0;
    node.line_start = first_token.line_start;
    node.column_start = first_token.column_start;

    while (!parser_is_eof(parser) && parser_at(parser).type != T_PAREN_CLOSE)
    {
        struct lex_token identifier = parser_expect(parser, T_IDENTIFIER);
        node.fn_decl->param_names =
            xrealloc(node.fn_decl->param_names,
                          sizeof(char *) * (++node.fn_decl->param_count));
        node.fn_decl->param_names[node.fn_decl->param_count - 1] = strdup(identifier.value);

        if (parser_at(parser).type == T_PAREN_CLOSE)
            break;

        parser_expect(parser, T_COMMA);
    }

    parser_expect(parser, T_PAREN_CLOSE);
    parser_expect(parser, T_BLOCK_BRACE_OPEN);

    while (!parser_is_eof(parser) && parser_at(parser).type != T_BLOCK_BRACE_CLOSE)
    {
        ast_node_t stmt = parser_parse_stmt(parser);
        node.fn_decl->body =
            xrealloc(node.fn_decl->body,
                          sizeof(ast_node_t) * (++node.fn_decl->size));
        node.fn_decl->body[node.fn_decl->size - 1] = stmt;
    }

    struct lex_token last_token = parser_expect(parser, T_BLOCK_BRACE_CLOSE);

    node.line_end = last_token.line_end;
    node.column_end = last_token.column_end;

    return node;
}

static ast_node_t parser_parse_call_expr(struct parser *parser)
{
    if (parser->token_count > (parser->index + 2) &&
        parser->tokens[parser->index].type == T_IDENTIFIER &&
        parser->tokens[parser->index + 1].type == T_PAREN_OPEN)
    {
        ast_node_t node;

        node.type = NODE_EXPR_CALL;
        node.filename = parser->filename;
        node.fn_call = xcalloc(1, sizeof(ast_call_t));
        node.fn_call->identifier = xcalloc(1, sizeof(ast_identifier_t));
        node.fn_call->argc = 0;
        node.fn_call->args = NULL;
        node.line_start = parser_at(parser).line_start;
        node.column_start = parser_at(parser).column_start;

        struct lex_token identifier = parser_expect(parser, T_IDENTIFIER);
        node.fn_call->identifier->symbol = strdup(identifier.value);

        parser_expect(parser, T_PAREN_OPEN);

        while (!parser_is_eof(parser) && parser_at(parser).type != T_PAREN_CLOSE)
        {
            ast_node_t param = parser_parse_expr(parser);
            node.fn_call->args =
                xrealloc(node.fn_call->args,
                              sizeof(ast_node_t) * (++node.fn_call->argc));
            node.fn_call->args[node.fn_call->argc - 1] = param;

            if (parser_at(parser).type == T_COMMA)
                parser_expect(parser, T_COMMA);
        }

        parser_expect(parser, T_PAREN_CLOSE);

        node.line_end = parser_at(parser).line_end;
        node.column_end = parser_at(parser).column_end;

        return node;
    }

    return parser_parse_primary_expr(parser);
}

static ast_node_t parser_parse_var_decl(struct parser *parser)
{
    assert(parser_at(parser).type == T_VAR || parser_at(parser).type == T_CONST);
    struct lex_token start_token = parser_ret_forward(parser);
    bool is_const = start_token.type == T_CONST;
    struct lex_token identifier = parser_expect(parser, T_IDENTIFIER);

    ast_node_t node;

    node.filename = parser->filename;
    node.type = NODE_VAR_DECL;
    node.var_decl = xcalloc(1, sizeof(ast_var_decl_t));
    node.line_start = start_token.line_start;
    node.column_start = start_token.column_start;

    node.var_decl->name = strdup(identifier.value);
    node.var_decl->is_const = is_const;

    if (parser_at(parser).type == T_SEMICOLON)
    {
        node.line_end = parser_at(parser).line_end;
        node.column_end = parser_at(parser).column_end;

        if (is_const)
        {
            PARSER_ERROR_ARGS(
                parser,
                "constant '%s' must have a value assigned to it when declaring",
                node.var_decl->name);
        }

        return node;
    }
    
    parser_expect(parser, T_ASSIGNMENT);

    ast_node_t expr_orig = parser_parse_expr(parser);
    ast_node_t *expr = create_node();
    memcpy(expr, &expr_orig, sizeof expr_orig);
    node.var_decl->value = expr;
    node.line_end = parser_at(parser).line_end;
    node.column_end = parser_at(parser).column_end;

    return node;
}

static ast_node_t parser_parse_expr(struct parser *parser)
{
    return parser_parse_assignment_expr(parser);
}

static ast_node_t parser_parse_assignment_expr(struct parser *parser)
{
    if (!parser_is_eof(parser) && (parser->index + 1) < parser->token_count &&
        parser->tokens[parser->index + 1].type == T_ASSIGNMENT)
    {
        printf("LMAO\n");
        struct lex_token start_token = parser_expect(parser, T_IDENTIFIER);
        char *identifier = strdup(start_token.value);
        parser_expect(parser, T_ASSIGNMENT);
        ast_node_t value_orig = parser_parse_expr(parser);
        ast_node_t *value = create_node();
        memcpy(value, &value_orig, sizeof (ast_node_t));
        ast_node_t node;
        node.filename = parser->filename;
        node.type = NODE_ASSIGNMENT;
        node.assignment_expr = xcalloc(1, sizeof(ast_assignment_expr_t));
        node.line_start = start_token.line_start;
        node.column_start = start_token.column_start;

        node.assignment_expr->assignee = xcalloc(1, sizeof(ast_node_t));
        node.assignment_expr->assignee->type = NODE_IDENTIFIER;
        node.assignment_expr->assignee->line_start = start_token.line_start;
        node.assignment_expr->assignee->line_end = start_token.line_end;
        node.assignment_expr->assignee->column_start = start_token.column_start;
        node.assignment_expr->assignee->column_end = start_token.column_end;
        node.assignment_expr->assignee->identifier =
            xcalloc(1, sizeof(ast_identifier_t));
        node.assignment_expr->assignee->identifier->symbol = identifier;
        node.assignment_expr->value = value;
        node.line_end = parser_at(parser).line_end;
        node.column_end = parser_at(parser).column_end;
        return node;
    }

    return parser_parse_array_lit(parser);
}

static ast_node_t parser_parse_binexp_inner(struct parser *parser, const char operator, ast_node_t left, ast_node_t right)
{
    ast_node_t binexpr;

    binexpr.filename = parser->filename;
    binexpr.type = NODE_BINARY_EXPR;
    binexpr.binexpr = xcalloc(1, sizeof(ast_binexpr_t));
    binexpr.line_start = left.line_start;
    binexpr.column_start = left.column_start;
    binexpr.line_end = right.line_end;
    binexpr.column_end = right.column_end;

    binexpr.binexpr->operator = (unsigned char) operator;
    binexpr.binexpr->left = create_node();
    memcpy(binexpr.binexpr->left, &left, sizeof left);
    binexpr.binexpr->right = create_node();
    memcpy(binexpr.binexpr->right, &right, sizeof right);

    return binexpr;
}

static ast_bin_operator_t parser_parse_binexp_operator(struct parser *parser)
{
    ast_bin_operator_t operator;
    const char *operator_str = parser_expect(parser, T_BINARY_OPERATOR).value;

    if (strcmp(operator_str, ">=") == 0)
        operator = OP_CMP_GE;
    else if (strcmp(operator_str, "<=") == 0)
        operator = OP_CMP_LE;
    else if (strcmp(operator_str, "==") == 0)
        operator = OP_CMP_EQ;
    else if (strcmp(operator_str, "===") == 0)
        operator = OP_CMP_EQ_S;
    else if (strcmp(operator_str, "!=") == 0)
        operator = OP_CMP_NE;
    else if (strcmp(operator_str, "!==") == 0)
        operator = OP_CMP_NE_S;
    else
        operator = (ast_bin_operator_t) operator_str[0];

    return operator;
}

static ast_node_t parser_parse_binexp_multiplicative(struct parser *parser)
{
    ast_node_t left = parser_parse_call_expr(parser);

    while (!parser_is_eof(parser) && parser_at(parser).type == T_BINARY_OPERATOR &&
           (parser_at(parser).value[0] == OP_TIMES || parser_at(parser).value[0] == OP_DIVIDE ||
            parser_at(parser).value[0] == OP_MODULUS))
    {
        ast_bin_operator_t operator = parser_parse_binexp_operator(parser);
        ast_node_t right = parser_parse_call_expr(parser);
        left = parser_parse_binexp_inner(parser, operator, left, right);
    }

    return left;
}

static ast_node_t parser_parse_binexp_additive(struct parser *parser)
{
    ast_node_t left = parser_parse_binexp_multiplicative(parser);

    while (!parser_is_eof(parser) && parser_at(parser).type == T_BINARY_OPERATOR &&
           (parser_at(parser).value[0] == OP_PLUS || parser_at(parser).value[0] == OP_MINUS))
    {
        ast_bin_operator_t operator = parser_parse_binexp_operator(parser);
        ast_node_t right = parser_parse_binexp_multiplicative(parser);
        left = parser_parse_binexp_inner(parser, operator, left, right);
    }

    return left;
}

static ast_node_t parser_parse_binexp_comparison(struct parser *parser)
{
    ast_node_t left = parser_parse_binexp_additive(parser);

    while (!parser_is_eof(parser) && parser_at(parser).type == T_BINARY_OPERATOR &&
           (parser_at(parser).value[0] == OP_CMP_GT || parser_at(parser).value[0] == OP_CMP_LT ||
            strcmp(parser_at(parser).value, ">=") == 0 || strcmp(parser_at(parser).value, "<=") == 0 ||
            strcmp(parser_at(parser).value, "==") == 0 || strcmp(parser_at(parser).value, "===") == 0 ||
            strcmp(parser_at(parser).value, "!=") == 0 || strcmp(parser_at(parser).value, "!==") == 0))
    {
        ast_bin_operator_t operator = parser_parse_binexp_operator(parser);
        ast_node_t right = parser_parse_binexp_additive(parser);
        left = parser_parse_binexp_inner(parser, operator, left, right);
    }

    return left;
}

static ast_node_t parser_parse_primary_expr(struct parser *parser)
{
    struct lex_token token = parser_at(parser);

    switch (token.type)
    {
        case T_IDENTIFIER:
        {
            parser_ret_forward(parser);

            ast_node_t identifier;

            identifier.filename = parser->filename;
            identifier.type = NODE_IDENTIFIER;
            identifier.identifier = xcalloc(1, sizeof(ast_identifier_t));
            identifier.line_start = token.line_start;
            identifier.line_end = token.line_end;
            identifier.column_start = token.column_start;
            identifier.column_end = token.column_end;

            identifier.identifier->symbol = strdup(token.value);
            return identifier;
        }

        case T_INT_LIT:
        {
            parser_ret_forward(parser);
            assert(strspn(token.value, "0123456789") == strlen(token.value) && "Invalid integer");

            ast_node_t intlit;

            intlit.filename = parser->filename;
            intlit.type = NODE_INT_LIT;
            intlit.integer = xcalloc(1, sizeof(ast_intlit_t));
            intlit.line_start = token.line_start;
            intlit.line_end = token.line_end;
            intlit.column_start = token.column_start;
            intlit.column_end = token.column_end;

            intlit.integer->intval = atoll(token.value);
            return intlit;
        }

        case T_STRING:
        {
            parser_ret_forward(parser);

            ast_node_t string;

            string.filename = parser->filename;
            string.type = NODE_STRING;
            string.string = xcalloc(1, sizeof(ast_string_t));
            string.line_start = token.line_start;
            string.line_end = token.line_end;
            string.column_start = token.column_start;
            string.column_end = token.column_end;

            string.string->strval = strdup(token.value);

            return string;
        }

        case T_PAREN_OPEN:
        {
            parser_ret_forward(parser);
            ast_node_t node = parser_parse_expr(parser);
            parser_expect(parser, T_PAREN_CLOSE);
            return node;
        }

        default:
            PARSER_ERROR_ARGS(parser, "unexpected token '%s' (%s)", token.value, lex_token_to_str(token.type));
    }
}

const char *ast_type_to_str(enum ast_node_type type)
{
    const char *translate[] = {
        [NODE_IDENTIFIER] = "IDENTIFIER",
        [NODE_BINARY_EXPR] = "BINARY_EXPR",
        [NODE_INT_LIT] = "INT_LIT",
        [NODE_ROOT] = "ROOT",
        [NODE_STRING] = "STRING",
        [NODE_VAR_DECL] = "VAR_DECL",
        [NODE_ASSIGNMENT] = "ASSIGNMENT",
        [NODE_EXPR_CALL] = "CALL_EXPR",
        [NODE_FN_DECL] = "FN_DECL",
        [NODE_ARRAY_LIT] = "ARRAY_LIT",
        [NODE_BLOCK] = "BLOCK",
        [NODE_IF_STMT] = "IF_STMT",
        [NODE_LOOP_STMT] = "LOOP_STMT",
    };

    size_t length = sizeof (translate) / sizeof (const char *);

    assert(type < length && "Invalid node type");
    return translate[type];
}

void parser_ast_free_inner(ast_node_t *node)
{
    log_debug("Freeing: %p", node);

    switch (node->type) {
        case NODE_ROOT:
            for (size_t i = 0; i < node->root->size; i++)
            {
                parser_ast_free_inner(&node->root->nodes[i]);
            }

            free(node->root->nodes);
            free(node->root);
            break;

        case NODE_IDENTIFIER:
            free(node->identifier->symbol);
            free(node->identifier);
            break;

        case NODE_STRING:
            free(node->string->strval);
            free(node->string);
            break;

        case NODE_INT_LIT:
            free(node->integer);
            break;

        case NODE_BINARY_EXPR:
            parser_ast_free(node->binexpr->left);
            parser_ast_free(node->binexpr->right);
            free(node->binexpr);
            break;

        case NODE_ASSIGNMENT:
            parser_ast_free(node->assignment_expr->assignee);
            parser_ast_free(node->assignment_expr->value);
            free(node->assignment_expr);
            break;

        case NODE_EXPR_CALL:
            for (size_t i = 0; i < node->fn_call->argc; i++)
                parser_ast_free_inner(&node->fn_call->args[i]);

            free(node->fn_call->args);
            free(node->fn_call->identifier->symbol);
            free(node->fn_call->identifier);
            free(node->fn_call);
            break;

        case NODE_VAR_DECL:
            if (node->var_decl->value != NULL)
                parser_ast_free(node->var_decl->value);

            free(node->var_decl->name);
            free(node->var_decl);
            break;

        case NODE_ARRAY_LIT:
            VECTOR_FOREACH(node->array_lit->elements)
            {
                parser_ast_free(((ast_node_t **) node->array_lit->elements->data)[i]);
            }

            vector_free(node->array_lit->elements);
            free(node->array_lit);
            break;

        case NODE_FN_DECL:
            free(node->fn_decl->identifier->symbol);
            free(node->fn_decl->identifier);

            for (size_t i = 0; i < node->fn_decl->param_count; i++)
                free(node->fn_decl->param_names[i]);

            free(node->fn_decl->param_names);

            for (size_t i = 0; i < node->fn_decl->size; i++)
                parser_ast_free_inner(&node->fn_decl->body[i]);

            free(node->fn_decl->body);
            free(node->fn_decl);
            break;

        case NODE_BLOCK:
            for (size_t i = 0; i < node->block->size; i++)
                parser_ast_free_inner(&node->block->children[i]);

            free(node->block->children);
            free(node->block);
            break;

        case NODE_IF_STMT:
            parser_ast_free(node->if_stmt->condition);
            parser_ast_free(node->if_stmt->if_block);

            if (node->if_stmt->else_block != NULL)
                parser_ast_free(node->if_stmt->else_block);

            free(node->if_stmt);
            break;

        case NODE_LOOP_STMT:
            if (node->loop_stmt->iter_count != NULL)
                parser_ast_free(node->loop_stmt->iter_count);

            if (node->loop_stmt->iter_varname != NULL)
                free(node->if_stmt->if_block);

            parser_ast_free(node->loop_stmt->body);
            free(node->loop_stmt);
            break;

        default:
            fatal_error("parser_ast_free_inner(): AST type not recognized: %s (%d)",
                     ast_type_to_str(node->type), node->type);
    }
}

void parser_ast_free(ast_node_t *node)
{
    parser_ast_free_inner(node);
    free(node);
}

#ifndef NDEBUG
static void blaze_debug__print_ast_indent(int indent_level)
{
    for (int i = 0; i < indent_level; i++)
        printf("\t");
}

static void __attribute__((format(printf, 2, 3))) blaze_debug__print_ast_indent_string(int indent_level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    blaze_debug__print_ast_indent(indent_level);
    vprintf(fmt, args);
    va_end(args);
}

static void blaze_debug__print_ast_internal(ast_node_t *node, int indent_level, bool newline, bool firstindent)
{
    blaze_debug__print_ast_indent_string(firstindent ? indent_level : 0, "Node {\n");
    int inner_indent_level = indent_level + 1;

    blaze_debug__print_ast_indent_string(inner_indent_level, "type: %s(%d),\n",
                                         ast_type_to_str(node->type), node->type);

    switch (node->type) {
        case NODE_ROOT:
            blaze_debug__print_ast_indent_string(inner_indent_level, "capacity: %lu,\n", node->root->size);
            blaze_debug__print_ast_indent_string(inner_indent_level, "children: [\n");

            for (size_t i = 0; i < node->root->size; i++)
            {
                blaze_debug__print_ast_internal(&node->root->nodes[i], inner_indent_level + 1, false, true);

                if (i < node->root->size - 1)
                    printf(",");

                printf("\n");
            }

            blaze_debug__print_ast_indent_string(inner_indent_level, "]\n");
            break;

        case NODE_BLOCK:
            blaze_debug__print_ast_indent_string(inner_indent_level, "capacity: %lu,\n", node->block->size);
            blaze_debug__print_ast_indent_string(inner_indent_level, "children: [\n");

            for (size_t i = 0; i < node->block->size; i++)
            {
                blaze_debug__print_ast_internal(&node->block->children[i], inner_indent_level + 1, false, true);

                if (i < node->block->size - 1)
                    printf(",");

                printf("\n");
            }

            blaze_debug__print_ast_indent_string(inner_indent_level, "]\n");
            break;

        case NODE_IF_STMT:
            blaze_debug__print_ast_indent_string(inner_indent_level, "condition: ");
            blaze_debug__print_ast_internal(node->if_stmt->condition, inner_indent_level, false, false);
            printf(",\n");
            blaze_debug__print_ast_indent_string(inner_indent_level, "if_body: ");
            blaze_debug__print_ast_internal(node->if_stmt->if_block, inner_indent_level, false, false);
            printf(",\n");
            blaze_debug__print_ast_indent_string(inner_indent_level, "else_body: ");

            if (node->if_stmt->else_block == NULL)
                puts("[None]");
            else
                blaze_debug__print_ast_internal(node->if_stmt->else_block, inner_indent_level, true, false);

            break;

        case NODE_LOOP_STMT:
            blaze_debug__print_ast_indent_string(inner_indent_level, "count: ");
            if (node->loop_stmt->iter_count == NULL)
                printf("null");
            else
                blaze_debug__print_ast_internal(node->loop_stmt->iter_count, inner_indent_level, false, false);
            printf(",\n");
            blaze_debug__print_ast_indent_string(inner_indent_level, "varname: %s,\n", node->loop_stmt->iter_varname == NULL ? "null" : node->loop_stmt->iter_varname);
            blaze_debug__print_ast_indent_string(inner_indent_level, "body: ");
            blaze_debug__print_ast_internal(node->loop_stmt->body, inner_indent_level, true, false);
            break;

        case NODE_ARRAY_LIT:
            blaze_debug__print_ast_indent_string(inner_indent_level, "length: %lu,\n", node->array_lit->elements->length);
            blaze_debug__print_ast_indent_string(inner_indent_level, "children: [\n");

            VECTOR_FOREACH(node->array_lit->elements)
            {
                blaze_debug__print_ast_internal(((ast_node_t **) node->array_lit->elements->data)[i], inner_indent_level + 1, false, true);

                if (i < node->array_lit->elements->length - 1)
                    printf(",");

                printf("\n");
            }

            blaze_debug__print_ast_indent_string(inner_indent_level, "]\n");
            break;

        case NODE_IDENTIFIER:
            blaze_debug__print_ast_indent_string(inner_indent_level, "symbol: \"%s\"\n", node->identifier->symbol);
            break;

        case NODE_STRING:
            blaze_debug__print_ast_indent_string(inner_indent_level, "value: \"%s\"\n", node->string->strval);
            break;

        case NODE_INT_LIT:
            blaze_debug__print_ast_indent_string(inner_indent_level, "value: %lld\n", node->integer->intval);
            break;

        case NODE_BINARY_EXPR:
            blaze_debug__print_ast_indent_string(inner_indent_level, "operator: '");
            ast_bin_operator_t operator = node->binexpr->operator;

            if (operator == OP_CMP_GE)
                printf(">=");
            else if (operator == OP_CMP_LE)
                printf("<=");
            else if (operator == OP_CMP_EQ)
                printf("==");
            else if (operator == OP_CMP_EQ_S)
                printf("===");
            else if (operator == OP_CMP_NE)
                printf("!=");
            else if (operator == OP_CMP_NE_S)
                printf("!==");
            else
                printf("%c", operator);

            printf("',\n");

            blaze_debug__print_ast_indent_string(inner_indent_level, "left: ");
            blaze_debug__print_ast_internal(node->binexpr->left, inner_indent_level, false, false);
            printf(", \n");
            blaze_debug__print_ast_indent_string(inner_indent_level, "right: ");
            blaze_debug__print_ast_internal(node->binexpr->right, inner_indent_level, true, false);
            break;

        case NODE_ASSIGNMENT:
            blaze_debug__print_ast_indent_string(inner_indent_level, "identifier: \"%s\",\n", node->assignment_expr->assignee->identifier->symbol);
            blaze_debug__print_ast_indent_string(inner_indent_level, "right: ");
            blaze_debug__print_ast_internal(node->assignment_expr->value, inner_indent_level, true, false);
            break;

        case NODE_EXPR_CALL:
            blaze_debug__print_ast_indent_string(inner_indent_level, "identifier: \"%s\",\n", node->fn_call->identifier->symbol);
            blaze_debug__print_ast_indent_string(inner_indent_level, "argc: %lu,\n", node->fn_call->argc);
            blaze_debug__print_ast_indent_string(inner_indent_level, "args: [\n");

            for (size_t i = 0; i < node->fn_call->argc; i++)
            {
                blaze_debug__print_ast_internal(&node->fn_call->args[i], inner_indent_level + 1, false, true);

                if (i < node->fn_call->argc - 1)
                    printf(",");

                printf("\n");
            }

            blaze_debug__print_ast_indent_string(inner_indent_level, "]\n");
            break;

        case NODE_FN_DECL:
            blaze_debug__print_ast_indent_string(inner_indent_level, "identifier: \"%s\",\n", node->fn_decl->identifier->symbol);
            blaze_debug__print_ast_indent_string(inner_indent_level, "param_count: %lu,\n", node->fn_decl->param_count);

            blaze_debug__print_ast_indent_string(inner_indent_level, "param_names: [\n");

            for (size_t i = 0; i < node->fn_decl->param_count; i++)
            {
                blaze_debug__print_ast_indent_string(inner_indent_level + 1, "%s", node->fn_decl->param_names[i]);

                if (i < node->fn_decl->param_count - 1)
                    printf(",");

                printf("\n");
            }

            blaze_debug__print_ast_indent_string(inner_indent_level, "],\n");
            blaze_debug__print_ast_indent_string(inner_indent_level, "size: %lu,\n", node->fn_decl->size);
            blaze_debug__print_ast_indent_string(inner_indent_level, "body: [\n");

            for (size_t i = 0; i < node->fn_decl->size; i++)
            {
                blaze_debug__print_ast_internal(&node->fn_decl->body[i], inner_indent_level + 1, false, true);

                if (i < node->fn_decl->size - 1)
                    printf(",");

                printf("\n");
            }

            blaze_debug__print_ast_indent_string(inner_indent_level, "]\n");
            break;

        case NODE_VAR_DECL:
            blaze_debug__print_ast_indent_string(inner_indent_level, "name: \"%s\",\n", node->var_decl->name);
            blaze_debug__print_ast_indent_string(inner_indent_level, "is_const: %s,\n", node->var_decl->is_const ? "true" : "false");
            blaze_debug__print_ast_indent_string(inner_indent_level, "value: ");

            if (node->var_decl->value != NULL)
                blaze_debug__print_ast_internal(node->var_decl->value, inner_indent_level, true, false);
            else
                printf("null\n");
            break;

        default:
            blaze_debug__print_ast_indent_string(inner_indent_level, "[Unknown]\n");
    }

    blaze_debug__print_ast_indent_string(indent_level, "}");

    if (newline)
        blaze_debug__print_ast_indent_string(indent_level, "\n");
}


void blaze_debug__print_ast(ast_node_t *node)
{
    if (node == NULL)
    {
        printf("[NULL AST]\n");
        return;
    }

    blaze_debug__print_ast_internal(node, 0, true, true);
}
#endif