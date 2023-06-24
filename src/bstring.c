#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "bstring.h"
#include "xmalloc.h"

string_t _str(const char *s)
{
    return strdup(s);
}

void concat(string_t str, const char *s) 
{
    str = xrealloc(str, strlen(str) + strlen(s));
    strcat(str, s);
}

/* DEPRECATED: this function will be removed. */
void concat_c(string_t str, char c) 
{
    size_t len = strlen(str);
    str = xrealloc(str, len + 1);
    str[len] = c;
}

void concat_c_safe(string_t str, size_t *len, char c) 
{
    str = xrealloc(str, ++(*len));
    str[(*len) - 1] = c;
}

void strfree(string_t s) 
{
    if (s != NULL)
    {
        free(s);
        s = NULL;
    }
}
