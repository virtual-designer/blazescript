#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "file.h"
#include "lexer.h"

void process_file(const char *name)
{
    struct filebuf buf = filebuf_init(name);
    filebuf_read(&buf);
    struct lex *lex = lex_init(buf.content);
    lex_analyze(lex);
    blaze_debug__lex_print(lex);
    lex_free(lex);
//    printf("Contents:\n%s\n", buf.content);
    filebuf_free(&buf);
}

int main(int argc, char **argv)
{
    if (argc < 2)
        fatal_error("no input file specified");

    process_file(argv[1]);
    return 0;
}