#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Built-in cd command" {
    run ./dsh <<EOF
cd /tmp
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"/tmp"* ]]
}

@test "Maximum pipe limit" {
    run ./dsh <<EOF
$(printf "echo %0.s" {1..10} | tr ' ' '|')
EOF
    [ "$status" -eq 0 ]
}