/*
 * Created by rakinar2 on 8/22/23.
 */

#ifndef BLAZESCRIPT_FILE_H
#define BLAZESCRIPT_FILE_H

#include <stdio.h>

struct filebuf
{
    size_t size;
    char *content;
    char *filename;
    FILE *file;
};

struct filebuf filebuf_init(const char *filename);
void filebuf_read(struct filebuf *buf);
void filebuf_free(struct filebuf *buf);
void filebuf_set_current_file(char *filename);

extern char *filebuf_current_file;

#endif /* BLAZESCRIPT_FILE_H */
