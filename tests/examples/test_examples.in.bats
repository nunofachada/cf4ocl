
#!/usr/bin/env bats
#
# Test suite for cf4ocl examples
#
# Requires: wc
#
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# ##################################### #
# Setup and teardown for each test case #
# ##################################### #

setup() {

	# Examples binary path
	CCL_EXBIN_PATH="@CMAKE_BINARY_DIR@/src/examples"

	# Device index to use in tests
	if [ -z ${CCL_TEST_DEVICE_INDEX} ]
	then
		CCL_TEST_DEVICE_INDEX="@TESTS_DEVICE_INDEX@"
	fi

}

# ##### #
# Tests #
# ##### #

# Test canonical example
@test "Canonical example" {

	run ${CCL_EXBIN_PATH}/canon ${CCL_TEST_DEVICE_INDEX}

	# Check output
	[[ "$output" =~  "Kernel execution produced the expected results." ]]

	# There should be no problems
	[ "$status" -eq 0 ]

}

# Test celullar automata example
@test "Cellular automata example" {

	run ${CCL_EXBIN_PATH}/ca ${CCL_TEST_DEVICE_INDEX}

	# There should be no problems
	[ "$status" -eq 0 ]

	# Check if images files were created
	CCL_EXCA_NIMGS=`ls out000*.png -la | wc -l`
	[ "$CCL_EXCA_NIMGS" -eq 64 ]
	rm -rf out000*.png

}

# Test device filter example
@test "Device filter example" {

	# Test create context with filtered devices
	run ${CCL_EXBIN_PATH}/device_filter 1

	# There should be no problems
	[ "$status" -eq 0 ]

	# Test listing of filtered devices
	run ${CCL_EXBIN_PATH}/device_filter 2

	# There should be no problems
	[ "$status" -eq 0 ]

}

# Test list devices example
@test "List devices example" {

	run ${CCL_EXBIN_PATH}/list_devices

	# There should be no problems
	[ "$status" -eq 0 ]

}

# Test image fill example
@test "Image fill example" {

	run ${CCL_EXBIN_PATH}/image_fill ${CCL_TEST_DEVICE_INDEX}

	# There should be no problems
	[ "$status" -eq 0 ]

}


# Test image filter example
@test "Image filter example" {

	run ${CCL_EXBIN_PATH}/image_filter \
		@CMAKE_SOURCE_DIR@/images/gantt_ca.png ${CCL_TEST_DEVICE_INDEX}

	# There should be no problems
	[ "$status" -eq 0 ]

	# Check that output image was created
	[ -f out.png ]
	rm -rf out.png

}
