#!/usr/bin/env bash
#
# Process Doxygen created manpages. Should be executed in the directory
# containing man directories (man1, man3, etc).
#
# 1 - Remove old .gz files
# 2 - Give man files a unique namespace
# 3 - Fix links to account for new namespace and compression
# 4 - Move utilities man pages to man1 (general commands)
# 5 - Compress files
#
# Requires: find sed gzip
#
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2016
#

# 1 - Remove old .gz files
rm -rf man?/*.gz
# 2 - Give man files a unique namespace
find . -type f | sed -n -e 's/.*\.[1-9]/mv & &cf4ocl2/p' | sh
# 3 - Fix links to account for new namespace and compression
find . -type f -exec sed -i -e \
    's/\.so man[1-9]\/[\._1-9a-zA-Z]*/&cf4ocl2.gz/' '{}' \;
# 4 - Move utilities man pages to man1
mkdir -p man1
for util in ccl_c ccl_devinfo ccl_plot_events
do
    # Perform move
    mv man3/${util}.3cf4ocl2 man1/${util}.1cf4ocl2
    # Set current utility man page reference extension to 1 (general commands)
    sed -i "s/\.TH \"${util}\" 3 /.TH \"${util}\" 1 /g" man1/${util}.1cf4ocl2
done

# 5 - Compress files
gzip -r -q .
