AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../src
noinst_LIBRARIES = libblazestd.a libblazert.a
libblazert_a_SOURCES = value.c print.c utils.c
libblazestd_a_SOURCES = lib.c http.c $(libblazert_a_SOURCES)
