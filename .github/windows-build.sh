export CPPFLAGS="-I/c/msys64/usr/include ${CPPFLAGS}"
export LDFLAGS="-L/c/msys64/usr/lib ${LDFLAGS}"
export PATH="/c/msys64/usr/bin/:${PATH}"
export PKG_CONFIG_PATH="/c/msys64/usr/lib/pkgconfig:${PKG_CONFIG_PATH}"

pacman -S --noconfirm --needed autotools autoconf automake autoconf-wrapper libtool automake-wrapper curl libcurl libcurl-devel mingw-w64-x86_64-curl

autoreconf -fvi

CURL_CFLAGS=$(curl-config --cflags)
CURL_LDFLAGS=$(curl-config --ldflags)
./configure CFLAGS="-I/usr/include -I/usr/local/include $CURL_CFLAGS" LDLAGS="=-L/usr/lib -L/usr/local/lib $CURL_LDFLAGS" --prefix="$(pwd)/build"
make
