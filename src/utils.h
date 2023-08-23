/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_UTILS_H
#define BLAZESCRIPT_UTILS_H

void fatal_error(const char *fmt, ...) __attribute__((noreturn));
void syntax_error(const char *fmt, ...) __attribute__((noreturn));
char *ctos(char c);

#endif /* BLAZESCRIPT_UTILS_H */
