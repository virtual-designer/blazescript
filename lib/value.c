#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datatype.h"
#include "lib.h"
#include "x86_64/stdio.h"

val_t *libblaze_val_create(val_type_t type)
{
    val_t *val = xmalloc(sizeof (val_t));
    val->type = type;
    return val;
}

__attribute__((used)) void libblaze_val_set_strval(val_t *val, char *string)
{
    val->strval = strdup(string);
}

__attribute__((used)) val_t *libblaze_val_create_strval(char *string)
{
    val_t *val = libblaze_val_create(VAL_STRING);
    val->strval = strdup(string);
    return val;
}

__attribute__((used)) val_t *libblaze_val_create_intval(uint64_t value)
{
    val_t *val = libblaze_val_create(VAL_INTEGER);
    val->intval = value;
    return val;
}

__attribute__((used)) void libblaze_val_alloc_str(val_t *val, size_t size)
{
    val->strval = xmalloc(size);
}

__attribute__((used)) void libblaze_fn_println(uint64_t argc, ...)
{
    va_list args;
    va_start(args, argc);

    for (uint64_t i = 0; i < argc; i++)
    {
        val_t *val = va_arg(args, val_t *);

        if (val->type == VAL_STRING)
            x86_64_libblaze_putstr(val->strval);
        else
            print_val(val);

        if (i != argc - 1)
            x86_64_libblaze_putchar(' ');
    }

    x86_64_libblaze_putchar('\n');
    va_end(args);
}

__attribute__((used)) void libblaze_val_free(val_t *val)
{
    assert(val != NULL);

    if (val->type == VAL_STRING)
        free(val->strval);

    free(val);
}
