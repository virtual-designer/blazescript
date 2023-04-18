#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <libgen.h>

#include "blaze.h"

config_t config;

int main(int argc, char **argv)
{
    config.progname = basename(argv[0]);

    if (argc < 2)
    {
        utils_error(true, "no input files");
    }



    return 0;
}
