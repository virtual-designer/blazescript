#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#include "xmalloc.h"
#include "lexer.h"
#include "string.h"
#include "blaze.h"

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

    sprintf(fmt_processed, COLOR("1", "%s:%lu: ") COLOR("1;31", "syntax error") ": %s\n", config.currentfile, line, fmt);
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
    if (strcmp(s, "function") == 0)
        return T_FUNCTION;
    if (strcmp(s, "if") == 0)
        return T_IF;
    if (strcmp(s, "else") == 0)
        return T_ELSE;

    return T_SKIPPABLE;
}

bool lex_token_array_shift(lex_t *array, lex_token_t *token)
{
    if (token) 
    {
        if (array->size == 0)
            return false;

        *token = array->tokens[0];
    }

    for (size_t i = 0; (i + 1) < array->size; i++) 
    {
        array->tokens[i] = array->tokens[i + 1];
    }

    array->size--;
    return true;
}

static void line_update_set(size_t *lineptr, size_t newvalue)
{
    *lineptr = newvalue;
}

static void line_update(size_t *lineptr, size_t incr)
{
    *lineptr += incr;
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
        lex_token_t token = { .value = strdup(char_buf), .type = T_SKIPPABLE, .line = line };
        bool multi_char = false, string_parsing = false;

        switch (code[i]) 
        {
            case '+':
            case '-':
            case '*':
            case '%':
                token.type = T_BINARY_OPERATOR;
            break;

            case '/':
                if ((i + 1) < len && code[i] == '/' && code[i + 1] == '/') 
                {
                    i += 2;

                    while (i < len)
                    {
                        i++;

                        if (code[i] == '\r' || code[i] == '\n')
                        {
                            i++;
                            break;
                        }
                    }

                    line_update(&line, 1);
                    continue;
                }
                else if ((i + 1) < len && code[i] == '/' && code[i + 1] == '*')
                {
                    bool is_terminated = false;

                    i += 2;

                    while ((i + 1) < len)
                    {
                        if (code[i] == '*' && code[i + 1] == '/')
                        {
                            i += 2;
                            is_terminated = true;
                            break;
                        }
                        
                        if (code[i + 1] == '\n' || code[i + 1] == '\r')
                            line_update(&line, 1);
                        
                        i++;
                    }

                    if (!is_terminated)
                        lex_error(true, "Unterminated comment, missing `*/` to close the comment block");

                    continue;
                } 

                token.type = T_BINARY_OPERATOR;
            break;

            case '!':
                token.type = T_UNARY_OPERATOR;
            break;

            case '(':
                token.type = T_PAREN_OPEN;
            break;

            case ')':
                token.type = T_PAREN_CLOSE;
            break;

            case '{':
                token.type = T_BLOCK_BRACE_OPEN;
            break;

            case '}':
                token.type = T_BLOCK_BRACE_CLOSE;
            break;
            
            case '[':
                token.type = T_ARRAY_BRACKET_OPEN;
            break;

            case ']':
                token.type = T_ARRAY_BRACKET_CLOSE;
            break;

            case ';':
                token.type = T_SEMICOLON;
            break;

            case ':':
                token.type = T_COLON;
            break;

            case ',':
                token.type = T_COMMA;
            break;

            case '.':
                token.type = T_DOT;
            break;

            default:
            {
                multi_char = true;

                if (string_parsing)
                    lex_error(true, "Error parsing string");

                if (code[i] == '\'' || code[i] == '"')
                {
                    string_parsing = true;

                    char quote = code[i] == '\'' ? '\'' : '"';
                    string_t str = _str("");
                    size_t length = 0;

                    i++;

                    while (i < len && code[i] != quote)
                    {
                        if ((i + 1) < len && code[i] == '\\' && (code[i + 1] == '\'' || code[i + 1] == '"'))
                        {
                            concat_c_safe(str, &length, code[i + 1]);
                            i += 2;
                            continue;
                        }

                        if (code[i] == '\n' || code[i] == '\r')
                            line_update(&line, 1);
                        
                        concat_c_safe(str, &length, code[i]);
                        i++;
                    }

                    if (code[i] != quote)
                    {
                        line_update(&line, 1);
                        lex_error(true, "Unterminated string, expected ending %s quote `%c`", quote == '"' ? "double" : "single", quote);
                    }

                    concat_c_safe(str, &length, '\0');
                    i++;

                    free(token.value);

                    token.type = T_STRING;
                    token.value = str;
                    string_parsing = false;
                }
                else if (lex_is_skippable(code[i])) 
                {
                    if (code[i] == '\n' || code[i] == '\r')
                        line_update(&line, 1);
                    
                    i++;
                    continue;
                }
                else if ((i + 1) < len && ((code[i] == '&' && code[i + 1] == '&') || (code[i] == '|' && code[i + 1] == '|')))
                {
                    token.type = T_BINARY_OPERATOR;
                    token.value = strdup(code[i] == '&' ? "&&" : "||");
                    i += 2;
                }
                else if ((i + 1) < len && (code[i] == '=' && code[i + 1] == '='))
                {
                    token.type = T_BINARY_OPERATOR;
                    token.value = strdup("==");
                    i += 2;
                }
                else if (i < len && code[i] == '=')
                {
                    token.type = T_ASSIGNMENT;
                    token.value = strdup("=");
                    i++;
                }
                else if (isdigit(code[i])) 
                {
                    char *number = strdup(""); 
                    size_t length = 0;

                    while (i < len && (isdigit(code[i]) || code[i] == '.'))
                    {
                        number = xrealloc(number, ++length);
                        number[length - 1] = code[i];
                        i++;
                    }

                    number = xrealloc(number, ++length);
                    number[length - 1] = '\0';

                    free(token.value);

                    token.type = T_NUMBER;
                    token.value = number;
                }
                else if (isalpha(code[i]) != 0 || code[i] == '_')
                {
                    char *identifier = strdup(""); 
                    size_t length = 0;

                    while (i < len && (isalpha(code[i]) || isdigit(code[i]) || code[i] == '_'))
                    {
                        identifier = xrealloc(identifier, ++length);
                        identifier[length - 1] = code[i];
                        i++;
                    }

                    identifier = xrealloc(identifier, ++length);
                    identifier[length - 1] = '\0';
                    
                    free(token.value);
                    token.value = identifier;

                    lex_tokentype_t keyword_token_type = lex_keyword(identifier);

                    if (keyword_token_type != T_SKIPPABLE) 
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
                    string_t identifier = _str(""); 
                    size_t length = 0;

                    while (i < len)
                    {
                        if (isspace(code[i]))
                        {
                            break;
                        }

                        concat_c_safe(identifier, &length, code[i]);
                        i++;
                    }

                    concat_c_safe(identifier, &length, '\0');

                    lex_error(true, "Unexpected token '%s' found", identifier);
                }
            }
            
            break;
        }

        if (token.type != T_SKIPPABLE)
        {
            lex_token_array_push(array, token);
        }
        
        if (!multi_char) 
            i++;
    } 
    
    lex_token_array_push(array, (lex_token_t) {
        .type = T_EOF,
        .value = NULL,
        .line = line
    });
}

