/*
 * Created by rakinar2 on 8/22/23.
 */

#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "utils.h"
#include "file.h"
#include "alloca.h"

char *filebuf_current_file = NULL;

static size_t filebuf_get_filesize(struct filebuf *buf)
{
    struct stat st;

    if (fstat(fileno(buf->file), &st) != 0)
        fatal_error("cannot stat '%s': %s", buf->filename, strerror(errno));

    return st.st_size;
}

struct filebuf filebuf_init(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
        fatal_error("could not open file '%s': %s", filename, strerror(errno));

    struct filebuf buf = {
        .size = 0,
        .content = NULL,
        .filename = blaze_strdup(filename),
        .file = file
    };

    filebuf_current_file = buf.filename;
    buf.size = filebuf_get_filesize(&buf);
    return buf;
}

void filebuf_set_current_file(char *filename)
{
    filebuf_current_file = filename;
}

void filebuf_read(struct filebuf *buf)
{
    assert(buf != NULL);
    char *content = blaze_malloc(buf->size + 1);

    if (fread(content, 1, buf->size, buf->file) != buf->size)
        fatal_error("could not read file '%s': %s", buf->filename, strerror(errno));

    content[buf->size] = 0;
    buf->content = content;
}

void filebuf_free(struct filebuf *buf)
{
    blaze_free(buf->filename);
    blaze_free(buf->content);
    filebuf_current_file = NULL;
}