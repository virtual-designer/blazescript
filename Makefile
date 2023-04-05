OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c)) 
CC = gcc
CFLAGS = -g -D_DEBUG -D_NODEBUG -std=gnu11 -Wall -Wextra

blaze: $(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) -o blaze -lm 

clean:
	rm -frv $(OBJECTS) blaze
