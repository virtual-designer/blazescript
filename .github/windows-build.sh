CURL_CFLAGS=$(curl-config --cflags)
CURL_LDFLAGS=$(curl-config --ldflags)
./configure CFLAGS="-I/usr/include -I/usr/local/include $CURL_CFLAGS" LDLAGS="=-L/usr/lib -L/usr/local/lib $CURL_LDFLAGS" --prefix="$(pwd)/build"
make
