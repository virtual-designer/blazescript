#!/bin/sh

TEST_NAME="Unnamed"

blaze_run() {
    echo $($BLAZE "$FILE" | sed -r "s/\x1B\[([0-9]{1,3}(;[0-9]{1,2};?)?)?[mGK]//g")
}

blaze_file() {
    echo "$(cat /dev/stdin)" > $FILE
}

blaze_test_name() {
    TEST_NAME="$1"
}

blaze_test() {
    output=$(blaze_run)
    expected=$(printf "$1")

    test "$output" = "$expected"

    if test "$?" != "0"; then
        printf "\033[1;31mFAIL\033[0m \033[2m%s\033[0m\n" "$TEST_NAME"
        exit 127
    else
        printf "\033[1;32mPASS\033[0m \033[2m%s\033[0m\n" "$TEST_NAME"
    fi
}
