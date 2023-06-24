bin_PROGRAMS = blaze blazec blazevm blazeas

blaze_SOURCES = \
    blaze.c \
    debug.c \
    eval.c \
    functions.c \
    lexer.c \
    map.c \
    parser.c \
    scope.c \
    bstring.c \
    xmalloc.c \
    utils.c

blazec_SOURCES = \
    blazec.c \
    debug.c \
    lexer.c \
    parser.c \
    bstring.c \
    xmalloc.c \
    utils.c \
    bytecode.c \
    opcode.c \
    compile.c \
    functions.c \
    eval.c \
    scope.c \
    map.c \
    stack.c

blazevm_SOURCES = \
    blazevm.c \
    debug.c \
    bstring.c \
    xmalloc.c \
    utils.c \
    bytecode.c \
    opcode.c \
    compile.c \
    functions.c \
    eval.c \
    scope.c \
    map.c \
    stack.c

blazeas_SOURCES = \
    blazeas.c \
    debug.c \
    xmalloc.c \
    utils.c \
    bytecode.c \
    opcode.c \
    map.c \
    stack.c \
    scope.c \
    functions.c \
    eval.c \
    bstring.c \
    compile.c \
    assemble.c

AM_CFLAGS = -D_NODEBUG
AM_LDFLAGS = -lm
