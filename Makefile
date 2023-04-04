OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c)) 
CC = gcc
CFLAGS = -g -D_DEBUG -std=gnu11

blaze: $(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) -o blaze -lm 

clean:
	rm -frv $(OBJECTS) blaze
