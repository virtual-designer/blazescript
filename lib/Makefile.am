AUTOMAKE_OPTIONS = subdir-objects

if DEBUG
GLOBAL_CPPFLAGS_ = -Wextra -fsanitize=address -fsanitize=leak
GLOBAL_LDFLAGS_ = -fsanitize=address -fsanitize=leak
else
GLOBAL_CPPFLAGS_ = -DNDEBUG
GLOBAL_LDFLAGS_ =
endif

SRC_ADD = 

if LINUX
SRC_ADD += x86_64/stdlib_linux.s \
           x86_64/stdio_linux.s
endif

if DARWIN
SRC_ADD += x86_64/stdlib_darwin.s
endif

if WINDOWS
SRC_ADD += x86_64/stdlib_windows.s
endif

AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../src $(GLOBAL_CPPFLAGS_)
AM_LDFLAGS = $(GLOBAL_LDFLAGS_)
AM_CCASFLAGS = -g

noinst_LIBRARIES = libblazestd.a libblazert.a
libblazert_a_SOURCES = value.c print.c utils.c alloca.c log.c $(SRC_ADD)
libblazestd_a_SOURCES = lib.c print.c http.c utils.c alloca.c log.c
