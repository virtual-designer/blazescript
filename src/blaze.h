#ifndef __BLAZE_H__
#define __BLAZE_H__

#include <stdbool.h>
#include "runtimevalues.h"
#include "utils.h"

#define VERSION "1.0.0-beta1"

typedef struct {
    char *progname; /* Name of the interpreter itself */
    char *entryfile;
    char *currentfile;
} config_t;

void blaze_error(bool shouldexit, char *format, ...);
void handle_result(runtime_val_t *result, bool newline, int tabs, bool quote_strings);

extern config_t config;

#endif /* __BLAZE_H__ */
 
