#ifndef __BLAZE_STRING_H__
#define __BLAZE_STRING_H__

#include <string.h>
#include <stdbool.h>

#define STREQ(str1, str2) ((bool) (strcmp(str1, str2) == 0))

typedef char * string_t;

string_t _str(const char *s);
void concat(string_t str, const char *s);
void concat_c(string_t str, char c);
void strfree(string_t s);
void concat_c_safe(string_t str, size_t *len, char c);

#endif
