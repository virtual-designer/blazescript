/*
 * Created by rakinar2 on 9/17/23.
 */

#ifndef BLAZESCRIPT_VALALLOC_H
#define BLAZESCRIPT_VALALLOC_H

#include "datatype.h"
#include <stddef.h>

#ifndef VAL_TBL_INIT_CAP
#define VAL_TBL_INIT_CAP 4096
#endif

struct val_alloc_tbl
{
    size_t size;
    size_t capacity;
    val_t *values;
    struct val_alloc_free_node *head;
};

struct val_alloc_free_node
{
    size_t index;
    struct val_alloc_free_node *next;
};

struct val_alloc_tbl val_alloc_tbl_init();
val_t *val_alloc(struct val_alloc_tbl *tbl);
val_t *val_multi_alloc(struct val_alloc_tbl *tbl, size_t n);
void val_alloc_free(struct val_alloc_tbl *tbl, val_t *ptr, bool free_inner);
void val_alloc_tbl_free(struct val_alloc_tbl *tbl, bool recursive);

#endif /* BLAZESCRIPT_VALALLOC_H */
