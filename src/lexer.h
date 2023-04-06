#ifndef __LEXER_H__
#define __LEXER_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#define LEX_INIT { .size = 0, .tokens = NULL }

typedef enum 
{
    T_VAR,
    T_CONST,
    T_BINARY_OPERATOR,
    T_IDENTIFIER,
    T_NUMBER,
    T_ASSIGNMENT,
    T_PAREN_OPEN,
    T_PAREN_CLOSE,
    T_SKIPPABLE,
    T_EOF,
    T_SEMICOLON,
    T_COMMA,
    T_BLOCK_BRACE_OPEN,
    T_BLOCK_BRACE_CLOSE,
    T_COLON
} lex_tokentype_t;

typedef struct 
{
    char *value;
    lex_tokentype_t type;
    size_t line;
} lex_token_t;

typedef struct 
{
    lex_token_t *tokens;
    size_t size;
} lex_t;

void lex_tokenize(lex_t *array, char *restrict code);
void lex_free(lex_t *array);
bool lex_token_array_shift(lex_t *array, lex_token_t *token);

#ifdef _DEBUG
void __debug_lex_print_token_array(lex_t *array);
#endif

#endif
