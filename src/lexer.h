/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_LEXER_H
#define BLAZESCRIPT_LEXER_H

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
    T_COMMA
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

struct lex *lex_init(char *filename, char *buf);
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
