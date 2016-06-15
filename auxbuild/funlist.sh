#!/usr/bin/env bash
#
# Create a Doxygen page with a list of cf4ocl functions and function-like
# macros.
#
# Requires: ctags cut grep awk find cflow
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# Get all cf4ocl functions
macros_to_ignore=G_GNUC_NULL_TERMINATED
ccl_functions=`LC_ALL=C ctags -I $macros_to_ignore -x --c-kinds=pd ../src/lib/ccl_*.h | cut -f1 -d " " | grep '^[^A-Z_]'`

# Remove previous page with function list, if any
echo "Function list {#funlist}" > ../docs/funlist.md
echo "=================" >> ../docs/funlist.md
echo "" >> ../docs/funlist.md
echo "@brief List of _cf4ocl_ functions and function-like macros." >> \
	../docs/funlist.md
echo "" >> ../docs/funlist.md

for ccl_fun in $ccl_functions
do
	if [[ $ccl_fun == "ccl_"* ]]
	then
		echo "* ::${ccl_fun}() - @copybrief ${ccl_fun}" >> ../docs/funlist.md
	fi
done


