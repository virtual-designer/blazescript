#!/bin/sh

. $(dirname "$0")/setup.sh



blaze_test_name "Print only one single string"

blaze_file << EOF
println("Hello world");
EOF

blaze_test "Hello world\n"


blaze_test_name "Print two different strings"

blaze_file << EOF
println("Hello world", "Some other string");
EOF

blaze_test "Hello world Some other string\n"


blaze_test_name "Print a number, string, boolean and null value all together"

blaze_file << EOF
println(1, "Hello world", true, null);
EOF

blaze_test "1 Hello world true null\n"


blaze_test_name "Print an array literal"

blaze_file << EOF
println(array [1, 2, 3, 4, "String", null, true, false]);
EOF

blaze_test 'Array [1, 2, 3, 4, "String", null, true, false]\n'
