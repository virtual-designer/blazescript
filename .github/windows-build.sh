pacman -S --noconfirm --needed autotools autoconf automake autoconf-wrapper libtool automake-wrapper aclocal

export CPPFLAGS="-I/c/msys2/usr/include ${CPPFLAGS}"
export LDFLAGS="-L/c/msys2/usr/lib ${LDFLAGS}"
export PATH="/c/msys2/usr/bin/:${PATH}"
export PKG_CONFIG_PATH="/c/msys2/usr/lib/pkgconfig:${PKG_CONFIG_PATH}"

rm -frv build-aux configure *.in src/*.in
autoreconf -fvi