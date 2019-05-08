#!/usr/bin/env bash
#
# This script is a filter for Doxygen which adds the respective URLs to OpenCL
# function names, so that OpenCL documentation is easily reacheable from the
# cf4ocl API.
#
# Requires: cut cat sed
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# The script expects an input file
if [ "$#" -eq "0" ]; then
    exit 1
elif [ ! -f "$1" ]; then
    exit 2
fi

# Read from input file
docfile=`cat $1`

# Get script dir
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check if file containing function-URL relations exist
if [ ! -f "${SCRIPT_DIR}/oclfunurls.txt" ]; then
    exit 3
fi

# Load function-URL relations
ocl_funs_urls=`cat ${SCRIPT_DIR}/oclfunurls.txt`

# Get OpenCL function names
ocl_funs=(`echo "$ocl_funs_urls" | cut -f1 -d " "`)

# Get OpenCL function URLs
ocl_urls=(`echo "$ocl_funs_urls" | cut -f2 -d " "`)

# Replace function names with function names and URLs
num_ocl_funs=${#ocl_funs[@]}
for ((i=0; i < $num_ocl_funs; i += 1))
do
    keyword=`echo "${ocl_funs[$i]}"`
    replacement=`echo "[${ocl_funs[$i]}](${ocl_urls[$i]})"`
    docfile=`echo "$docfile" | sed -e "s,${keyword},${replacement},g"`
done
echo "$docfile"

