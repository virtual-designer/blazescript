if DEBUG
GLOBAL_CPPFLAGS_ = -Wextra -fsanitize=address -fsanitize=leak
GLOBAL_LDFLAGS_ = -fsanitize=address -fsanitize=leak
else
GLOBAL_CPPFLAGS_ = -DNDEBUG
GLOBAL_LDFLAGS_ =
endif

bin_PROGRAMS = blaze blazevm blazec

blaze_HEADERS_ = alloca.h \
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
		  vector.h \
		  map.h \
		  valalloc.h

blazec_HEADERS_ = alloca.h \
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
		  vector.h \
		  map.h \
		  compile.h \
		  compile-x86_64.h \
		  valalloc.h \
		  asm.h \
		  arch.h

blazevm_HEADERS_ = bytecode.h \
	      opcode.h \
          register.h \
          disassemble.h \
          stack.h \
		  valalloc.h

blaze_SOURCES = alloca.c \
                file.c \
                lexer.c \
                log.c \
                blaze.c \
                parser.c \
                utils.c \
                eval.c \
                scope.c \
                valmap.c \
                vector.c \
			    datatype.c \
			    map.c \
			    valalloc.c \
                $(blaze_HEADERS_)

blazec_SOURCES = alloca.c \
                file.c \
                lexer.c \
                log.c \
                blazec.c \
                parser.c \
                utils.c \
                eval.c \
                scope.c \
                valmap.c \
                vector.c \
			    datatype.c \
			    map.c \
			    valalloc.c \
			    compile.c \
			    compile-x86_64.c \
			    asm.c \
			    arch.c \
                $(blazec_HEADERS_)

blazevm_SOURCES = alloca.c \
				  log.c \
				  utils.c \
				  vector.c \
				  datatype.c \
				  bytecode.c \
				  opcode.c \
				  register.c \
				  parser.c \
				  scope.c \
				  lexer.c \
                  valmap.c \
                  eval.c \
				  blazevm.c \
				  disassemble.c \
                  file.c \
                  stack.c \
			      valalloc.c \
                  $(blazevm_HEADERS_)

AM_CPPFLAGS = -std=gnu11 -I$(top_srcdir)/include $(GLOBAL_CPPFLAGS_)
AM_LDFLAGS = -L$(top_srcdir)/lib $(GLOBAL_LDFLAGS_)
blaze_LDADD = -lblazestd
blazec_LDADD = -lblazestd
blazevm_LDADD = -lblazestd -lm
