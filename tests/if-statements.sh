#!/bin/sh

. $(dirname "$0")/setup.sh

blaze_test_name "Hello world"

blaze_file << EOF
println("Hello world");
EOF

blaze_test "Hello world\n" 1
