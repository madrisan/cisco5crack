#! /bin/sh

# This file is part of `C5C'.
# Copyright (C) 2002-2005 by Davide Madrisan <davide.madrisan@gmail.com>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of version 2 of the GNU General Public License as published by the
# Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.


# this script must be run from the package root folder:
# sh ./config/c5c_files.sh > FILES.md5sum

if expr a : '\(a\)' >/dev/null 2>&1; then
    as_expr=expr
else
    as_expr=false
fi

if (basename /) >/dev/null 2>&1 && test "X`basename / 2>&1`" = "X/"; then
    as_basename=basename
else
    as_basename=false
fi

as_me=`$as_basename "$0" ||
$as_expr X/"$0" : '.*/\([^/][^/]*\)/*$' \| \
          X"$0" : 'X\(//\)$' \| \
          X"$0" : 'X\(/\)$' \| \
                : '\(.\)' 2>/dev/null ||
echo X/"$0" |
    sed '/^.*\/\([^/][^/]*\)\/*$/{ s//\1/; q; }
         /^X\/\(\/\/\)$/{ s//\1/; q; }
         /^X\/\(\/\).*/{ s//\1/; q; }
         s/.*/./; q'`

ac_aux_dir=config

test -f ./$ac_aux_dir/$as_me ||
    { echo "$as_me: error: run me from the package root folder" >&2
    { (exit 1); exit 1; }; }

echo md5sum |
    sed -e :loop -e 's/^.\{1,33\}$/&_/;t loop' -e s/$/filename/

for f in `find ./ -type f |
    sed 's/\.\///
        /\.deps/d
        /\.\(a\|o\)$/d
        /autom4te.cache/d
        /c5c.*\.tar\.\(gz\|bz2\)/d
        /\['$ac_aux_dir']\.\(h\|log\)$/d
        /dictionary\/*/d
        /docs\/md5psdc$/d
        /FILES\.md5sum/d
        /history/d
        /Makefile$/d
        /restore_cc/d
        /score_cc/d
        /stamp-h1/d' | sort`;
    do md5sum $f
done
