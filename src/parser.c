/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
#include "alloca.h"

#include "ast.h"

struct parser
{
    size_t index;
    size_t token_count;
    struct lex_token *tokens;
};

ast_node_t parser_parse_stmt(struct parser *parser);

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

ast_root_t parser_create_ast_node(struct parser *parser)
{
    ast_root_t root = {
        .size = 0,
        .nodes = NULL
    };

    while (!parser_is_eof(parser)) {
        root.nodes = xrealloc(root.nodes, (++root.size) * sizeof (ast_node_t));
        root.nodes[root.size - 1] = parser_parse_stmt(parser);
    }

    return root;
}

void parser_free(struct parser *parser)
{
    free(parser);
}

ast_node_t parser_parse_stmt(struct parser *parser)
{
    // TODO
}