BLAZE_OBJECTS = src/blaze.o \
				src/debug.o \
				src/eval.o \
				src/functions.o \
				src/lexer.o \
				src/map.o \
				src/parser.o \
				src/scope.o \
				src/string.o \
				src/xmalloc.o \
				src/utils.o

 BLAZEC_OBJECTS = src/blazec.o \
 				 src/debug.o \
 				 src/lexer.o \
 				 src/parser.o \
 				 src/string.o \
 				 src/xmalloc.o \
 				 src/utils.o \
 				 src/bytecode.o \
 				 src/opcode.o \
 				 src/compile.o \
 				 src/functions.o \
 				 src/eval.o \
 				 src/scope.o \
 				 src/map.o \
 				 src/stack.o

BLAZEVM_OBJECTS = src/blazevm.o \
				 src/debug.o \
				 src/string.o \
				 src/xmalloc.o \
				 src/utils.o \
				 src/bytecode.o \
				 src/opcode.o \
				 src/compile.o \
				 src/functions.o \
				 src/eval.o \
				 src/scope.o \
				 src/map.o \
				 src/stack.o

# BLAZE_STDLIB_OBJECTS = blaze_stdlib.o

CC = gcc
# AR = ar
CFLAGS = -g -D_DEBUG -D_NODEBUG -std=gnu11 -Wall -Wextra
# BLAZEC_CFLAGS := $(shell llvm-config --cflags)
# BLAZEC_LDFLAGS := $(shell llvm-config --ldflags --system-libs --libs core)

all: blaze blazevm blazec

# blazec blazevm

blaze: $(BLAZE_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZE_OBJECTS) -o $@ -lm

blazec: $(BLAZEC_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZEC_OBJECTS) -o $@ -lm

blazevm: $(BLAZEVM_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZEVM_OBJECTS) -o $@ -lm

# libblaze.a: $(BLAZE_STDLIB_OBJECTS)
# 	$(AR) cvr $@ $(BLAZE_STDLIB_OBJECTS)

clean:
	rm -frv $(BLAZE_OBJECTS) $(BLAZEVM_OBJECTS) $(BLAZEC_OBJECTS) blaze blazevm blazec
	
# $(BLAZEC_OBJECTS) $(BLAZEVM_OBJECTS) blazec blazevm
# blaze blazec$(BLAZEC_OBJECTS)
