if DEBUG
__CFLAGS = -fsanitize=address -fsanitize=leak
else
__CFLAGS = -DNDEBUG
endif

bin_PROGRAMS = blaze
blaze_SOURCES = alloca.c \
                file.c \
                lexer.c \
                log.c \
                main.c \
                parser.c \
                utils.c \
                eval.c \
                scope.c \
                valmap.c \
                lib.c
blaze_CFLAGS = $(__CFLAGS)
blaze_CPPFLAGS = -std=gnu11
blaze_LDADD = -lcurl