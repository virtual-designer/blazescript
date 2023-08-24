/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "lexer.h"
#include "parser.h"
#include "alloca.h"

#include "ast.h"
#include "log.h"
#include "utils.h"

struct parser
{
    size_t index;
    size_t token_count;
    struct lex_token *tokens;
};

static ast_node_t parser_parse_stmt(struct parser *parser);
static ast_node_t parser_parse_expr(struct parser *parser);
static ast_node_t parser_parse_primary_expr(struct parser *parser);
static ast_node_t parser_parse_binexp_additive(struct parser *parser);
static ast_node_t parser_parse_binexp_multiplicative(struct parser *parser);

struct parser *parser_init()
{
    struct parser *parser = xcalloc(1, sizeof (struct parser));
    parser->token_count = 0;
    parser->index = 0;
    parser->tokens = NULL;
    return parser;
}

struct parser *parser_init_from_lex(struct lex *lex)
{
    struct parser *parser = parser_init();
    parser->token_count = lex_get_token_count(lex);
    parser->tokens = lex_get_tokens(lex);
    return parser;
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
        syntax_error("unexpected end of file, expecting %s", lex_token_to_str(type));
    else if (parser_at(parser).type != type)
        syntax_error("unexpected token '%s' (%s), expecting %s", parser_at(parser).value,
                     lex_token_to_str(parser_at(parser).type), lex_token_to_str(type));

    return parser->tokens[parser->index++];
}

ast_node_t *parser_create_ast_node(struct parser *parser)
{
    ast_root_t *root = xcalloc(1, sizeof (ast_node_t));

    root->size = 0;
    root->nodes = NULL;

    while (!parser_is_eof(parser)) {
        root->nodes = xrealloc(root->nodes, (++root->size) * sizeof (ast_node_t));
        root->nodes[root->size - 1] = parser_parse_stmt(parser);
    }

    ast_node_t *node = xcalloc(1, sizeof (ast_node_t));

    node->type = NODE_ROOT;
    node->root = root;

    return node;
}

void parser_free(struct parser *parser)
{
    free(parser);
}

static ast_node_t parser_parse_stmt(struct parser *parser)
{
    return parser_parse_expr(parser);
}

static ast_node_t parser_parse_expr(struct parser *parser)
{
    return parser_parse_binexp_additive(parser);
}

static ast_node_t parser_parse_binexp_inner(struct parser *parser, const char operator, ast_node_t left, ast_node_t right)
{
    ast_node_t binexpr = {
        .type = NODE_BINARY_EXPR,
        .binexpr = xcalloc(1, sizeof (ast_binexpr_t))
    };

    binexpr.binexpr->operator = (unsigned char) operator;

    binexpr.binexpr->left = xcalloc(1, sizeof left);
    memcpy(binexpr.binexpr->left, &left, sizeof left);

    binexpr.binexpr->right = xcalloc(1, sizeof right);
    memcpy(binexpr.binexpr->right, &right, sizeof right);
    return binexpr;
}

static ast_node_t parser_parse_binexp_multiplicative(struct parser *parser)
{
    ast_node_t left = parser_parse_primary_expr(parser);

    while (!parser_is_eof(parser) && parser_at(parser).type == T_BINARY_OPERATOR &&
           (parser_at(parser).value[0] == OP_TIMES || parser_at(parser).value[0] == OP_DIVIDE ||
            parser_at(parser).value[0] == OP_MODULUS))
    {
        const char operator = parser_ret_forward(parser).value[0];
        ast_node_t right = parser_parse_primary_expr(parser);
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
        const char operator = parser_ret_forward(parser).value[0];
        ast_node_t right = parser_parse_binexp_multiplicative(parser);
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

            ast_node_t identifier = {
                .type = NODE_IDENTIFIER,
                .identifier = xcalloc(1, sizeof(ast_identifier_t))
            };

            identifier.identifier->symbol = strdup(token.value);
            return identifier;
        }

        case T_INT_LIT:
        {
            parser_ret_forward(parser);
            assert(strspn(token.value, "0123456789") == strlen(token.value) && "Invalid integer");

            ast_node_t intlit = {
                .type = NODE_INT_LIT,
                .integer = xcalloc(1, sizeof (ast_intlit_t))
            };

            intlit.integer->intval = atoll(token.value);
            return intlit;
        }

        case T_STRING:
        {
            parser_ret_forward(parser);

            ast_node_t string = {
                .type = NODE_STRING,
                .string = xcalloc(1, sizeof (ast_string_t))
            };

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
            syntax_error("%lu:%lu: unexpected token '%s' (%s)", token.line_start,
                         token.column_start, token.value, lex_token_to_str(token.type));
    }
}

const char *ast_type_to_str(enum ast_node_type type)
{
    const char *translate[] = {
        [NODE_IDENTIFIER] = "IDENTIFIER",
        [NODE_BINARY_EXPR] = "BINARY_EXPR",
        [NODE_INT_LIT] = "INT_LIT",
        [NODE_ROOT] = "ROOT",
        [NODE_STRING] = "STRING"
    };

    size_t length = sizeof (translate) / sizeof (const char *);

    assert(type < length && "Invalid node type");
    return translate[type];
}

static void parser_ast_free_inner(ast_node_t *node)
{
    log_info("Freeing: %p", node);

    switch (node->type) {
        case NODE_ROOT:
            for (size_t i = 0; i < node->root->size; i++)
                parser_ast_free_inner(&node->root->nodes[i]);

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

        default:
            log_warn("parser_ast_free_inner(): AST type not recognized: %s (%d)",
                     ast_type_to_str(node->type), node->type);
    }
}

void parser_ast_free(ast_node_t *node)
{
    parser_ast_free_inner(node);
    free(node);
}

#ifndef _NDEBUG
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
            blaze_debug__print_ast_indent_string(inner_indent_level, "size: %lu,\n", node->root->size);
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
            blaze_debug__print_ast_indent_string(inner_indent_level, "operator: '%c',\n", node->binexpr->operator);
            blaze_debug__print_ast_indent_string(inner_indent_level, "left: ");
            blaze_debug__print_ast_internal(node->binexpr->left, inner_indent_level, false, false);
            printf(", \n");
            blaze_debug__print_ast_indent_string(inner_indent_level, "right: ");
            blaze_debug__print_ast_internal(node->binexpr->right, inner_indent_level, true, false);
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
    blaze_debug__print_ast_internal(node, 0, true, true);
}
#endif