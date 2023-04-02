#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "string.h"
#include "xmalloc.h"

string_t _str(const char *s)
{
    char *dup = xmalloc(strlen(s) + 1);
    strcpy(dup, s);
    return (string_t) dup;
}

void concat(string_t str, const char *s) 
{
    str = xrealloc(str, strlen(str) + strlen(s));
    strcat(str, s);
}

void concat_c(string_t str, char c) 
{
    size_t len = strlen(str);
    str = xrealloc(str, len + 1);
    str[len] = c;
}

void strfree(string_t s) 
{
    if (s != NULL)
    {
        free(s);
        s = NULL;
    }
}
