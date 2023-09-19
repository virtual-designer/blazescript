/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_LEXER_H
#define BLAZESCRIPT_LEXER_H

#include <stdbool.h>
#include <stdlib.h>

enum lex_token_type
{
    T_UNKNOWN,
    T_SEMICOLON,
    T_INT_LIT,
    T_EOF,
    T_STRING,
    T_BINARY_OPERATOR,
    T_IDENTIFIER,
    T_PAREN_OPEN,
    T_PAREN_CLOSE,
    T_VAR,
    T_CONST,
    T_ASSIGNMENT,
    T_COMMA,
    T_FUNCTION,
    T_BLOCK_BRACE_OPEN,
    T_BLOCK_BRACE_CLOSE,
    T_PERIOD,
    T_SQUARE_BRACE_OPEN,
    T_SQUARE_BRACE_CLOSE,
    T_IMPORT,
    T_ARRAY,
    T_IF,
    T_ELSE,
    T_LOOP,
    T_AS
};

struct lex_token
{
    enum lex_token_type type;
    char *value;
    size_t line_start;
    size_t line_end;
    size_t column_start;
    size_t column_end;
};


struct lex
{
    size_t len;
    char *buf;
    char *filename;
    struct lex_token *tokens;
    size_t token_count;
    size_t current_line;
    size_t current_column;
    size_t index;
};

struct lex lex_init(char *filename, char *buf);
void lex_free(struct lex *lex);
bool lex_analyze(struct lex *lex);
struct lex_token *lex_get_tokens(struct lex *lex);
size_t lex_get_token_count(struct lex *lex);
const char *lex_token_to_str(enum lex_token_type type);
char *lex_get_filename(struct lex *lex);
void lex_set_contents(struct lex *lex, const char *new_buf);

#ifndef NDEBUG
void blaze_debug__lex_print(struct lex *lex);
#endif

#endif /* BLAZESCRIPT_LEXER_H */
