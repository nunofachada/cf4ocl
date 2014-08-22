#!/usr/bin/env bash
#
# Check if any function declaration exists without its definition.
#
# Requires: ctags cut grep sort cut git
# Author: Nuno Fachada <faken@fakenmc.com>
# Licence: GNU General Public License version 3 (GPLv3)
# Date: 2014
#

defs=`LC_ALL=C ctags -I G_GNUC_NULL_TERMINATED -x --c-kinds=f ../src/lib/*.c | grep -v "static" | sort -n -s | cut -f1 -d " "`
decls=`LC_ALL=C ctags -I G_GNUC_NULL_TERMINATED -x --c-kinds=p ../src/lib/*.h | grep -v "static" | sort -n -s | cut -f1 -d " "`
git diff --no-index -- <(printf "$defs") <(printf "$decls")

