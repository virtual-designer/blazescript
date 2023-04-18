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

BLAZEC_OBJECTS = blazec.o \
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
CC = gcc
CFLAGS = -g -D_DEBUG -D_NODEBUG1 -std=gnu11 -Wall -Wextra

all: blaze blazec

blaze: $(BLAZE_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZE_OBJECTS) -o $@ -lm 

blazec: $(BLAZEC_OBJECTS)
	$(CC) $(CFLAGS) $(BLAZEC_OBJECTS) -o $@ -lm 

clean:
	rm -frv $(OBJECTS) blaze blazec
