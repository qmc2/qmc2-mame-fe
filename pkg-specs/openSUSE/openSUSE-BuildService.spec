Name:           qmc2
Version:        0.243
Release:        1
Summary:        M.A.M.E. Catalog / Launcher II
Group:          System/Emulators/Other
License:        GPL-2.0
URL:            http://qmc2.batcom-it.net
Source0:        %{name}-%{version}.tar.bz2

BuildRequires:  libQt5WebKit5-devel
BuildRequires:  libQt5WebKitWidgets-devel
BuildRequires:  libQt5Xml-devel
BuildRequires:  libQt5Sql-devel
BuildRequires:  libQt5Sql5-sqlite
BuildRequires:  libQt5OpenGL-devel
BuildRequires:  libqt5-qtscript-devel
BuildRequires:  libqt5-qtxmlpatterns-devel
BuildRequires:  libqt5-qtsvg-devel
BuildRequires:  libqt5-qtmultimedia-devel
BuildRequires:  libSDL2-devel
BuildRequires:  libarchive-devel
BuildRequires:  make
BuildRequires:  gcc
BuildRequires:  rsync
BuildRequires:  update-desktop-files
BuildRequires:  openSUSE-release
BuildRequires:  fdupes

%description
QMC2 is a Qt based multi-platform GUI front-end for MAME.

%prep
%setup -qcT
tar -xjf %{SOURCE0}
mv qmc2-mame-fe sdlmame
tar -xjf %{SOURCE0}
mv qmc2-mame-fe arcade
tar -xjf %{SOURCE0}
mv qmc2-mame-fe qchdman
tar -xjf %{SOURCE0}
mv qmc2-mame-fe manpages

%build
pushd sdlmame
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake-qt5 DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} JOYSTICK=1 MULTIMEDIA=1 SDL=2 LIBARCHIVE=1 WIP=0 CXX_FLAGS=-O3 CC_FLAGS=-O3 qmc2
popd

pushd arcade
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake-qt5 DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} JOYSTICK=1 SDL=2 LIBARCHIVE=1 WIP=0 CXX_FLAGS=-O3 CC_FLAGS=-O3 arcade
popd

pushd qchdman
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake-qt5 DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} WIP=0 CXX_FLAGS=-O3 CC_FLAGS=-O3 qchdman
popd

pushd manpages
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake-qt5 DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} man
popd

%install
rm -rf $RPM_BUILD_ROOT

pushd sdlmame
make install QMAKE=%{_prefix}/bin/qmake-qt5 DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} JOYSTICK=1 MULTIMEDIA=1 SDL=2 LIBARCHIVE=1 WIP=0
popd

pushd arcade
make arcade-install QMAKE=%{_prefix}/bin/qmake-qt5 DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} JOYSTICK=1 SDL=2 LIBARCHIVE=1 WIP=0
popd

pushd qchdman
make qchdman-install QMAKE=%{_prefix}/bin/qmake-qt5 DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} WIP=0
popd

pushd manpages
make man-install QMAKE=%{_prefix}/bin/qmake-qt5 DESTDIR=$RPM_BUILD_ROOT MAN_DIR=/usr/share/man DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir}
popd

# manually install doc files in order to avoid "files-duplicate" warning
install -dp -m 0755 $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}
cp -a sdlmame/data/doc/html/ $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}/

# symlink duplicate files
%fdupes -s $RPM_BUILD_ROOT/usr/share

# update the desktop files
%suse_update_desktop_file -i %{name}-sdlmame Game ArcadeGame
%suse_update_desktop_file -i %{name}-arcade Game ArcadeGame
%suse_update_desktop_file -i qchdman Game ArcadeGame

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
* Tue Apr  5 2022 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.243
- updated spec to QMC2 0.243

* Sat Apr  2 2022 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.242
- updated spec to QMC2 0.242

* Thu Mar  1 2018 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.196
- updated spec to QMC2 0.196

* Thu Mar  1 2018 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.195-2
- switched to using Qt 5

* Wed Feb 28 2018 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.195-1
- updated spec to QMC2 0.195
