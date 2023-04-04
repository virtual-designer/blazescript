#ifndef __BLAZE_H__
#define __BLAZE_H__

#include <stdbool.h>

typedef struct {
    char *progname;
} config_t;

void blaze_error(bool shouldexit, char *format, ...);

extern config_t config;

#endif /* __BLAZE_H__ */
 