void __debug_lex_print_token_array(lex_t *array)
{
    puts("Debug -------------");

    for (size_t i = 0; i < array->size; i++)
    {
        printf("[%lu] [Line: %lu] - %d - '%s'\n", i, array->tokens[i].line, array->tokens[i].type, array->tokens[i].value);
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

char *lex_token_stringify(lex_token_t token, bool quotes)
{
    string_t str;
    
    switch (token.type) 
    {
        case T_ASSIGNMENT:
            return "Assignment operator";

        case T_BINARY_OPERATOR:
            switch (token.value[0])
            {
                case '+':
                    return "Binary addition operator";
                case '-':
                    return "Binary subtraction operator";
                case '*':
                    return "Binary multiplication operator";
                case '/':
                    return "Binary division operator";
                case '%':
                    return "Binary modulus operator";

                default:
                    goto token_return_as_is;
            }

        case T_CONST:
            return "const";

        case T_EOF:
            return "End of file";

        case T_IDENTIFIER:
            return "Identifier";

        case T_NUMBER:
            return "Number";

        case T_SKIPPABLE:
            return "Skippable";

        case T_STRING:
            return "String";

        case T_VAR:
            return "var";

        case T_FUNCTION:
            return "function";

        default:
token_return_as_is:
            str = _str("\"");
            concat(str, token.value);
            concat_c(str, '"');
            return str;
    }
}
