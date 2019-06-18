#!/usr/bin/env bash
#
# Toggle system ICDs on and off. If no ICDs are given, all disabled ICDs are
# enabled. Otherwise given ICDs are enabled and the rest are disabled.
#
# **Examples:**
#
# ```
# # Enable only pocl, disable other ICDs
# $ ./icdtoggle.sh pocl
# # Enable all ICDs previously disabled with this script
# $ ./icdtoggle.sh
# ```
#
# Requires: sudo
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2019
#

# ICDs path
icdpath=/etc/OpenCL/vendors
offsuf=.off

# Check args
if [ "$#" -eq "0" ]; then
    # No args? then enable all disabled ICDs

    # Obtain all disabled ICDs
    icdsoff=$(ls ${icdpath}/*.icd${offsuf})

    # For each disabled ICD...
    for icdoff in $icdsoff
    do
        # Enable it
        icdon=${icdoff%${offsuf}}
        echo "Enabled ${icdon}"
        sudo mv $icdoff $icdon
    done

else
    # Args were passed, keep only the respective ICDs enabled

    # Obtain all enabled ICDs
    icdson=$(ls ${icdpath}/*.icd)

    # Cycle through all enabled ICDs
    for icdon in $icdson
    do

        # Assume current ICD is to be disabled
        todisable=1

        # Check if we should not disable the current ICD
        for arg in "$@"
        do

            # Add ".icd" to current arg
            icd="${icdpath}/${arg}.icd"

            # Is current arg the same as the current enabled ICD?
            if [ "$icd" == "$icdon" ]; then

                # Do not disable current ICD and get out of loop
                todisable=0
                break
            fi

        done

        # Should we disable current ICD?
        if [ "$todisable" == "1" ]; then

            # Disable current ICD
            echo "Disabled ${icdon}"
            sudo mv ${icdon} ${icdon}${offsuf}
        fi

    done

fi