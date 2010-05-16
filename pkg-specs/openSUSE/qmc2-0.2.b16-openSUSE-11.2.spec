Name:           qmc2
Version:        0.2.b16
Release:        1
Summary:        M.A.M.E./M.E.S.S. Catalog / Launcher II
Distribution:   openSUSE 11.2
Group:          Applications/Emulators
License:        GPLv2
URL:            http://qmc2.arcadehits.net/wordpress
Source0:        http://dl.sourceforge.net/qmc2/%{name}-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  libqt4-devel
BuildRequires:  libkde4-devel
BuildRequires:  SDL-devel
BuildRequires:  make
BuildRequires:  gcc
BuildRequires:  rsync

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
    PRETTY=0 PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMESS JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0
popd

pushd sdlmame
make %{?_smp_mflags} QMAKE=%{_prefix}/bin/qmake CTIME=0 DISTCFG=1\
    PRETTY=0 PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMAME JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0
popd

%install
rm -rf $RPM_BUILD_ROOT

pushd sdlmess
make install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PRETTY=0 CTIME=0 PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMESS JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0
popd

# remove the old qmc2.ini since we only need one
rm -f $RPM_BUILD_ROOT%{_sysconfdir}/qmc2/qmc2.ini

pushd sdlmame
make install QMAKE=%{_prefix}/bin/qmake DESTDIR=$RPM_BUILD_ROOT DISTCFG=1 \
    PRETTY=0 CTIME=0 PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir} \
    EMULATOR=SDLMAME JOYSTICK=1 PHONON=1 WIP=0 OPENGL=0
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
* Sun May 16 2010 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b16-1
- Updated spec to 0.2.b16

* Fri Mar 12 2010 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b15-1
- Updated spec to 0.2.b15

* Tue Jan 19 2010 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b14-1
- Updated spec to 0.2.b14

* Sat Jan 02 2010 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b13-1
- Updated spec to 0.2.b13

* Sun Nov 13 2009 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b12-1
- Updated spec to openSUSE 11.2 (first officially supported openSUSE release)

* Sun Nov 01 2009 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b12-1
- Updated spec to 0.2.b12 final

* Fri Sep 11 2009 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b11-1
- Updated spec to 0.2.b11 final

* Tue Jul 21 2009 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b10-1
- Updated spec to 0.2.b10 final

* Tue Jun 09 2009 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b9-1
- Updated spec to 0.2.b9 final

* Sun May 17 2009 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b9-SVN-3
- Merged the three packages into one

* Fri May 15 2009 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b9-SVN-2
- Added more precise (sub-)package descriptions

* Mon May 04 2009 R. Reucher <rene[dot]reucher[at]batcom-it[dot]net> - 0.2.b9-SVN-1
- Initial version of this spec based on Julian Sikorski's Fedora spec file
