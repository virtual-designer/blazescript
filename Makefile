all:
	$(MAKE) -C src
	mv src/blaze .
	mv src/blazec .

clean:
	$(MAKE) -C src clean
	rm -frv *.o blaze blazec
