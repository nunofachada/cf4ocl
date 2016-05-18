#!/usr/bin/env bash
#
# This script scrapes the OpenCL function names from the OpenCL headers and
# obtains the most recent documentation URLs for each function.
#
# Requires: ctags curl cut head
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2014
#
macros_to_ignore=CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED,CL_EXT_SUFFIX__VERSION_1_2_DEPRECATED,CL_EXT_SUFFIX__VERSION_2_0_DEPRECATED,CL_API_SUFFIX__VERSION_1_0,CL_API_SUFFIX__VERSION_1_1,CL_API_SUFFIX__VERSION_1_2,CL_API_SUFFIX__VERSION_2_0,CL_API_ENTRY,CL_API_CALL
ocl_versions="2.0 1.2 1.1 1.0"
ocl_functions=`LC_ALL=C ctags -I $macros_to_ignore -x --c-kinds=p ../ocl/2.0/CL/* | cut -f1 -d " "`
for ocl_fun in $ocl_functions
do
  for ocl_ver in $ocl_versions
  do
    doc_url="http://www.khronos.org/registry/cl/sdk/${ocl_ver}/docs/man/xhtml/${ocl_fun}.html"
    doc_status=`curl -s --head ${doc_url} | head -n 1 | cut -f2 -d " "`
    if [[ $doc_status == "200" ]]
    then
      echo "${ocl_fun}() ${doc_url}"
      break
    fi
  done
done
