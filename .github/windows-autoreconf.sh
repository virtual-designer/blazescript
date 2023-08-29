export CPPFLAGS="-I/c/msys64/usr/include ${CPPFLAGS}"
export LDFLAGS="-L/c/msys64/usr/lib ${LDFLAGS}"
export PATH="/c/msys64/usr/bin/:${PATH}"
export PKG_CONFIG_PATH="/c/msys64/usr/lib/pkgconfig:${PKG_CONFIG_PATH}"

pacman -S --noconfirm --needed autotools autoconf automake autoconf-wrapper libtool automake-wrapper curl libcurl libcurl-devel mingw-w64-x86_64-curl
curl-config 

autoreconf -fvi
