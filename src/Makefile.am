if DEBUG
__CFLAGS = -fsanitize=address -fsanitize=leak
endif

bin_PROGRAMS = blaze
blaze_SOURCES = alloca.c \
                file.c \
                lexer.c \
                log.c \
                main.c \
                parser.c \
                utils.c \
                eval.c
blaze_CFLAGS = $(__CFLAGS)