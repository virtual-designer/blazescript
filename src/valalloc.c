/*
 * Created by rakinar2 on 9/17/23.
 */

#include "valalloc.h"
#include "alloca.h"
#include <stdlib.h>

struct val_alloc_tbl val_alloc_tbl_init()
{
    struct val_alloc_tbl value = {
        .size = 0,
        .capacity = VAL_TBL_INIT_CAP,
        .values = xcalloc(sizeof (val_t), VAL_TBL_INIT_CAP),
        .head = NULL
    };

    return value;
}

bool val_alloc_tbl_resize(struct val_alloc_tbl *tbl)
{
    if (tbl->size < tbl->capacity - 1)
        return false;

    tbl->capacity *= 2;
    tbl->values = xrealloc(tbl->values, (sizeof (val_t)) * tbl->capacity);
    return true;
}

val_t *val_alloc(struct val_alloc_tbl *tbl)
{
    if (tbl->head != NULL)
    {
        struct val_alloc_free_node *free_node = tbl->head;
        size_t index = free_node->index;
        tbl->head = free_node->next;
        free(free_node);
        return tbl->values + index;
    }

    val_alloc_tbl_resize(tbl);
    return tbl->values + (tbl->size++);
}

val_t *val_multi_alloc(struct val_alloc_tbl *tbl, size_t n)
{
    val_alloc_tbl_resize(tbl);
    val_t *ptr = tbl->values + tbl->size;
    tbl->size += n;
    return ptr;
}

void val_alloc_free(struct val_alloc_tbl *tbl, val_t *ptr, bool free_inner)
{
    struct val_alloc_free_node *free_node = tbl->head;
    tbl->head = xcalloc(1, sizeof (struct val_alloc_free_node));
    tbl->head->index = ptr - tbl->values;
    tbl->head->next = free_node;

    if (free_inner)
    {
        val_free_force_no_root(ptr);
    }

    ptr->type = VAL_NULL;
    ptr->nofree = false;
}

void val_alloc_tbl_free(struct val_alloc_tbl *tbl, bool recursive)
{
    if (recursive)
    {
        for (size_t i = 0; i < tbl->size; i++)
        {
            val_free_force_no_root(&tbl->values[i]);
        }
    }

    struct val_alloc_free_node *node = tbl->head;

    while (node != NULL)
    {
        struct val_alloc_free_node *tmp = node->next;
        free(node);
        node = tmp;
    }

    free(tbl->values);
}