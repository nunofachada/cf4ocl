#!/usr/bin/env bats
#
# Test suite for ccl_devinfo utility
#
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# ##################################### #
# Setup and teardown for each test case #
# ##################################### #

setup() {

	# ccl_devinfo binary
	CCL_DI_COM="@CMAKE_BINARY_DIR@/src/utils/ccl_devinfo"

	# How many devices?
	CCL_DI_NDEVS=`${CCL_DI_COM} | grep -c "\[ Device #"`

	# Device index to use in tests
	if [ -z ${CCL_TEST_DEVICE_INDEX} ]
	then
		CCL_TEST_DEVICE_INDEX="@TESTS_DEVICE_INDEX@"
	fi

	# Minimum OpenCL version between device platform and cf4ocl
	CCL_DI_OCL_VERSION_PLATF=`${CCL_DI_COM} -o -d ${CCL_TEST_DEVICE_INDEX} -c VERSION | grep -o "OpenCL [0-9]\.[0-9]" | cut -d " " -f 2`
	CCL_DI_OCL_VERSION_CF4OCL=`${CCL_DI_COM} --version | grep -o "OpenCL [0-9]\.[0-9]" | cut -d " " -f 2`
	CCL_DI_OCL_VERSION=`echo "if (${CCL_DI_OCL_VERSION_PLATF} < ${CCL_DI_OCL_VERSION_CF4OCL}) { ${CCL_DI_OCL_VERSION_PLATF} } else { ${CCL_DI_OCL_VERSION_CF4OCL} }" | bc`

}

# Test help option, which should return status 0.
@test "Help options" {

	run ${CCL_DI_COM} -?
	[ "$status" -eq 0 ]

	run ${CCL_DI_COM} --help
	[ "$status" -eq 0 ]

}

