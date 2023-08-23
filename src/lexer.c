/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "utils.h"
#include "alloca.h"

struct lex
{
    size_t len;
    const char *buf;
    struct lex_token *tokens;
    size_t token_count;
    size_t current_line;
};

struct lex *lex_init(const char *buf)
{
    struct lex *lex = xmalloc(sizeof (struct lex));

    lex->buf = buf;
    lex->len = strlen(buf);
    lex->current_line = 1;
    lex->token_count = 0;
    lex->tokens = NULL;

    return lex;
}

void lex_free(struct lex *lex)
{
    for (size_t i = 0; i < lex->token_count; i++)
        free(lex->tokens[i].value);

    free(lex->tokens);
    free(lex);
}

void lex_tokens_array_push(struct lex *lex, struct lex_token token)
{
    lex->tokens = xrealloc(lex->tokens, (++lex->token_count) * sizeof (struct lex_token));
    lex->tokens[lex->token_count - 1] = token;
}

void lex_analyze(struct lex *lex)
{
    size_t i = 0;
    size_t len = lex->len;
    const char *buf = lex->buf;

    while (i < len)
    {
        switch (buf[i])
        {
            default:
                if (isspace(buf[i]))
                {
                    if (buf[i] == '\r' || buf[i] == '\n')
                        lex->current_line++;

                    i++;
                    continue;
                }
                else if (isdigit(buf[i]))
                {
                    char *numbuf = NULL;
                    size_t numbuf_size = 0;

                    while (i < len && isdigit(buf[i]))
                    {
                        numbuf = xrealloc(numbuf, ++numbuf_size);
                        numbuf[numbuf_size - 1] = buf[i];
                        i++;
                    }

                    i--;

                    numbuf = xrealloc(numbuf, ++numbuf_size);
                    numbuf[numbuf_size - 1] = 0;

                    lex_tokens_array_push(lex, (struct lex_token) {
                        .type = T_NUM_LIT,
                        .value = numbuf,
                        .line_start = lex->current_line,
                        .line_end = lex->current_line
                    });
                }
                else
                    syntax_error("unknown token '%c'", buf[i]);
        }

        i++;
    }

    lex_tokens_array_push(lex, (struct lex_token) {
        .type = T_EOF,
        .value = strdup("[EOF]"),
        .line_start = lex->current_line,
        .line_end = lex->current_line
    });
}

#ifndef _NDEBUG
void blaze_debug__lex_print(struct lex *lex)
{
    for (size_t i = 0; i < lex->token_count; i++)
    {
        printf("[%lu] Token { type: %d, value: \"%s\", start_line: %lu, end_line: %lu }\n",
               i, lex->tokens[i].type, lex->tokens[i].value, lex->tokens[i].line_start, lex->tokens[i].line_end);
    }
}
#endif