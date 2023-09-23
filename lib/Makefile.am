if DEBUG
GLOBAL_CPPFLAGS_ = -Wextra -fsanitize=address -fsanitize=leak
GLOBAL_LDFLAGS_ = -fsanitize=address -fsanitize=leak
else
GLOBAL_CPPFLAGS_ = -DNDEBUG
GLOBAL_LDFLAGS_ =
endif

AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../src $(GLOBAL_CPPFLAGS_)
AM_LDFLAGS = $(GLOBAL_LDFLAGS_)

noinst_LIBRARIES = libblazestd.a libblazert.a
libblazert_a_SOURCES = value.c print.c utils.c alloca.c
libblazestd_a_SOURCES = lib.c print.c http.c utils.c alloca.c log.c
