#!/usr/bin/env bats
#
# Test suite for ccl_kerninfo utility
#
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# ##################################### #
# Setup and teardown for each test case #
# ##################################### #

setup() {

	# ccl_kerninfo binary
	CCL_KI_COM="@CMAKE_BINARY_DIR@/src/utils/ccl_kerninfo"

	# ccl_devinfo binary
	CCL_KI_DEVINFO="@CMAKE_BINARY_DIR@/src/utils/ccl_devinfo"

	# How many devices?
	CCL_KI_NDEVS=`${CCL_KI_DEVINFO} | grep -c "\[ Device #"`

	# Device index to use in tests
	if [ -z ${CCL_TEST_DEVICE_INDEX} ]
	then
		CCL_TEST_DEVICE_INDEX="@TESTS_DEVICE_INDEX@"
	fi

	# Minimum OpenCL version between device platform and cf4ocl
	CCL_KI_OCL_VERSION_PLATF=`${CCL_KI_DEVINFO} -o -d ${CCL_TEST_DEVICE_INDEX} -c VERSION | grep -o "OpenCL [0-9]\.[0-9]" | cut -d " " -f 2`
	CCL_KI_OCL_VERSION_CF4OCL=`${CCL_KI_DEVINFO} --version | grep -o "OpenCL [0-9]\.[0-9]" | cut -d " " -f 2`
	CCL_KI_OCL_VERSION=`echo "if (${CCL_KI_OCL_VERSION_PLATF} < ${CCL_KI_OCL_VERSION_CF4OCL}) { ${CCL_KI_OCL_VERSION_PLATF} } else { ${CCL_KI_OCL_VERSION_CF4OCL} }" | bc`

	# Test kernels folder
	CCL_KI_K_FOLDER="@CMAKE_SOURCE_DIR@/tests/test_kernels"

	# Fully working kernels
	CCL_KI_K_SUM="${CCL_KI_K_FOLDER}/sum_full.cl"
	CCL_KI_K_XOR="${CCL_KI_K_FOLDER}/xor_full.cl"

	# Fully working kernel names
	CCL_KI_K_SUM_NAME="test_sum_full"
	CCL_KI_K_XOR_NAME="test_xor_full"

	# Non-working kernel
	CCL_KI_K_BAD="${CCL_KI_K_FOLDER}/not_ok.cl"
	CCL_KI_K_BAD_NAME="fun"

	# Function implementations
	CCL_C_KIMPL_SUM="${CCL_C_K_FOLDER}/sum_impl.cl"
	CCL_C_KIMPL_XOR="${CCL_C_K_FOLDER}/xor_impl.cl"

	# Function names
	CCL_C_KIMPL_SUM_FNAME="do_sum"
	CCL_C_KIMPL_XOR_FNAME="do_xor"

}

# ########################## #
# Test erroneous invocations #
# ########################## #

# Test invocation without arguments
@test "Invocation without arguments" {

	run ${CCL_KI_COM}
	[ "$status" -ne 0 ]

}

# Test version
@test "Version" {

	run ${CCL_KI_COM} --version
	[ "$status" -eq 0 ]

}

# Test invocation with wrong number of valid arguments
@test "Invocation with wrong number of valid arguments" {

	run ${CCL_KI_COM} -s
	[ "$status" -ne 0 ]

	run ${CCL_KI_COM} -s ${CCL_KI_K_SUM}
	[ "$status" -ne 0 ]

}

# Test invocation with wrong number of invalid arguments
@test "Invocation with wrong number of invalid arguments" {

	run ${CCL_KI_COM} --not-an-option
	[ "$status" -ne 0 ]

	run ${CCL_KI_COM} -n _non_existing_file.cl
	[ "$status" -ne 0 ]

}

# Test invocation with correct number of invalid arguments
@test "Invocation with correct number of invalid arguments" {

	run ${CCL_KI_COM} --bad-option _non_existing_file.cl unknown_kernel \
		${CCL_TEST_DEVICE_INDEX}
	[ "$status" -ne 0 ]

}

# Test with valid file and valid kernel
@test "Valid file and valid kernel" {

	run ${CCL_KI_COM} -s ${CCL_KI_K_SUM} ${CCL_KI_K_SUM_NAME} \
		${CCL_TEST_DEVICE_INDEX}
	[ "$status" -eq 0 ]

	run ${CCL_KI_COM} -s ${CCL_KI_K_XOR} ${CCL_KI_K_XOR_NAME} \
		${CCL_TEST_DEVICE_INDEX}
	[ "$status" -eq 0 ]

}

# Test with invalid program
@test "Invalid program" {

	run ${CCL_KI_COM} -s ${CCL_KI_K_BAD} ${CCL_KI_K_BAD_NAME} \
		${CCL_TEST_DEVICE_INDEX}
	[ "$status" -ne 0 ]

}

# Test with valid program, unknown kernel
@test "Valid program, unknown kernel" {

	run ${CCL_KI_COM} -s ${CCL_KI_K_SUM} _this_kernel_does_not_exist_ \
		${CCL_TEST_DEVICE_INDEX}
	[ "$status" -ne 0 ]

}

# Test with valid program without a kernel, just a function
@test "Valid program without a kernel, just a function" {

	run ${CCL_KI_COM} -s ${CCL_C_KIMPL_SUM} ${CCL_C_KIMPL_SUM_FNAME} \
		${CCL_TEST_DEVICE_INDEX}
	[ "$status" -ne 0 ]

	run ${CCL_KI_COM} -s ${CCL_C_KIMPL_XOR} ${CCL_C_KIMPL_XOR_FNAME} \
		${CCL_TEST_DEVICE_INDEX}
	[ "$status" -ne 0 ]
}
