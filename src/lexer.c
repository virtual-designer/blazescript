/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include "lexer.h"
#include "utils.h"
#include "alloca.h"

#define LEX_ERROR_ARGS(lex, fmt, ...) \
    SYNTAX_ERROR_LINE_ARGS(lex->filename, lex->current_line, lex->current_column, fmt, __VA_ARGS__)

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

struct keyword
{
    const char *identifier;
    enum lex_token_type token_type;
};

struct char_to_token_map
{
    const char token_char;
    enum lex_token_type token_type;
};

static const struct char_to_token_map chartokens[] = {
    { ';', T_SEMICOLON },
    { '(', T_PAREN_OPEN },
    { ')', T_PAREN_CLOSE },
    { '+', T_BINARY_OPERATOR },
    { '-', T_BINARY_OPERATOR },
    { '/', T_BINARY_OPERATOR },
    { '*', T_BINARY_OPERATOR },
    { '%', T_BINARY_OPERATOR },
    { '=', T_ASSIGNMENT },
    { ',', T_COMMA },
};

static const struct keyword keywords[] = {
    { "var", T_VAR },
    { "const", T_CONST },
};

struct lex *lex_init(char *filename, char *buf)
{
    struct lex *lex = xmalloc(sizeof (struct lex));

    lex->buf = strdup(buf);
    lex->len = strlen(lex->buf);
    lex->current_line = 1;
    lex->current_column = 1;
    lex->token_count = 0;
    lex->index = 0;
    lex->tokens = NULL;
    lex->filename = strdup(filename);

    return lex;
}

void lex_set_contents(struct lex *lex, const char *new_buf)
{
    free(lex->buf);
    lex->buf = strdup(new_buf);
}

void lex_free(struct lex *lex)
{
    for (size_t i = 0; i < lex->token_count; i++)
        free(lex->tokens[i].value);

    free(lex->tokens);
    free(lex->buf);
    free(lex->filename);
    free(lex);
}

static void lex_tokens_array_push(struct lex *lex, struct lex_token token)
{
    lex->tokens = xrealloc(lex->tokens, (++lex->token_count) * sizeof (struct lex_token));
    lex->tokens[lex->token_count - 1] = token;
}

static inline void lex_token_push_default(struct lex *lex, enum lex_token_type type, char *value)
{
    lex_tokens_array_push(lex, (struct lex_token) {
        .type = type,
        .value = value,
        .line_start = lex->current_line,
        .line_end = lex->current_line,
        .column_start = lex->current_column,
        .column_end = lex->current_column,
    });
}

static inline void lex_token_push_nocol(struct lex *lex, enum lex_token_type type, char *value)
{
    lex_tokens_array_push(lex, (struct lex_token) {
        .type = type,
        .value = value,
        .line_start = lex->current_line,
        .line_end = lex->current_line,
        .column_start = 1,
        .column_end = 1,
    });
}

static inline void lex_token_push_noline(struct lex *lex, enum lex_token_type type, char *value)
{
    lex_tokens_array_push(lex, (struct lex_token) {
        .type = type,
        .value = value,
        .line_start = 0,
        .line_end = 0,
        .column_start = lex->current_column,
        .column_end = lex->current_column,
    });
}

static inline bool lex_has_value(struct lex *lex)
{
    return lex->index < lex->len;
}

static inline char lex_char_forward(struct lex *lex)
{
    assert(lex_has_value(lex) && "No character is remaining to return");
    char c = lex->buf[lex->index++];

    if (c == '\r' || c == '\n')
    {
        lex->current_line++;
        lex->current_column = 1;
    }
    else
        lex->current_column++;

    return c;
}


static inline char lex_char(struct lex *lex)
{
    assert(lex_has_value(lex) && "No character is remaining to return");
    return lex->buf[lex->index];
}

static bool lex_string(struct lex *lex)
{
    size_t column_start = lex->current_column;
    char *strbuf = NULL;
    size_t strbuf_size = 0;
    char quote = lex_char_forward(lex);

    while (lex_has_value(lex) && lex_char(lex) != quote)
    {
        strbuf = xrealloc(strbuf, ++strbuf_size);
        strbuf[strbuf_size - 1] = lex_char_forward(lex);
    }

    if (!lex_has_value(lex) || lex_char(lex) != quote)
    {
        LEX_ERROR_ARGS(lex, "unterminated string: expected '%c'", quote);
        return false;
    }

    lex_char_forward(lex);

    strbuf = xrealloc(strbuf, ++strbuf_size);
    strbuf[strbuf_size - 1] = 0;

    lex_tokens_array_push(lex, (struct lex_token) {
        .type = T_STRING,
        .value = strbuf,
        .line_start = lex->current_line,
        .line_end = lex->current_line,
        .column_start = column_start,
        .column_end = lex->current_column,
    });

    return true;
}

