#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "blaze.h"
#include "debug.h"

void __debug_structdmp(void *invar, char *structname)
{
    char cmd[160];

    sprintf(cmd, "echo 'p (struct %s) *%p\n' > gdbcmds", structname, invar);
    system(cmd);

    sprintf(cmd, "echo 'where\ndetach' | gdb -batch --command=gdbcmds %s %d > struct.dump", config.progname, getpid());
    system(cmd);

    sprintf(cmd, "cat struct.dump");
    system(cmd);
}
