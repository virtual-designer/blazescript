if DEBUG
__CFLAGS = -fsanitize=address -fsanitize=leak
else
__CFLAGS = -DNDEBUG
endif

_blaze_HEADERS_ = alloca.h \
	      ast.h \
		  datatype.h \
		  eval.h \
		  file.h \
		  lexer.h \
		  log.h \
		  parser.h \
		  scope.h \
		  utils.h \
		  valmap.h \
		  vector.h

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
                vector.c \
                $(_blaze_HEADERS_)
blaze_CFLAGS = $(__CFLAGS)
blaze_CPPFLAGS = -std=gnu11 -I$(top_srcdir)/include
blaze_LDFLAGS = -L$(top_srcdir)/lib
blaze_LDADD = -lcurl -lblazestd