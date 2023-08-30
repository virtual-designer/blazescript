if DEBUG
__CPPFLAGS = -fsanitize=address -fsanitize=leak
__LDFLAGS = -fsanitize=address -fsanitize=leak
else
__CPPFLAGS = -DNDEBUG
__LDFLAGS =
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

AM_CPPFLAGS = -std=gnu11 -I$(top_srcdir)/include $(__CPPFLAGS)
AM_LDFLAGS = -L$(top_srcdir)/lib $(__LDFLAGS)
blaze_LDADD = -lblazestd