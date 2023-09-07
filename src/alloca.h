/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_ALLOCA_H
#define BLAZESCRIPT_ALLOCA_H

#include <stddef.h>
#include <stdbool.h>

struct alloca_tbl_t {
    void **ptrs;
    size_t count;
};

void *blaze_malloc(size_t size);
void *blaze_realloc(void *old_ptr, size_t new_size);
void *blaze_calloc(size_t n, size_t size);
bool blaze_free(void *ptr);
void blaze_alloca_tbl_free();
void blaze_alloca_tbl_init();
size_t blaze_alloca_tbl_push_ptr(void *ptr);
void *blaze_strdup(const char *str);

void *xmalloc(size_t size);
void *xcalloc(size_t n, size_t size);
void *xrealloc(void *old_ptr, size_t new_size);

#endif /* BLAZESCRIPT_ALLOCA_H */
