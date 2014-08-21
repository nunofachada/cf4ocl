#!/usr/bin/env bash
#
# Scrape the cf4ocl code, get all public functions and macros, check 
# what is tested and what is not.
#
# Requires: ctags curl cut head
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2014
#
macros_to_ignore=G_GNUC_NULL_TERMINATED
ccl_functions=`LC_ALL=C ctags -I $macros_to_ignore -x --c-kinds=pd ../src/cf4ocl2/* | cut -f1 -d " "`
for ccl_fun in $ccl_functions
do
  echo "${ccl_fun}"
done
