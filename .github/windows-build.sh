pacman -S --noconfirm --needed autotools autoconf automake autoconf-wrapper libtool automake-wrapper aclocal

export CPPFLAGS="-I/c/msys2/usr/include ${CPPFLAGS}"
export LDFLAGS="-L/c/msys2/usr/lib ${LDFLAGS}"
export PATH="/c/msys2/usr/bin/:${PATH}"
export PKG_CONFIG_PATH="/c/msys2/usr/lib/pkgconfig:${PKG_CONFIG_PATH}"

autoreconf -fvi
./configure --with-unicode-handler=icu
make
make check V=1 VERBOSE=1
make install