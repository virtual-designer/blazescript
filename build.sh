#!/bin/sh

cd lib;

for f in *.c; do
    echo "CC "$f
    gcc -DNDEBUG -O2 -g -I. -I.. -I../include -I../src -c $f -o $f.o

    if test "$?" != "0"; then
        exit 1
    fi
done

echo "AR libblazestd.a"

ar rcs libblazestd.a *.o
    if test "$?" != "0"; then
        exit 1
    fi
cd ..

cd src;

for f in *.c; do
    echo "CC "$f

    gcc -DNDEBUG -O2 -g -I. -I.. -I../include -c $f -o $f.o
    if test "$?" != "0"; then
        exit 1
    fi
done

echo "CCLD blaze"
gcc  -O2 -g *.o ../lib/libblazestd.a -o blaze

if test "$?" != "0"; then
    exit 1
fi