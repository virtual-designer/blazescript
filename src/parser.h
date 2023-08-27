/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_PARSER_H
#define BLAZESCRIPT_PARSER_H

#include "ast.h"

struct parser *parser_init();
struct parser *parser_init_from_lex(struct lex *lex);
void parser_free(struct parser *parser);
ast_node_t *parser_create_ast_node(struct parser *parser);
void parser_ast_free(ast_node_t *node);
void parser_set_tokens(struct parser *parser, struct lex_token *tokens, size_t count);
void parser_set_filename(struct parser *parser, const char *filename);
ast_node_t *parser_ast_deep_copy(ast_node_t *node);

#ifndef NDEBUG
void blaze_debug__print_ast(ast_node_t *node);
#endif

#endif /* BLAZESCRIPT_PARSER_H */
