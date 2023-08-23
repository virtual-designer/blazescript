/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_PARSER_H
#define BLAZESCRIPT_PARSER_H

#include "ast.h"

struct parser *parser_init();
struct parser *parser_init_from_lex(struct lex *lex);
void parser_free(struct parser *parser);
ast_root_t parser_create_ast_node(struct parser *parser);

#endif /* BLAZESCRIPT_PARSER_H */
