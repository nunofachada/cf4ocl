#!/usr/bin/env bash
#
# Performs all cf4ocl tests for all available OpenCL devices.
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
    echo "** "
    echo "** Performing tests for device #${CCL_DIDX} (${CCL_DNAME})"
    echo "** "

    echo " "
    echo "=> Utility tests"
    echo " "

    # Perform utility tests for current device
    CCL_TEST_DEVICE_INDEX=${CCL_DIDX} bats @CMAKE_CURRENT_BINARY_DIR@/utils

    echo " "
    echo "=> Examples tests"
    echo " "

    # Perform example tests for current device
    CCL_TEST_DEVICE_INDEX=${CCL_DIDX} bats @CMAKE_CURRENT_BINARY_DIR@/examples

    echo " "
    echo "=> Library tests"
    echo " "

    # Perform library tests for current device
    CCL_TEST_DEVICE_INDEX=${CCL_DIDX} gtester -k \
        @CMAKE_CURRENT_BINARY_DIR@/lib/test_*

done
