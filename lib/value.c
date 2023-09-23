#include "alloca.h"
#include "datatype.h"
#include "lib.h"
#include "utils.h"
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
        libblaze_fatal_error("could not allocate memory: %s", strerror(errno));

    return ptr;
}

val_t *libblaze_val_create(val_type_t type)
{
    val_t *val = xmalloc(sizeof (val_t));
    val->type = type;
    return val;
}

__attribute_used__ void libblaze_val_set_strval(val_t *val, char *string)
{
    val->strval = strdup(string);
}

__attribute_used__ val_t *libblaze_val_create_strval(char *string)
{
    val_t *val = libblaze_val_create(VAL_STRING);
    val->strval = strdup(string);
    return val;
}

__attribute_used__ void libblaze_val_alloc_str(val_t *val, size_t size)
{
    val->strval = xmalloc(size);
}

__attribute_used__ void libblaze_fn_println(uint64_t argc, ...)
{
    va_list args;
    va_start(args, argc);

    for (uint64_t i = 0; i < argc; i++)
    {
        val_t *val = va_arg(args, val_t *);

        if (val->type == VAL_STRING)
            printf("%s", val->strval);
        else
            print_val(val);

        if (i != argc - 1)
            putchar(' ');
    }

    putchar('\n');
    va_end(args);
}

__attribute_used__ void libblaze_val_free(val_t *val)
{
    assert(val != NULL);

    if (val->type == VAL_STRING)
        free(val->strval);

    free(val);
}