static void lex_number(struct lex *lex)
{
    size_t column_start = lex->current_column;
    char *numbuf = NULL;
    size_t numbuf_size = 0;

    while (lex_has_value(lex) && isdigit(lex_char(lex)))
    {
        numbuf = xrealloc(numbuf, ++numbuf_size);
        numbuf[numbuf_size - 1] = lex_char_forward(lex);
    }

    numbuf = xrealloc(numbuf, ++numbuf_size);
    numbuf[numbuf_size - 1] = 0;

    lex_tokens_array_push(lex, (struct lex_token) {
        .type = T_INT_LIT,
        .value = numbuf,
        .line_start = lex->current_line,
        .line_end = lex->current_line,
        .column_start = column_start,
        .column_end = lex->current_column,
    });
}

static enum lex_token_type convert_keyword_to_token(const char *keyword)
{
    for (size_t i = 0; i < (sizeof (keywords) / sizeof keywords[0]); i++)
    {
        if (strcmp(keywords[i].identifier, keyword) == 0)
            return keywords[i].token_type;
    }

    return T_UNKNOWN;
}

static void lex_identifier_or_keyword(struct lex *lex)
{
    size_t column_start = lex->current_column;
    char *identifier = NULL;
    size_t size = 0;

    while (lex_has_value(lex) && isalnum(lex_char(lex)))
    {
        identifier = xrealloc(identifier, ++size);
        identifier[size - 1] = lex_char_forward(lex);
    }

    identifier = xrealloc(identifier, ++size);
    identifier[size - 1] = 0;

    enum lex_token_type keyword_token_type = convert_keyword_to_token(identifier);

    lex_tokens_array_push(lex, (struct lex_token) {
        .type = keyword_token_type == T_UNKNOWN ? T_IDENTIFIER : keyword_token_type,
        .value = identifier,
        .line_start = lex->current_line,
        .line_end = lex->current_line,
        .column_start = column_start,
        .column_end = lex->current_column,
    });
}
static void lex_push_char_token(struct lex *lex, enum lex_token_type type)
{
    size_t column_start = lex->current_column;

    lex_tokens_array_push(lex, (struct lex_token) {
        .type = type,
        .value = ctos(lex_char_forward(lex)),
        .line_start = lex->current_line,
        .line_end = lex->current_line,
        .column_start = column_start,
        .column_end = lex->current_column,
    });
}

bool lex_analyze(struct lex *lex)
{
    while (lex_has_value(lex))
    {
        char c = lex_char(lex);

        if (isspace(c))
        {
            lex_char_forward(lex);
            continue;
        }

        for (size_t i = 0; i < (sizeof (chartokens) / sizeof (chartokens[0])); i++)
        {
            if (c == chartokens[i].token_char)
            {
                lex_push_char_token(lex, chartokens[i].token_type);
                goto loop_end;
            }
        }

        if (c == '"' || c == '\'')
            lex_string(lex);
        else if (isdigit(c))
            lex_number(lex);
        else if (isalpha(c))
            lex_identifier_or_keyword(lex);
        else
        {
            syntax_error("unknown token '%c'", lex_char(lex));
            return false;
        }

        loop_end:
            NULL;
    }

    lex_token_push_default(lex, T_EOF, strdup("[EOF]"));
    return true;
}

struct lex_token *lex_get_tokens(struct lex *lex)
{
    return lex->tokens;
}

size_t lex_get_token_count(struct lex *lex)
{
    return lex->token_count;
}

char *lex_get_filename(struct lex *lex)
{
    return lex->filename;
}

const char *lex_token_to_str(enum lex_token_type type)
{
    const char *translate[] = {
        [T_IDENTIFIER] = "T_IDENTIFIER",
        [T_STRING] = "T_STRING",
        [T_BINARY_OPERATOR] = "T_BINARY_OPERATOR",
        [T_INT_LIT] = "T_INT_LIT",
        [T_EOF] = "T_EOF",
        [T_SEMICOLON] = "T_SEMICOLON",
        [T_PAREN_OPEN] = "T_PAREN_OPEN",
        [T_PAREN_CLOSE] = "T_PAREN_CLOSE",
        [T_UNKNOWN] = "T_UNKNOWN",
        [T_ASSIGNMENT] = "T_ASSIGNMENT",
        [T_VAR] = "T_VAR",
        [T_CONST] = "T_CONST",
        [T_COMMA] = "T_COMMA",
    };

    size_t length = sizeof (translate) / sizeof (const char *);

    assert(type < length && "Invalid token type");
    return translate[type];
}

#ifndef NDEBUG
void blaze_debug__lex_print(struct lex *lex)
{
    for (size_t i = 0; i < lex->token_count; i++)
    {
        printf("[%lu] Token { type: %s(%d), value: \"%s\", line: [%lu-%lu], column: [%lu-%lu] }\n",
               i, lex_token_to_str(lex->tokens[i].type), lex->tokens[i].type, lex->tokens[i].value, lex->tokens[i].line_start,
               lex->tokens[i].line_end, lex->tokens[i].column_start, lex->tokens[i].column_end);
    }
}
#endif