/*
 * Created by rakinar2 on 8/22/23.
 */

#include "alloca.h"
#include "utils.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct alloca_tbl_t global_alloca_tbl;

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
        fatal_error("could not allocate memory: %s", strerror(errno));

    return ptr;
}

void *xcalloc(size_t n, size_t size)
{
    void *ptr = calloc(n, size);

    if (ptr == NULL)
        fatal_error("could not allocate memory: %s", strerror(errno));

    return ptr;
}

void *xrealloc(void *old_ptr, size_t new_size)
{
    void *ptr = realloc(old_ptr, new_size);

    if (ptr == NULL)
        fatal_error("could not allocate memory: %s", strerror(errno));

    return ptr;
}

struct alloca_tbl_t alloca_tbl_init()
{
    struct alloca_tbl_t tbl;
    tbl.count = 0;
    tbl.ptrs = NULL;
    return tbl;
}

void alloca_tbl_free(struct alloca_tbl_t *tbl)
{
    free(tbl->ptrs);
}

size_t alloca_tbl_push_ptr(struct alloca_tbl_t *tbl, void *ptr)
{
    tbl->ptrs = xrealloc(tbl->ptrs, sizeof(void *) * (++tbl->count));
    tbl->ptrs[tbl->count - 1] = ptr;
    return tbl->count - 1;
}

size_t blaze_alloca_tbl_push_ptr(void *ptr)
{
    return alloca_tbl_push_ptr(&global_alloca_tbl, ptr);
}

ssize_t alloca_tbl_remove_ptr(struct alloca_tbl_t *tbl, void *ptr)
{
    for (ssize_t i = 0; i < tbl->count; i++)
    {
        if (tbl->ptrs[i] == ptr)
        {
            tbl->ptrs[i] = NULL;
            return i;
        }
    }

    return -1;
}

void blaze_alloca_tbl_init()
{
    global_alloca_tbl = alloca_tbl_init();
}

void blaze_alloca_tbl_free()
{
    for (size_t i = 0; i < global_alloca_tbl.count; i++)
    {
        if (global_alloca_tbl.ptrs[i] != NULL)
        {
            free(global_alloca_tbl.ptrs[i]);
            global_alloca_tbl.ptrs[i] = NULL;
        }
    }

    alloca_tbl_free(&global_alloca_tbl);
}

void *blaze_malloc(size_t size)
{
    void *ptr = xmalloc(size);
    alloca_tbl_push_ptr(&global_alloca_tbl, ptr);
    return ptr;
}

void *blaze_realloc(void *ptr, size_t new_size)
{
    void *new_ptr = xrealloc(ptr, new_size);

    if (new_ptr != ptr)
    {
        if (ptr != NULL)
            alloca_tbl_remove_ptr(&global_alloca_tbl, ptr);

        alloca_tbl_push_ptr(&global_alloca_tbl, new_ptr);
    }

    return new_ptr;
}

void *blaze_calloc(size_t n, size_t size)
{
    void *ptr = xcalloc(n, size);
    alloca_tbl_push_ptr(&global_alloca_tbl, ptr);
    return ptr;
}

void *blaze_strdup(const char *str)
{
    char *ptr = strdup(str);
    alloca_tbl_push_ptr(&global_alloca_tbl, ptr);
    return ptr;
}

bool blaze_free(void *ptr)
{
    if (ptr == NULL)
    {
        return false;
    }

    for (ssize_t i = 0; i < global_alloca_tbl.count; i++)
    {
        if (global_alloca_tbl.ptrs[i] != NULL && global_alloca_tbl.ptrs[i] == ptr)
        {
            free(global_alloca_tbl.ptrs[i]);
            global_alloca_tbl.ptrs[i] = NULL;
            return true;
        }
    }

    return false;
}