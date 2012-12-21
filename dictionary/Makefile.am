# This file is part of `C5C'.
# Copyright (C) 2002-2005 by Davide Madrisan <davide.madrisan@gmail.com>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of version 2 of the GNU General Public License as published by the
# Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY, to the extent permitted by law; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.

it_IT_listfiles = \
 it_IT.A.txt  \
 it_IT.B.txt  \
 it_IT.C.txt  \
 it_IT.D.txt  \
 it_IT.E.txt  \
 it_IT.F.txt  \
 it_IT.G.txt  \
 it_IT.H.txt  \
 it_IT.I.txt  \
 it_IT.J.txt  \
 it_IT.K.txt  \
 it_IT.L.txt  \
 it_IT.M.txt  \
 it_IT.N.txt  \
 it_IT.O.txt  \
 it_IT.P.txt  \
 it_IT.Q.txt  \
 it_IT.R.txt  \
 it_IT.S.txt  \
 it_IT.T.txt  \
 it_IT.U.txt  \
 it_IT.V.txt  \
 it_IT.W.txt  \
 it_IT.X.txt  \
 it_IT.Y.txt  \
 it_IT.Z.txt  \
 README-it_IT

EXTRA_DIST = $(it_IT_listfiles)

dictionarydir = $(pkgdatadir)
dictionary_DATA = $(it_IT_listfiles)

dictionary-it: dictionary-it-tgz

dictionary-it-tgz: $(it_IT_listfiles)
	$(AMTAR) chof - $(it_IT_listfiles) | \
	   GZIP=$(GZIP_ENV) gzip > $(srcdir)/dict-it_IT.tar.gz

dictionary-it-bz2: $(it_IT_listfiles)
	$(AMTAR) chof - $(it_IT_listfiles) | \
	   bzip2 -9 -c > $(srcdir)/dict-it_IT.tar.bz2
