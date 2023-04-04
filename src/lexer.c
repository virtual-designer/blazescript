#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "xmalloc.h"
#include "lexer.h"
#include "string.h"

static size_t line = 1;

static void lex_token_array_resize(lex_t *array, size_t elements) 
{
    array->tokens = xrealloc(array->tokens, sizeof (lex_token_t) * (array->size + elements));
    array->size += elements;
}

static void lex_token_array_push(lex_t *array, lex_token_t token)
{
    lex_token_array_resize(array, 1);
    array->tokens[array->size - 1] = token;
}

static bool lex_is_skippable(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void lex_error(bool should_exit, const char *fmt, ...)
{
    va_list args;
    char fmt_processed[strlen(fmt) + 50];
    va_start(args, fmt);

    sprintf(fmt_processed, "Lexer error: %s at line %lu\n", fmt, line);
    vfprintf(stderr, fmt_processed, args);

    va_end(args);

    if (should_exit)
        exit(EXIT_FAILURE);
}

static lex_tokentype_t lex_keyword(char *s)
{
    if (strcmp(s, "var") == 0)
        return T_VAR;
    if (strcmp(s, "const") == 0)
        return T_CONST;

    return T_DEFAULT;
}

bool lex_token_array_shift(lex_t *array, lex_token_t *token)
{
    if (token) 
    {
        if (array->size == 0)
            return false;

        *token = array->tokens[0];
    }

    for (size_t i = 0; i < array->size; i++) 
    {
        array->tokens[i] = array->tokens[i + 1];
    }

    array->size--;
    return true;
}

void lex_tokenize(lex_t *array, char *code)
{
    array->tokens = NULL;
    size_t len = strlen(code);
    size_t i = 0;

    while (i < len)
    {
        char char_buf[2];
        sprintf(char_buf, "%c", code[i]);
        lex_token_t token = { .value = strdup(char_buf), .type = T_DEFAULT };
        bool multi_char = false;

        switch (code[i]) 
        {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
                token.type = T_BINARY_OPERATOR;
            break;

            case '=':
                token.type = T_ASSIGNMENT;
            break;

            case '(':
                token.type = T_PAREN_OPEN;
            break;

            case ')':
                token.type = T_PAREN_CLOSE;
            break;

            case ';':
                token.type = T_SEMICOLON;
            break;

            default:
            {
                multi_char = true;

                if (isdigit(code[i])) 
                {
                    string_t number = _str(""); 

                    while (i < len && (isdigit(code[i]) || code[i] == '.'))
                    {
                        concat_c(number, code[i]);
                        i++;
                    }

                    free(token.value);

                    token.type = T_NUMBER;
                    token.value = number;
                }
                else if (lex_is_skippable(code[i])) 
                {
                    if (code[i] == '\n' || code[i] == '\r')
                        line++;
                    
                    i++;
                    token.type = T_DEFAULT;
                }
                else if (isalpha(code[i]) != 0)
                {
                    string_t identifier = _str(""); 

                    while (i < len && (isalpha(code[i]) || isdigit(code[i]) || code[i] == '_'))
                    {
                        concat_c(identifier, code[i]);
                        i++;
                    }
                    
                    free(token.value);
                    token.value = identifier;

                    lex_tokentype_t keyword_token_type = lex_keyword(identifier);

                    if (keyword_token_type != T_DEFAULT) 
                    {
                        token.type = keyword_token_type;
                    }
                    else 
                    {
                        token.type = T_IDENTIFIER;
                    }
                }
                else 
                {
                    lex_error(true, "Unexpected character found");
                }
            }
            
            break;
        }

        if (token.type != T_DEFAULT)
        {
            // printf("Pushed: [%lu] %d\n", i, token.type);
            lex_token_array_push(array, token);
        }
        
        if (!multi_char) 
            i++;
    } 
    
    lex_token_array_push(array, (lex_token_t) {
        .type = T_EOF,
        .value = NULL
    });
}

void __debug_lex_print_token_array(lex_t *array)
{
    puts("Debug -------------");

    for (size_t i = 0; i < array->size; i++)
    {
        printf("[%lu] - %d - '%s'\n", i, array->tokens[i].type, array->tokens[i].value);
    }
}

void lex_free(lex_t *array)
{
    if (array->tokens != NULL)
    {
        free(array->tokens);
        array->tokens = NULL;
    }

    array->size = 0;
}
