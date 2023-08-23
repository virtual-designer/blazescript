/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_LEXER_H
#define BLAZESCRIPT_LEXER_H

enum lex_token_type
{
    T_SEMICOLON,
    T_NUM_LIT,
    T_EOF
};

struct lex_token
{
    enum lex_token_type type;
    char *value;
    size_t line_start;
    size_t line_end;
};

struct lex *lex_init(const char *buf);
void lex_free(struct lex *lex);
void lex_analyze(struct lex *lex);

#ifndef _NDEBUG
void blaze_debug__lex_print(struct lex *lex);
#endif

#endif /* BLAZESCRIPT_LEXER_H */
