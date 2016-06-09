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

	# How many platforms
	CCL_DI_NPLATFS=`${CCL_DI_COM} | grep -c "Platform #"`

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

# Test help options
@test "Help and version options" {

	run ${CCL_DI_COM} -?
	[ "$status" -eq 0 ]

	run ${CCL_DI_COM} -h
	[ "$status" -eq 0 ]

	run ${CCL_DI_COM} --help
	[ "$status" -eq 0 ]

	run ${CCL_DI_COM} --version
	[ "$status" -eq 0 ]

}

# Test list option
@test "List options" {

	run ${CCL_DI_COM} --list
	[ "$status" -eq 0 ]

}

# Test verbose option
@test "Verbose option" {

	run ${CCL_DI_COM} --verbose
	[ "$status" -eq 0 ]

}


# Test no options (default behavior)
@test "No options (default behavior)" {

	run ${CCL_DI_COM}
	[ "$status" -eq 0 ]

}

# Test if number of devices in index zero is the same as the number of platforms
@test "Is number of devices in index 0 the same as the number of platforms?" {

	CCL_DI_NDEVS_IDX0=`${CCL_DI_COM} -d 0 | grep -c "\[ Device #0"`
	[ ${CCL_DI_NPLATFS} -eq ${CCL_DI_NDEVS_IDX0} ]

}

# Test all/notfound options
@test "All/notfound options" {

	run ${CCL_DI_COM} --all
	[ "$status" -eq 0 ]

	run ${CCL_DI_COM} --all --notfound
	[ "$status" -eq 0 ]

}

# Test no-platf option, check that number of devices is the same, check each
# device individually
@test "No-platf option" {

	CCL_DI_NDEVS_NOPLATF=`${CCL_DI_COM} --no-platf | grep -c "\[ Device #"`
	[ ${CCL_DI_NDEVS} -eq ${CCL_DI_NDEVS_NOPLATF} ]

}

# Test custom information option
@test "Custom information option" {

	run ${CCL_DI_COM} --no-platf -c NAME -c IMAGE_SUPPORT -c VERSION \
		-d ${CCL_TEST_DEVICE_INDEX}
	[ "$status" -eq 0 ]

}

# Test for all devices in each platform individually (-p option)
@test "All devices in each platform (-p option)" {

	for ((CCL_DI_PIDX=0; CCL_DI_PIDX < ${CCL_DI_NPLATFS}; CCL_DI_PIDX += 1))
	do
		run ${CCL_DI_COM} -p ${CCL_DI_PIDX}
		[ "$status" -eq 0 ]
	done

}

# Test for individual devices in each platform individually (-p and -d options)
@test "Individual devices in each platform (-p and -d options)" {

	CCL_DI_TOTPNDEVS=0
	for ((CCL_DI_PIDX=0; CCL_DI_PIDX < ${CCL_DI_NPLATFS}; CCL_DI_PIDX += 1))
	do
		CCL_DI_PNDEVS=`${CCL_DI_COM} -p ${CCL_DI_PIDX} | grep  -c "\[ Device #"`
		let "CCL_DI_TOTPNDEVS=CCL_DI_TOTPNDEVS+CCL_DI_PNDEVS"
		for ((CCL_DI_DIDX=0; CCL_DI_DIDX < ${CCL_DI_PNDEVS}; CCL_DI_DIDX += 1))
		do
			run ${CCL_DI_COM} -p ${CCL_DI_PIDX} -d ${CCL_DI_DIDX}
			[ "$status" -eq 0 ]
		done
	done
	[ ${CCL_DI_TOTPNDEVS} -eq ${CCL_DI_NDEVS} ]

}
