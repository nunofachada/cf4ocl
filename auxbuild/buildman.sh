#!/usr/bin/env bash
#
# Process Doxygen created manpages. Should be executed in the directory
# containing man directories (man1, man3, etc).
#
# 1 - Remove old .gz files
# 2 - Give man files a unique namespace
# 3 - Fix links to account for new namespace and compression
# 4 - Compress files
#
# Requires: find sed sh gzip
#
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2014
#

# 1 - Remove old .gz files
rm -rf man?/*.gz
# 2 - Give man files a unique namespace
find . -type f | sed -n 's/.*[1-9]/mv & &cf4ocl2/p' | sh
# 3 - Fix links to account for new namespace and compression
find . -type f -exec sed -i 's/\.so man[1-9]\/[\._1-9a-zA-Z]*/&cf4ocl2.gz/' '{}' \;
# 4 - Compress files
gzip -r -q .
