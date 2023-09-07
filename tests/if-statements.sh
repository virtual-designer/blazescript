#!/bin/sh

. "$(dirname "$0")"/setup.sh

blaze_test_name "Basic if statements"
blaze_file << EOF
if (true) {
    println("True");
}

if (false) {
    println("False");
}
EOF
blaze_test "True"

blaze_test_name "Basic if-else statements"
blaze_file << EOF
if (true) {
    println("True");
}
else {
    println("Not True");
}

if (false) {
    println("False");
}
else {
    println("Not False");
}
EOF
blaze_test "True\nNot False\n"

blaze_test_name "If-else statements with truthy value test"
blaze_file << EOF
const _test0 = 1;
const _test1 = "String";
const _test2 = true;
const _test3 = 0;
const _null_value = null;

if (_test0) {
    println("_test0 is truthy");
}
else {
    println("_test0 is not truthy");
}

if (_test1) {
    println("_test1 is truthy");
}
else {
    println("_test1 is not truthy");
}

if (_test2) {
    println("_test2 is truthy");
}
else {
    println("_test2 is not truthy");
}

if (_test3) {
    println("_test3 is truthy");
}
else {
    println("_test3 is not truthy");
}

if (_null_value) {
    println("_null_value is truthy");
}
else {
    println("_null_value is not truthy");
}
EOF
blaze_test "_test0 is truthy\n_test1 is truthy\n_test2 is truthy\n_test3 is not truthy\n_null_value is not truthy\n"

blaze_test_name "If-else statements with conditions"
blaze_file << EOF
if (true == 1) {
    println("True is 1");
}
else {
    println("Fail");
}

if (true === 1) {
    println("Fail");
}
else {
    println("True is not 1");
}

if (5 > 3) {
    println("5 > 3");
}
else {
    println("Fail");
}

if (5 >= 3) {
    println("5 >= 3");
}
else {
    println("Fail");
}


if (5 < 3) {
    println("Fail");
}
else {
    println("5 < 3");
}

if (5 <= 3) {
    println("Fail");
}
else {
    println("5 <= 3");
}

if ("str1" === "str1") {
    println("str1 === str1");
}
else {
    println("Fail");
}

if ("str2" === "str1") {
    println("Fail");
}
else {
    println("str1 === str2");
}
EOF
blaze_test "True is 1\nTrue is not 1\n5 > 3\n5 >= 3\n5 < 3\n5 <= 3\nstr1 === str1\nstr1 === str2\n"