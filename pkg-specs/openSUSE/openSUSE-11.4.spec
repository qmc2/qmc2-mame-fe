Name:           qmc2
Version:        0.35
Release:        1
Summary:        M.A.M.E./M.E.S.S. Catalog / Launcher II
Distribution:   openSUSE 11.4
Group:          System/Emulators/Other
License:        GPL-2
URL:            http://qmc2.arcadehits.net/wordpress
Source0:        http://dl.sourceforge.net/qmc2/%{name}-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  libqt4-devel
BuildRequires:  libkde4-devel
BuildRequires:  libqt4-x11
BuildRequires:  SDL-devel
BuildRequires:  make
BuildRequires:  gcc
BuildRequires:  rsync
BuildRequires:  desktop-file-utils
BuildRequires:  openSUSE-release

%description
QMC2 is a Qt 4 based multi-platform GUI front end for several MAME and MESS variants.

%prep
%setup -qcT
tar -xjf %{SOURCE0}
mv %{name} sdlmame
tar -xjf %{SOURCE0}
mv %{name} sdlmess

%build
pushd sdlmess
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake CTIME=0 DISTCFG=1\
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMESS JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

pushd sdlmame
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake CTIME=0 DISTCFG=1\
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMAME JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

%install
rm -rf $RPM_BUILD_ROOT

pushd sdlmess
make install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMESS JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

# remove the old qmc2.ini since we only need one
rm -f $RPM_BUILD_ROOT%{_sysconfdir}/qmc2/qmc2.ini

pushd sdlmame
make install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMAME JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0 \
    CXX_FLAGS=-O3 CC_FLAGS=-O3
popd

# validate the desktop files
desktop-file-validate $RPM_BUILD_ROOT%{_datadir}/applications/qmc2-sdlmame.desktop
desktop-file-validate $RPM_BUILD_ROOT%{_datadir}/applications/qmc2-sdlmess.desktop

# make sure the executable permissions are set correctly
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qmc2-sdlmame
chmod 755 $RPM_BUILD_ROOT%{_bindir}/qmc2-sdlmess
chmod 755 $RPM_BUILD_ROOT%{_bindir}/runonce

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc sdlmame/data/doc/html
%config(noreplace) %{_sysconfdir}/qmc2
%{_bindir}/runonce
%{_datadir}/qmc2
%{_bindir}/qmc2
%{_bindir}/qmc2-sdlmame
%{_bindir}/qmc2-sdlmess
%{_datadir}/applications/qmc2-sdlmame.desktop
%{_datadir}/applications/qmc2-sdlmess.desktop

%changelog
* Tue Nov 15 2011 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.35-1
- updated spec to QMC2 0.35

* Wed Jun 29 2011 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.34-1
- updated spec to QMC2 0.34

* Sun Mar 03 2011 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b20-1
- updated spec to QMC2 0.2.b20

* Tue Mar 23 2011 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b19-1
- created spec file for openSUSE 11.4 (copy from the spec for 11.3)
