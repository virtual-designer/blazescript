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

BLAZEAS_OBJECTS = src/blazeas.o \
				 src/debug.o \
				 src/xmalloc.o \
				 src/utils.o \
				 src/bytecode.o \
				 src/opcode.o \
				 src/map.o \
				 src/stack.o \
				 src/scope.o \
				 src/functions.o \
				 src/eval.o \
				 src/string.o \
				 src/compile.o \
				 src/assemble.o

CC = gcc
CFLAGS = -g -D_DEBUG -D_NODEBUG -std=gnu11 -Wall -Wextra

all: blaze blazevm blazec blazeas

blaze: $(BLAZE_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZE_OBJECTS) -o $@ -lm

blazec: $(BLAZEC_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZEC_OBJECTS) -o $@ -lm

blazevm: $(BLAZEVM_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZEVM_OBJECTS) -o $@ -lm

blazeas: $(BLAZEAS_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZEAS_OBJECTS) -o $@ -lm

clean:
	rm -frv $(BLAZE_OBJECTS) $(BLAZEVM_OBJECTS) $(BLAZEC_OBJECTS) $(BLAZEAS_OBJECTS) blaze blazevm blazec blazeas
