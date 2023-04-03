OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c)) 
CPPFILES := $(wildcard src/*.cpp)
CC = gcc
CFLAGS = -g -D_DEBUG -std=gnu11
CXXFLAGS = -g -D_DEBUG -std=gnu++23

blaze: $(OBJECTS) 
	g++ $(CXXFLAGS) -fPIC -c -o src/hashmap.o $(CPPFILES)
	g++ -L$$(pwd)/src $(CFLAGS) $(OBJECTS) src/hashmap.o -o blaze -lm

clean:
	rm -frv $(OBJECTS) blaze
