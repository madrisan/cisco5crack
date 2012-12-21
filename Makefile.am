# This file is part of `C5C`.
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
# You should have received a copy of the GNU General Public License along  with
# this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.


# auxdir = @ac_aux_dir@
auxdir = config

AUTOMAKE_OPTIONS = 1.8 check-news dist-bzip2 gnu nostdinc no-dist-gzip
ACLOCAL_AMFLAGS = -I $(auxdir)

SUBDIRS = dictionary $(auxdir) docs src

FILES_MD5SUM = FILES.md5sum
EXTRA_DIST = BUGS $(FILES_MD5SUM) README.EGD VERSION \
             $(PACKAGE_NAME).spec.in $(PACKAGE_NAME).spec

# automatically generate the md5sum-list `FILES.md5sum', and the RPM spec file
# `$(PACKAGE_NAME).spec' from `$(PACKAGE_NAME).spec.in'
dist-update: $(top_builddir)/config.status Makefile \
             $(srcdir)/$(FILES_MD5SUM) $(srcdir)/$(PACKAGE_NAME).spec distcheck

.PHONY: $(srcdir)/$(FILES_MD5SUM)
$(srcdir)/$(FILES_MD5SUM):
	cd $(top_builddir) && $(SHELL) ./$(auxdir)/manifest.sh > $(FILES_MD5SUM)

.PHONY: $(srcdir)/$(PACKAGE_NAME).spec
$(srcdir)/$(PACKAGE_NAME).spec: $(srcdir)/$(PACKAGE_NAME).spec.in \
                    $(top_builddir)/config.status
	cd $(top_builddir) && $(SHELL) ./config.status --file=$@ < /dev/null

dist-rpm: dist-update
	test -e $(srcdir)/$(PACKAGE_NAME).spec && \
	  rpm_release=`sed -n '/Release/{s/.*:[ ]*//;p}' $(srcdir)/$(PACKAGE_NAME).spec` || \
	   { echo "fatal error: cannot find $(srcdir)/$(PACKAGE_NAME).spec" >&2; \
	     exit 1; }; \
	rpm_name=$(PACKAGE_NAME)-$(PACKAGE_VERSION)-$$rpm_release; \
	rpm_sourcedir=`rpm --eval=%{_sourcedir}`; \
	rpm_srcrpmdir=`rpm --eval=%{_srcrpmdir}`; \
	(cp $(srcdir)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.bz2 $$rpm_sourcedir && \
	  rpm -ba --target $(host_triplet) $(srcdir)/$(PACKAGE_NAME).spec) || exit 1; \
	echo "Moving the generated rpm and srpm packages to $(srcdir)/:"; \
	find `rpm --eval=%{_srcrpmdir}` -name $$rpm_name\*src.rpm -printf "  %h/%f\n" \
	  -exec mv {} $(srcdir) ';'; \
	find `rpm --eval=%{_rpmdir}` -name $$rpm_name\*.*rpm -printf "  %h/%f\n" \
	  -exec mv {} $(srcdir) ';'; \
	echo "Removing temporary files..."; \
	$(RM) -r `rpm --eval=%{_builddir}`/$(PACKAGE_NAME)-$(PACKAGE_VERSION); \
	$(RM) $$rpm_sourcedir/$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.bz2; \
	echo "All done. Enjoy..."
