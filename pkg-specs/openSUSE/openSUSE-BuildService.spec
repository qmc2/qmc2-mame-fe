Name:           qmc2
Version:        0.183
Release:        1
Summary:        M.A.M.E. Catalog / Launcher II
Group:          System/Emulators/Other
License:        GPL-2.0
URL:            http://qmc2.batcom-it.net
Source0:        http://dl.sourceforge.net/qmc2/%{name}-%{version}.tar.bz2

BuildRequires:  libqt4-devel
BuildRequires:  libkde4-devel
BuildRequires:  libqt4-x11
BuildRequires:  libSDL2-devel
BuildRequires:  make
BuildRequires:  gcc
BuildRequires:  rsync
BuildRequires:  desktop-file-utils
BuildRequires:  openSUSE-release
BuildRequires:  fdupes
BuildRequires:  libarchive-devel

%description
QMC2 is a Qt based multi-platform GUI front-end for MAME.

%prep
%setup -qcT
tar -xjf %{SOURCE0}
mv %{name} sdlmame
tar -xjf %{SOURCE0}
mv %{name} arcade
tar -xjf %{SOURCE0}
mv %{name} qchdman
tar -xjf %{SOURCE0}
mv %{name} manpages

%build
pushd sdlmame
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} JOYSTICK=1 PHONON=1 SDL=2 LIBARCHIVE=1 WIP=0 CXX_FLAGS=-O3 CC_FLAGS=-O3 qmc2
popd

pushd arcade
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} JOYSTICK=1 SDL=2 LIBARCHIVE=1 WIP=0 CXX_FLAGS=-O3 CC_FLAGS=-O3 arcade
popd

pushd qchdman
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} WIP=0 CXX_FLAGS=-O3 CC_FLAGS=-O3 qchdman
popd

pushd manpages
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} man
popd

%install
rm -rf $RPM_BUILD_ROOT

pushd sdlmame
make install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} JOYSTICK=1 PHONON=1 SDL=2 LIBARCHIVE=1 WIP=0
popd

pushd arcade
make arcade-install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} JOYSTICK=1 SDL=2 LIBARCHIVE=1 WIP=0
popd

pushd qchdman
make qchdman-install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} WIP=0
popd

pushd manpages
make man-install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT MAN_DIR=/usr/share/man DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir}
popd

# manually install doc files in order to avoid "files-duplicate" warning
install -dp -m 0755 $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}
cp -a sdlmame/data/doc/html/ $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}/

# symlink duplicate files
%fdupes -s $RPM_BUILD_ROOT/usr/share

# update the desktop files
%suse_update_desktop_file %{name}-sdlmame Game ArcadeGame
%suse_update_desktop_file %{name}-arcade Game ArcadeGame
%suse_update_desktop_file qchdman Game ArcadeGame

# make sure the executable permissions are set correctly
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qmc2-sdlmame
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qmc2-arcade
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qchdman

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc %{_defaultdocdir}/%{name}/
%config(noreplace) %{_sysconfdir}/qmc2
%{_datadir}/qmc2
%{_bindir}/qmc2
%{_bindir}/qmc2-sdlmame
%{_bindir}/qmc2-arcade
%{_bindir}/qchdman
%{_datadir}/applications/qmc2-sdlmame.desktop
%{_datadir}/applications/qmc2-arcade.desktop
%{_datadir}/applications/qchdman.desktop
%{_mandir}/man6/qmc2-main-gui.6.gz
%{_mandir}/man6/qmc2.6.gz
%{_mandir}/man6/qmc2-sdlmame.6.gz
%{_mandir}/man6/qmc2-arcade.6.gz
%{_mandir}/man6/qchdman.6.gz

%changelog
* Thu Jan 26 2017 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.183-1
- updated spec to QMC2 0.183 (changed versioning scheme to match MAME's version)

* Thu Dec  1 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.71-1
- updated spec to QMC2 0.71

* Thu Oct 27 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.70-1
- updated spec to QMC2 0.70

* Wed Sep 28 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.69-1
- updated spec to QMC2 0.69

* Thu Sep  1 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.68-1
- updated spec to QMC2 0.68

* Wed Jul 27 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.67-1
- updated spec to QMC2 0.67

* Fri Jul  1 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.66-1
- updated spec to QMC2 0.66

* Thu May 26 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.65-1
- updated spec to QMC2 0.65

* Thu Apr 28 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.64-1
- updated spec to QMC2 0.64

* Wed Mar 30 2016 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.63-1
- updated spec to QMC2 0.63
