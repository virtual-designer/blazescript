#!/bin/sh

. $(dirname "$0")/setup.sh



blaze_test_name "Print only one single string"

blaze_file << EOF
println("Hello world");
EOF

blaze_test "Hello world\n" 1


blaze_test_name "Print two different strings"

blaze_file << EOF
println("Hello world", "Some other string");
EOF

blaze_test "Hello world Some other string\n" 2


blaze_test_name "Print a number, string, boolean and null value all together"

blaze_file << EOF
println(1, "Hello world", true, null);
EOF

blaze_test "1 Hello world true null\n" 2


blaze_test_name "Print an object literal"

blaze_file << EOF
println({
    some_property: 123,
    a_string: "Blaze",
    null_value: null,
    boolval: true
});
EOF

blaze_test '[Object] { some_property: 123, null_value: null, boolval: true, a_string: "Blaze" }\n' 2
