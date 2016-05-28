#!/usr/bin/env bats
#
# Test suite for ccl_c utility
#
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# ##################################### #
# Setup and teardown for each test case #
# ##################################### #

setup() {

	# ccl_c binary
	CCL_C_COM="@CMAKE_BINARY_DIR@/src/utils/ccl_c"

	# Device index to use in tests
	if [ -z ${CCL_C_DEV_IDX} ]
	then
		CCL_C_DEV_IDX="@TESTS_DEVICE_INDEX@"
	fi

	# Folder containing example kernels
	CCL_C_EXAMPLES_FOLDER="@CMAKE_SOURCE_DIR@/src/examples"

	# Working kernel 1
	CCL_C_K1_OK="${CCL_C_EXAMPLES_FOLDER}/canon.cl"

	# Working kernel 2
	CCL_C_K2_OK="${CCL_C_EXAMPLES_FOLDER}/ca.cl"

	# Non-working kernel
	CCL_C_K3_KO="@CMAKE_CURRENT_BINARY_DIR@/temp.cl"
	echo "#include <wrong.h>\nint fun(void* x) {return 0;}\n" > ${CCL_C_K3_KO}

	# Base name for temporary binary files
	CCL_C_TMP_BIN="@CMAKE_CURRENT_BINARY_DIR@/temp.bin"

}

teardown() {

	# Remove possible temporary files
	rm -f ${CCL_C_K3_KO}
	rm -f ${CCL_C_TMP_BIN}{1..2}

}

# ################################### #
# Test help, list and version options #
# ################################### #

# Test help option, which should return status 0.
@test "Help options" {

	run ${CCL_C_COM} -?
	[ "$status" -eq 0 ]

	run ${CCL_C_COM} --help
	[ "$status" -eq 0 ]

}

# Test version, which should return status 0.
@test "Get version" {

	run ${CCL_C_COM} --version
	[ "$status" -eq 0 ]

}

# Test device list, which should return status 0.
@test "List devices" {

	run ${CCL_C_COM} -l
	[ "$status" -eq 0 ]

	run ${CCL_C_COM} --list
	[ "$status" -eq 0 ]

}

# ############### #
# Test build task #
# ############### #

# Test simple build with one source file.
@test "Test build with one source file" {

	run ${CCL_C_COM} -s ${CCL_C_K1_OK} -d ${CCL_C_DEV_IDX}
	[ "$status" -eq 0 ]

}

# Test build with two source files.
@test "Test build with two source files" {

	run ${CCL_C_COM} -s ${CCL_C_K1_OK} -s ${CCL_C_K2_OK} -d ${CCL_C_DEV_IDX}
	[ "$status" -eq 0 ]

}

# Test build with one source, create binary, then with one binary.
@test "Test build with one source, create binary, then with one binary" {

	# Test build with one source and create binary.
	run ${CCL_C_COM} -s ${CCL_C_K1_OK} -o ${CCL_C_TMP_BIN}1 -d ${CCL_C_DEV_IDX}
	[ "$status" -eq 0 ]
	[ -f ${CCL_C_TMP_BIN}1 ]

	# Test build with one binary.
	run ${CCL_C_COM} -b ${CCL_C_TMP_BIN}1 -d ${CCL_C_DEV_IDX}
	[ "$status" -eq 0 ]

}

# Test build with two binaries created from two different source files. */
@test "Test build with two binaries created from two different source files" {

	# Test build with one source and create binary.
	run ${CCL_C_COM} -s ${CCL_C_K1_OK} -o ${CCL_C_TMP_BIN}1 -d ${CCL_C_DEV_IDX}
	[ "$status" -eq 0 ]
	[ -f ${CCL_C_TMP_BIN}1 ]

	# Test build with another source and create another binary.
	run ${CCL_C_COM} -s ${CCL_C_K2_OK} -o ${CCL_C_TMP_BIN}2 -d ${CCL_C_DEV_IDX}
	[ "$status" -eq 0 ]
	[ -f ${CCL_C_TMP_BIN}2 ]

	# Test build with two binaries. */
	run ${CCL_C_COM} -b ${CCL_C_TMP_BIN}1 -b ${CCL_C_TMP_BIN}2 -d \
		${CCL_C_DEV_IDX}
	[ "$status" -ne 0 ]

}

# Test build with one binary and one source file.
@test "Test build with one binary and one source file" {

	# Test build with one source and create binary.
	run ${CCL_C_COM} -s ${CCL_C_K1_OK} -o ${CCL_C_TMP_BIN}1 -d ${CCL_C_DEV_IDX}
	[ "$status" -eq 0 ]
	[ -f ${CCL_C_TMP_BIN}1 ]

	# Test build with one binary and one source file.
	run ${CCL_C_COM} -s ${CCL_C_K1_OK} -b ${CCL_C_TMP_BIN}2 -d ${CCL_C_DEV_IDX}
	[ "$status" -ne 0 ]

}

# Test build with source headers.
@test "Test build with source headers" {
	run ${CCL_C_COM} -h ${CCL_C_K1_OK} -d ${CCL_C_DEV_IDX}
	[ "$status" -ne 0 ]
}

# Test build with erroneous kernel.
@test "Test build with erroneous kernel" {

	run ${CCL_C_COM} -s ${CCL_C_K3_KO} -d ${CCL_C_DEV_IDX}
	[ "$status" -ne 0 ]

}

# Test build with non-existing device.
@test "Test build with non-existing device" {

	run ${CCL_C_COM} -s ${CCL_C_K1_OK} -d 100000 # TODO Replace value with number of lines returned in -l option + 1
	[ "$status" -ne 0 ]

}

# Test build with non-existing file.
@test "Test build with non-existing file" {

	run ${CCL_C_COM} -s this_file_does_not_exist.cl -d ${CCL_C_DEV_IDX}
	[ "$status" -ne 0 ]

}
