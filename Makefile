all:
	$(MAKE) -C src
	mv src/blaze .
#	mv src/blazec .
#	mv src/libblaze.a .

#blazec:
#	$(MAKE) -C src blazec libblaze.a
#	mv src/blazec .
#	mv src/libblaze.a .

clean:
	$(MAKE) -C src clean
	rm -frv *.o blaze 
#blazec libblaze.a
