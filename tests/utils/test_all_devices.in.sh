#!/usr/bin/env bash
#
# Performs cf4ocl utility tests for all available OpenCL devices.
#
# Requires: grep cut bc bats xargs
#
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# ccl_devinfo binary
CCL_DI_COM="@CMAKE_BINARY_DIR@/src/utils/ccl_devinfo"

# How many devices?
CCL_NDEVS=`${CCL_DI_COM} | grep -c "\[ Device #"`

# Perform tests for all devices
for ((CCL_DIDX=0; CCL_DIDX < ${CCL_NDEVS}; CCL_DIDX += 1))
do

	# Get and show device name
	CCL_DNAME=`${CCL_DI_COM} -o -c NAME -d ${CCL_DIDX} | grep " NAME " | cut -f 2 -d "|" | xargs`
	echo " "
	echo "* Performing utility tests for device #${CCL_DIDX} (${CCL_DNAME})"
	echo " "

	# Perform tests for current device
	CCL_TEST_DEVICE_INDEX=${CCL_DIDX} bats @CMAKE_CURRENT_BINARY_DIR@

done
