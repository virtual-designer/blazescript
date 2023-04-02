#ifndef __BLAZE_STRING_H__
#define __BLAZE_STRING_H__

#include "lexer.h"

typedef char * string_t;

string_t _str(const char *s);
void concat(string_t str, const char *s);
void concat_c(string_t str, char c);
void strfree(string_t s);

#endif
