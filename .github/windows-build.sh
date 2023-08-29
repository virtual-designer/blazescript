./configure CFLAGS="-I/usr/include -I/usr/local/include -I/mingw32/include" LDLAGS="=-L/usr/lib -L/usr/local/lib -L/mingw32/lib" --prefix="$(pwd)/build"
make
