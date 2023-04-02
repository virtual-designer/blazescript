#ifndef __PARSER_H__
#define __PARSER_H__

#include "ast.h"

ast_stmt parser_create_ast(char *code);
ast_stmt parser_parse_expr();

#ifdef _DEBUG
void __debug_parser_print_ast_stmt(ast_stmt *prog);
#endif

#endif
