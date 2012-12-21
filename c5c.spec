# c5c.spec.  Generated from c5c.spec.in by configure.
#
# c5c.spec -- RPM specification for c5c package
# This file is part of 'c5c'.
# Copyright (C) 2002-2005 Davide Madrisan <davide.madrisan@gmail.com>
#
# This is a specification file for the RedHat Package Manager (RPM).
# It is part of the c5c source tree and this way directly included
# in c5c distribution tarballs.  This way one can use a simple
#   rpm -tb c5c-1.1.5.tar.gz
# command to build binary RPM packages from a original distribution tarball.

# note:  rpm -ba --target i686-pc-linux-gnu c5c.spec

%{!?gccmarch: %define gccmarch i586}
%define c5cgrp c5c
%define localstatedir /var

Name:          c5c
Version:       1.1.5
Release:       1qilnx
Summary:       A tool to decrypt Cisco MD5 passwords (IOS and PIX)
Group:         Applications/Security
Distribution:  QiLinux
Vendor:        QiLinux
Packager:      Davide Madrisan <davide.madrisan@gmail.com>
URL:           http://www.qilinux.it
Source:        http://ftp.qilinux.it/pub/QiLinux/devel/tools/%{name}-%{version}.tar.bz2
License:       GPL
BuildRoot:     %{_tmppath}/%{name}-%{version}-build

# ExcludeArch: alpha
# ExcludeOS:

%description
c5c is a command line tool that can be used to try an offline
attack to the MD5 passwords used in the Cisco devices running IOS and the
Cisco PIX firewalls.  Its primary purpose is to help system administrators
to detect weak passwords.  This program supports bruteforce attacks bound
by a user defined regular expression, and the vocabulary attack.

%prep
%setup -q

%build
[ -e configure ] || ./conf/bootstrap.sh
export CFLAGS="-march=%{gccmarch} -O2"
#export CC="diet gcc"
./configure \
   --prefix=%{_prefix} \
   --enable-dev-random \
   --enable-egd-support \
   --enable-shortpasswd
#  --enable-gcc-warnings

%{__make}

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"
install -d %{buildroot}%{localstatedir}/log/%{name}
%makeinstall

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%pre
/usr/sbin/groupadd %{c5cgrp} &>/dev/null || :

%preun
if [ $1 = 0 ]; then
   /usr/sbin/groupdel %{c5cgrp} &>/dev/null
fi
exit 0

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%dir %attr(770,-,%{c5cgrp}) %{localstatedir}/log/%{name}
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/*
%doc AUTHORS BUGS ChangeLog COPYING NEWS TODO README README.EGD
%doc docs/{*.txt,md5psdc.tex}

%changelog
* Sat Dec 11 2004 Davide Madrisan <davide.madrisan@gmail.com> 1.1.5qilnx
- specfile updates and cleanups
- updated to 1.1.5

* Sat Nov 11 2003 Davide Madrisan <davide.madrisan@gmail.com> 1.1.3
- spec file updates

* Thu Sep 04 2003 Davide Madrisan <davide.madrisan@gmail.com> 1.1.2
- added the italian dictionary in the list of %doc's

* Wed Jul 23 2003 Davide Madrisan <davide.madrisan@gmail.com>
- source is now a bz2 compressed tarball (%{name}-%{version}.tar.bz2),
- added support for snapshots generation in the build section.

* Sun Jun 29 2003 Davide Madrisan <davide.madrisan@gmail.com>
- spec file converted to a spec.in file to be automatically updated by make
  (i.e. program name and version are updated automatically).

* Wed May 21 2003 Davide Madrisan <davide.madrisan@gmail.com>
- added check to ensure that %{buildroot} is not "/" before rm'ing it,
- no need to create dirs in the install section,
  "make prefix=%{buildroot}%{prefix} install" does this,
- quiet setup,
- added docs.

* Sat May 17 2003 Davide Madrisan <davide.madrisan@gmail.com>
- general cleanups,
- update for release 1.0.2,
- strip after files have been installed into buildroot.

* Sat Mar 15 2003 Davide Madrisan <davide.madrisan@gmail.com>
- initial build (for release 1.0.0).
