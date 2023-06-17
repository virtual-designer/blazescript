#!/bin/sh

blaze_run() {
    echo $($BLAZE "$FILE" | sed -r "s/\x1B\[([0-9]{1,3}(;[0-9]{1,2};?)?)?[mGK]//g")
}

blaze_file() {
    echo "$(cat /dev/stdin)" > $FILE
}

blaze_test() {
    output=$(blaze_run)
    expected=$(printf "$1")

    test "$output" = "$expected"

    if test "$?" != "0"; then
        exit 127
    fi
}
