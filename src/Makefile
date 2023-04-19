BLAZE_OBJECTS = blaze.o \
				debug.o \
				eval.o \
				functions.o \
				lexer.o \
				map.o \
				parser.o \
				scope.o \
				string.o \
				xmalloc.o \
				utils.o

# BLAZEC_OBJECTS = blazec.o \
# 				 debug.o \
# 				 eval.o \
# 				 functions.o \
# 				 lexer.o \
# 				 map.o \
# 				 parser.o \
# 				 scope.o \
# 				 string.o \
# 				 xmalloc.o \
# 				 utils.o \
# 				 compile.o

# BLAZE_STDLIB_OBJECTS = blaze_stdlib.o

CC = gcc
# AR = ar
CFLAGS = -g -D_DEBUG -D_NODEBUG -std=gnu11 -Wall -Wextra
# BLAZEC_CFLAGS := $(shell llvm-config --cflags)
# BLAZEC_LDFLAGS := $(shell llvm-config --ldflags --system-libs --libs core)

all: blaze # blazec libblaze.a $(BLAZEC_OBJECTS) 
 
blaze: $(BLAZE_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZE_OBJECTS) -o $@ -lm 

# $(BLAZEC_OBJECTS):
# 	$(CC) $(CFLAGS) $(BLAZEC_CFLAGS) -c $(patsubst %.o,%.c,$@) -o $@

# blazec: $(BLAZEC_OBJECTS)
# 	$(CC) $(CFLAGS) $(BLAZEC_OBJECTS) $(BLAZEC_LDFLAGS) -o $@ -lm 

# libblaze.a: $(BLAZE_STDLIB_OBJECTS)
# 	$(AR) cvr $@ $(BLAZE_STDLIB_OBJECTS)

clean:
	rm -frv $(BLAZE_OBJECTS) 
	
# blaze blazec$(BLAZEC_OBJECTS)
