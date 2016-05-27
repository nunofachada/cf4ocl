#!/usr/bin/env bats

setup() {
	echo "Start test"
}

teardown() {
	echo "End test"
}

@test "Help options" {
    run ./src/utils/ccl_c -?
    [ "$status" -eq 0 ]
    run ./src/utils/ccl_c --help
    [ "$status" -eq 0 ]
}

@test "Get version" {
    run ./src/utils/ccl_c --version
    [ "$status" -eq 0 ]
}

@test "List devices" {
    run ./src/utils/ccl_c -l
    [ "$status" -eq 0 ]
    run ./src/utils/ccl_c --list
    [ "$status" -eq 0 ]
}
