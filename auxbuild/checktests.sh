#!/usr/bin/env bash
#
# Scrape the cf4ocl code, get all public functions and macros, check
# what is tested (and where) and what is not.
#
# Tested functions (and the test functions which call them) are printed
# to stdout. Untested functions are printed to stderr.
#
# Requires: ctags cut grep awk find cflow
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# Get all cf4ocl functions
macros_to_ignore=G_GNUC_NULL_TERMINATED
ccl_functions=`LC_ALL=C ctags -I $macros_to_ignore -x --c-kinds=pd ../src/lib/*.h | cut -f1 -d " " | grep '^[^A-Z_]'`

# Get all tests source files
test_srcs=`find ../tests/lib/ -type f -iname "*test*"`

# For each cf4ocl function, get test functions which call it
declare -A fun_table
for ccl_fun in $ccl_functions
do
	for test_src in $test_srcs
	do
		callers=`cflow $test_src | awk '/^    [^ ]/ { caller=$1 }; /^.*'$ccl_fun'/ { printf caller " " }'`
		fun_table[${ccl_fun}]+=$callers
	done
done

# Print cf4ocl functions and respective test caller functions
untested_funs=0
for ccl_fun in $ccl_functions
do
	if [[ -n ${fun_table[$ccl_fun]} ]]
	then
		printf "$ccl_fun\n"
		for caller in ${fun_table[$ccl_fun]}
		do
			printf "\t$caller\n"
		done
	else
		untested_funs=$(($untested_funs+1))
		printf "$ccl_fun\n" 1>&2
	fi
done
printf "\n SUMMARY: cf4ocl has ${#fun_table[@]} functions, $untested_funs of which are untested.\n"
