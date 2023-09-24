#include "stdio.h"
#include "common.h"

void libblaze_putchar(char c)
{
    BLAZE(putchar)(c);
}

void libblaze_puts(char *s)
{
    BLAZE(puts)(s);
}

void libblaze_putstr(char *s)
{
    BLAZE(putstr)(s);
}
