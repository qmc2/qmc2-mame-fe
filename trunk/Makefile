# >>> START OF MAKE OPTIONS <<<
#
# You can either edit the default values below or (recommended) specify them on
# the 'make' command line (non-string options could alternatively be defined as
# environment variables).
#
# Please don't change anything below the line containing END OF MAKE OPTIONS!
#

# >>> FORCE_MINGW <<<
#
# Enable (1) or disable (0) support for the MinGW (GCC) compiler on Windows.
#
# When this option has not been specified, OS auto-detection will take place,
# forcing the use of MinGW when the OS is found to be 'Windows_NT'.
#
# Notes:
#
# Using this option on any other OS than Windows will make the build fail! Also,
# you'll need a working MinGW GCC installation (incl. Qt and SDL). This option
# will enable some assumptions with regard to these requirements!
#
# This is NOT for users of MS Visual C++ 2010!
#
# For detailed build instructions on Windows, see our Wiki:
# http://wiki.batcom-it.net/index.php?title=The_%27ultimate%27_guide_to_QMC2#Windows
#
ifndef FORCE_MINGW
ifeq ($(OS),Windows_NT)
FORCE_MINGW = 1
else
FORCE_MINGW = 0
endif
endif

# >>> AUDIOEFFECTDIALOGS <<<
#
# Enable (1) or disable (0) support for audio-effect dialogs.
#
ifndef AUDIOEFFECTDIALOGS
ifeq '$(FORCE_MINGW)' '1'
AUDIOEFFECTDIALOGS = 0
else
AUDIOEFFECTDIALOGS = 1
endif
endif

# >>> PREFIX <<<
#
# The prefix directory used by the 'make install' target.
#
ifndef PREFIX
ifeq '$(FORCE_MINGW)' '1'
PREFIX = .
else
PREFIX = /usr/local
endif
endif

# >>> QUIET <<<
#
# Compile and link QMC2 'quietly' (1) or show all command lines completely (0)?
#
ifndef QUIET
QUIET = 0
endif

# >>> CTIME <<<
#
# Decides if compilation time is measured (1) or not (0).
#
# Of course, you could as well use 'time make ...' :).
#
ifndef CTIME
CTIME = 0
endif

# >>> DEBUG <<<
#
# Choose debugging options:
#
# 0 .... Generate no debugging code at all (default, recommended)
# 1 .... Add symbols for the debugger, show compiler warnings (ok)
# 2 .... Add symbols for the debugger, show compiler warnings and include QMC2's
#        debugging code (not recommended, only for developers or if explicitly
#        asked to do so by a developer)
#
ifndef DEBUG
DEBUG = 0
endif

# >>> ARCH <<<
#
# Target system's OS name -- this should generally not be changed, only if you
# want to compile a specific OS's code branch or if the 'uname' command doesn't
# tell the correct OS name of your system (see also OSREL and MACHINE!).
#
ifndef ARCH
ifeq '$(FORCE_MINGW)' '1'
ARCH = Windows
else
ARCH = $(shell uname)
endif
endif

# >>> OSREL <<<
#
# Target system's OS-release -- please only change this if you really know what
# you're doing :)!
#
ifndef OSREL
ifeq '$(FORCE_MINGW)' '1'
OSREL = $(shell arch\Windows\osrel.bat)
else
OSREL = $(shell uname -r)
endif
endif

# >>> MACHINE <<<
#
# Target system's machine/CPU type -- please only change this if you really know
# what you're doing :)!
#
ifndef MACHINE
ifeq '$(FORCE_MINGW)' '1'
MACHINE = $(shell gcc -dumpmachine)
else
MACHINE = $(shell uname -m)
endif
endif

# >>> DATADIR <<<
#
# The data directory used by the 'make install' target.
#
ifndef DATADIR
ifeq '$(ARCH)' 'Darwin'
DATADIR = /Library/Application Support
else
ifeq '$(FORCE_MINGW)' '1'
DATADIR = data
else
DATADIR = $(PREFIX)/share
endif
endif
endif

# >>> SYSCONFDIR <<<
#
# The system configuration directory used by the 'make install' target.
#
ifndef SYSCONFDIR
ifeq '$(ARCH)' 'Darwin'
SYSCONFDIR = /Library/Application Support
else
ifeq '$(FORCE_MINGW)' '1'
SYSCONFDIR =
else
SYSCONFDIR = /etc
endif
endif
endif

# >>> BINDIR <<<
#
# The directory where binaries / application bundles will be installed to (used
# by the 'make install' target).
#
ifndef BINDIR
ifeq '$(ARCH)' 'Darwin'
BINDIR = /Applications
else
ifeq '$(FORCE_MINGW)' '1'
BINDIR = .
else
BINDIR = $(PREFIX)/bin
endif
endif
endif

# >>> OSCFG <<<
#
# Use the global OS build configuration file (1) or not (0)?
#
ifndef OSCFG
OSCFG = 1
endif

# >>> DISTCFG <<<
#
# Use the distribution-specific build configuration (1) or not (0)?
#
ifndef DISTCFG
DISTCFG = 0
endif

# >>> IMGSET <<<
#
# Select the image set to be used (see data/img/<image-set-dirs>/).
#
ifndef IMGSET
IMGSET = classic
endif

# >>> JOYSTICK <<<
#
# Enable joystick support (1) or not (0).
#
# Requires the SDL (Simple Directmedia Layer) development library!
#
ifndef JOYSTICK
JOYSTICK = 1
endif

# >>> OPENGL <<<
#
# Enable OpenGL based features (1) or use standard features only (0).
#
# Requires OpenGL if enabled (hardware acceleration recommended).
#
# Note that this is currently only used to enable drawing of images through
# OpenGL, which may actually be a bit slower than the standard 2D paint engine.
# This was just added as an experiment -- though a successful one :).
#
ifndef OPENGL
OPENGL = 0
endif

# >>> WIP <<<
#
# Enable (1) or disable (0) unfinished 'work in progress' code.
#
# Note that WIP code may be far from complete, may not work as expected, can
# crash easily or probably will not even compile correctly. We try to make WIP
# code 'as clean as possible', but there's no guarantee for nothing :).
#
ifndef WIP
WIP = 0
endif

# >>> PHONON / FORCE_PHONON <<<
#
# Enable Phonon based features (1) or leave them out of the build (0).
#
# Requires libphonon and a working Phonon backend (such as gstreamer or xine on
# Linux, DirectX 9+ on Windows and QuickTime 7+ on Mac OS X).
#
# Built-in Phonon features include the MP3 audio player and the YouTube video
# widget.
#
# For Qt 5, this will be disabled automatically (Qt 5 has no Phonon module)
# unless you set FORCE_PHONON = 1 to use an external Qt/Phonon module!
#
ifndef PHONON
PHONON = 1
endif
ifndef FORCE_PHONON
FORCE_PHONON = 0
endif

# >>> CCACHE <<<
#
# Enable (1) or disable (0) the use of the 'ccache' compiler cache utility.
#
# See also CCACHE_CC and CCACHE_CXX in arch/*.cfg!
#
ifndef CCACHE
CCACHE = 0
endif

# >>> DISTCC <<<
#
# Enable (1) or disable (0) the use of a distributed compiler.
#
# Distributed compilers we've tested successfully are 'distcc' and 'icecc'.
#
# See also DISTCC_CC and DISTCC_CXX in arch/*.cfg!
#
ifndef DISTCC
DISTCC = 0
endif

# >>> QT_TRANSLATION <<<
#
# This is used by the 'make install' target to define the Qt translations QMC2
# should use.
#
# If this is set to 'qmc2' (the default), the Qt translation files which are
# redistributed with QMC2 will be installed and used. If you would like to use
# different 'qt_<lang>.qm' files, set this to the absolute path where those
# translation files are installed (without the trailing '/') -- the install
# target will then create symbolic links instead of installing files.
#
ifndef QT_TRANSLATION
QT_TRANSLATION = qmc2
endif

# >>> BROWSER_EXTRAS <<<
#
# Enable (1) or disable (0) extra browser features such as QtWebKit's 'Web
# Inspector'.
#
ifndef BROWSER_EXTRAS
BROWSER_EXTRAS = 1
endif

# >>> BROWSER_PLUGINS <<<
#
# Enable (1) or disable (0) Netscape/Mozilla plugins in the 'MiniWebBrowser'
# and the 'HTML editor'?
#
# Caution: browser plugins may be buggy and can cause crashes! However, since
# Qt 4.7, browser plugins work quite nicely so they're enabled now as per
# default.
#
ifndef BROWSER_PLUGINS
BROWSER_PLUGINS = 1
endif

# >>> BROWSER_JAVA <<<
#
# Enable (1) or disable (0) Java in the 'MiniWebBrowser'?
#
ifndef BROWSER_JAVA
BROWSER_JAVA = 1
endif

# >>> BROWSER_JAVASCRIPT <<<
#
# Enable (1) or disable (0) JavaScript in the 'MiniWebBrowser'?
#
ifndef BROWSER_JAVASCRIPT
BROWSER_JAVASCRIPT = 1
endif

# >>> BROWSER_PREFETCH_DNS <<<
#
# Enable (1) or disable (0) prefetching of DNS lookups.
#
# This is only supported for Qt >= 4.6 and will be ignored otherwise.
#
ifndef BROWSER_PREFETCH_DNS
BROWSER_PREFETCH_DNS = 0
endif

# >>> FADER_SPEED <<<
#
# Select the audio fading speed of the MP3 player.
#
# FADER_SPEED = 0 .... Pause/resume instantly (fastest)
# FADER_SPEED > 0 .... Fading pause/resume (slower)
#
ifndef FADER_SPEED
ifeq '$(FORCE_MINGW)' '1'
FADER_SPEED = 2000
else
FADER_SPEED = 500
endif
endif

# >>> CC_FLAGS <<<
#
# Specify additional flags passed to the C compiler.
#
ifndef CC_FLAGS
CC_FLAGS =
endif

# >>> CXX_FLAGS <<<
#
# Specify additional flags passed to the C++ compiler.
#
ifndef CXX_FLAGS
CXX_FLAGS =
endif

# >>> L_FLAGS <<<
#
# Specify additional flags passed to the linker.
#
ifndef L_FLAGS
L_FLAGS =
endif

# >>> L_LIBS <<<
#
# Specify additional libraries passed to the linker.
#
ifndef L_LIBS
L_LIBS =
endif

# >>> L_LIBDIRS <<<
#
# Specify additional library directories passed to the linker.
#
ifndef L_LIBDIRS
L_LIBDIRS =
endif

# >>> L_LIBDIRFLAGS <<<
#
# Specify an optional value for QMAKE_LIBDIR_FLAGS (usually not required).
#
# This option can be useful to solve Qt library version conflicts when you have
# different versions of Qt installed and the linker chooses the wrong one, then
# for example using "L_LIBDIRFLAGS=-L<path-to-qt>/lib" will ensure that the
# given library path will be used before other (system library) paths.
#
ifndef L_LIBDIRFLAGS
L_LIBDIRFLAGS =
endif

# >>> LINKER <<<
#
# Specify (overwrite) the linker to be used (may be useful for cross-compile).
#
ifndef LINKER
LINKER =
endif

# >>> MKSPEC <<<
#
# Specify (overwrite) the Qt mkspec (qmake spec) to be used.
#
ifndef MKSPEC
ifeq '$(FORCE_MINGW)' '1'
MKSPEC = win32-g++-4.6
else
ifeq '$(ARCH)' 'Darwin'
MKSPEC = macx-g++
else
MKSPEC =
endif
endif
endif

# >>> MAC_UNIVERSAL <<<
#
# Enable (1) or disable (0) the creation of a Mac OS X universal binary
#
# This is only used on Mac OS X and disabled by default.
#
ifndef MAC_UNIVERSAL
MAC_UNIVERSAL = 0
endif

# >>> YOUTUBE <<<
#
# Enable (1) or disable (0) support for game/machine 'attached' YouTube videos.
#
# With Qt 4 this feature requires Phonon and will thus be disabled automatically
# when Phonon has been disabled globally (PHONON=0)! In case of Qt 5 we use the
# QtMultimedia module where Phonon isn't required.
#
# You'll also need a decent back-end that supports FLV and MP4 video formats
# (codecs). Take a look at the README (section 2, software requirements) for
# recommended codec-packages on certain platforms.
#
ifndef YOUTUBE
YOUTUBE = 1
endif

# >>> LOCAL_QML_IMPORT_PATH <<<
#
# Specifies a local path that's used as an additional path for QML imports. The
# default path is 'imports'.
#
ifndef LOCAL_QML_IMPORT_PATH
LOCAL_QML_IMPORT_PATH = imports
endif

# >>> MAN_DIR <<<
#
# The base-directory used by the 'make doc-install' target to install man-pages.
#
ifneq '$(FORCE_MINGW)' '1'
ifndef MAN_DIR
ifeq '$(ARCH)' 'Darwin'
MAN_DIR = /usr/share/man
else
MAN_DIR = $(PREFIX)/man
endif
endif
endif

# >>> SDL <<<
#
# SDL version to use (1 or 2). Auto-detected if unspecified -- SDL=1 has
# precendence.
#
ifneq '$(FORCE_MINGW)' '1'
ifndef SDL
SDL = $(shell scripts/sdl-version.sh)
else
ifneq '$(SDL)' '1'
ifneq '$(SDL)' '2'
SDL = $(shell scripts/sdl-version.sh)
endif
endif
endif
else
SDL = 2
endif

# >>> END OF MAKE OPTIONS -- PLEASE DO NOT CHANGE ANYTHING AFTER THIS LINE <<<

# project name
PROJECT = qmc2

# version
VERSION_MAJOR = 0
VERSION_MINOR = 58

# commands are platform/distribution-specific
ifneq '$(ARCH)' 'Windows'
include arch/default.cfg
ifeq '$(OSCFG)' '1'
OSCFGFILE = $(shell scripts/os-detect.sh | $(GREP) "System cfg-file" | $(COLRM) 1 30)
ifeq ($(wildcard $(OSCFGFILE)),)
OS = $(shell scripts/os-detect.sh | $(GREP) "Operating System" | $(COLRM) 1 30)
$(info No operating system specific configuration found for '$(OS)')
else
include $(OSCFGFILE)
endif
endif
ifeq '$(DISTCFG)' '1'
DISTCFGFILE = $(shell scripts/os-detect.sh | $(GREP) "Distribution cfg-file" | $(COLRM) 1 30)
ifeq ($(wildcard $(DISTCFGFILE)),)
DIST = $(shell scripts/os-detect.sh | $(GREP) "Distribution / OS version" | $(COLRM) 1 30)
$(info No distribution specific configuration found for '$(DIST)')
else
include $(DISTCFGFILE)
endif
endif
else
include arch/Windows.cfg
endif

ifneq '$(ARCH)' 'Windows'
QMC2_EMULATOR = SDLMAME
else
QMC2_EMULATOR = MAME
endif

# associate icon files
ifeq '$(ARCH)' 'Darwin'
MYAPPICON = mame.icns
endif

ifndef SVN_REV
# determine the SVN revision (if any)
ifneq '$(ARCH)' 'Windows'
SVN_REV=$(shell $(SVNVERSION) 2>&1 | $(SED) -e "s/[MS]//g" -e "s/^[[:digit:]]*://" | $(GREP) "^[0-9]*$$")
else
# this assumes Tortoise SVN!
SVN_SUBWCREV_CMD=$(shell arch\Windows\which.bat subwcrev)
ifneq '$(SVN_SUBWCREV_CMD)' ''
SVN_REV=$(shell arch\Windows\svnversion.bat)
else
SVN_REV=
endif
endif
endif

ifeq '$(SVN_REV)' ''
SVN_REV=0
endif

ifneq '$(ARCH)' 'Windows' 

# global QMC2 configuration file
GLOBAL_QMC2_INI=$(shell $(ECHO) $(DESTDIR)/$(SYSCONFDIR)/$(PROJECT)/$(PROJECT).ini | $(SED) -e "s*//*/*g")

# global data directory
GLOBAL_DATADIR=$(shell $(ECHO) $(DESTDIR)/$(DATADIR) | $(SED) -e "s*//*/*g")

endif

# qmake version check (major release)
ifndef QMAKEV
ifeq '$(ARCH)' 'Windows'
QMAKEV = 2
else
QMAKEV = $(shell $(ECHO) `$(QMAKE) -v` | $(AWK) '{print $$3}' | $(COLRM) 2)
endif
endif

ifeq '$(QMAKEV)' '3'
ifeq '$(FORCE_PHONON)' '1'
PHONON = 1
else
PHONON = 0
endif
endif

ifneq '$(QMAKEV)' '1'

ifneq '$(ARCH)' 'Windows'
QT_LIBVERSION = $(shell $(QMAKE) -v | $(GREP) "Qt version" | $(AWK) '{print $$4}')
ifeq '$(ARCH)' 'Darwin'
QMAKEFILE = Makefile.qmake
QT_LIBTMP = $(shell $(ECHO) $(QT_LIBVERSION) | tr "." " " )
QT_LIBMAJ = $(shell $(ECHO) $(QT_LIBTMP) | $(AWK) '{ print $$1 }')
QT_LIBMIN = $(shell $(ECHO) $(QT_LIBTMP) | $(AWK) '{ print $$2 }')
QT_LIB48PLUS = $(shell (([ $(QT_LIBMAJ) -ge 4 ] && [ $(QT_LIBMIN) -ge 8 ]) || [ $(QT_LIBMAJ) -ge 5 ]) && $(ECHO) true)
ifneq '$(QT_LIB48PLUS)' 'true'
$(error Sorry, Qt 4.8+ not found!)
endif
endif
endif

# complete version string
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR)

# work around for qmake's issues with spaces in values
blank =
space = $(blank) $(blank)

# pre-compiler definitions (passed to qmake)
DEFINES = DEFINES+=QMC2_$(QMC2_EMULATOR) QMC2_VERSION=$(VERSION) QMC2_SVN_REV=$(SVN_REV) BUILD_OS_NAME=$(OSNAME) BUILD_OS_RELEASE=$(OSREL) BUILD_MACHINE=$(MACHINE) PREFIX=$(PREFIX) DATADIR="$(subst $(space),:,$(DATADIR))" SYSCONFDIR="$(subst $(space),:,$(SYSCONFDIR))" QMC2_JOYSTICK=$(JOYSTICK) QMC2_OPENGL=$(OPENGL) QMC2_PHONON=$(PHONON) QMC2_FADER_SPEED=$(FADER_SPEED)

# available translations
QMC2_TRANSLATIONS = de es el fr it pl pt ro sv us
QT_TRANSLATIONS = de es fr pl pt sv

# process make options
ifeq '$(DEBUG)' '2'
DEFINES += QMC2_DEBUG
endif


ifeq '$(ARCH)' 'Darwin'
DEFINES += QMC2_MAC_UNIVERSAL=$(MAC_UNIVERSAL)
endif

ifeq '$(BROWSER_EXTRAS)' '1'
DEFINES += QMC2_BROWSER_EXTRAS_ENABLED
endif

ifeq '$(BROWSER_PLUGINS)' '1'
DEFINES += QMC2_BROWSER_PLUGINS_ENABLED
endif

ifeq '$(BROWSER_JAVA)' '1'
DEFINES += QMC2_BROWSER_JAVA_ENABLED
endif

ifeq '$(BROWSER_JAVASCRIPT)' '1'
DEFINES += QMC2_BROWSER_JAVASCRIPT_ENABLED
endif

ifeq '$(BROWSER_PREFETCH_DNS)' '1'
DEFINES += QMC2_BROWSER_PREFETCH_DNS_ENABLED
endif

ifneq '$(QMAKEV)' '3'
ifeq '$(PHONON)' '0'
YOUTUBE = 0
endif
endif

ifeq '$(YOUTUBE)' '1'
DEFINES += QMC2_YOUTUBE_ENABLED
endif

ifeq '$(AUDIOEFFECTDIALOGS)' '0'
DEFINES += QMC2_NOEFFECTDIALOGS
endif

ifeq '$(WIP)' '1'
DEFINES += QMC2_WIP_ENABLED
endif

# setup SDL library and include paths
ifdef SDL_LIBS
undef SDL_LIBS
endif
ifdef SDL_INCLUDEPATH
undef SDL_INCLUDEPATH
endif
ifneq '$(ARCH)' 'Windows'
ifneq '$(ARCH)' 'Darwin'
ifeq '$(JOYSTICK)' '1'
SDL_LIBS = LIBS+='$(shell scripts/sdl-libs.sh $(SDL))'
SDL_INCLUDEPATH = INCLUDEPATH+='$(shell scripts/sdl-includepath.sh $(SDL))'
DEFINES += $(shell scripts/sdl-defines.sh $(SDL))
endif
endif
endif

# setup additional qmake features for release or debug builds
ifdef QMAKE_CONF
undef QMAKE_CONF
endif
ifdef ARCADE_QMAKE_CONF
undef ARCADE_QMAKE_CONF
endif

# setup TARGET's application icon and generic name
EMUICO = mame.png
GENERICNAME = M.A.M.E. Catalog / Launcher II

# target name (variant)
ifneq '$(ARCH)' 'Windows'
TARGET_NAME = $(PROJECT)-sdlmame
else
TARGET_NAME = $(PROJECT)-mame
endif

QMAKE_CONF = TARGET=$(TARGET_NAME)

ifeq '$(DEBUG)' '0'
QMAKE_CONF += CONFIG+=warn_off CONFIG+=release
else
QMAKE_CONF += CONFIG+=warn_on CONFIG+=debug
endif

# setup library and include paths for MinGW environment
ifeq '$(ARCH)' 'Windows'
TEST_FILE=$(shell gcc -print-file-name=libSDL2.a)
MINGW_LIBDIR=$(shell arch\Windows\dirname.bat $(TEST_FILE))
ifeq '$(SDL)' '2'
QMAKE_CONF += QMC2_LIBS+=-L$(MINGW_LIBDIR) QMC2_INCLUDEPATH+=$(MINGW_LIBDIR)../include QMC2_INCLUDEPATH+=$(MINGW_LIBDIR)../include/SDL2
ARCADE_QMAKE_CONF += QMC2_ARCADE_INCLUDEPATH+=$(MINGW_LIBDIR)../include QMC2_ARCADE_INCLUDEPATH+=$(MINGW_LIBDIR)../include/SDL2
else
QMAKE_CONF += QMC2_LIBS+=-L$(MINGW_LIBDIR) QMC2_INCLUDEPATH+=$(MINGW_LIBDIR)../include QMC2_INCLUDEPATH+=$(MINGW_LIBDIR)../include/SDL
ARCADE_QMAKE_CONF += QMC2_ARCADE_INCLUDEPATH+=$(MINGW_LIBDIR)../include QMC2_ARCADE_INCLUDEPATH+=$(MINGW_LIBDIR)../include/SDL
endif
endif

# optionally setup the qmake spec
ifdef QT_MAKE_SPEC
undef QT_MAKE_SPEC
endif

ifneq '$(MKSPEC)' ''
QT_MAKE_SPEC = -spec $(MKSPEC)
endif

# setup additional Qt configuration options
ifdef QT_CONF
undef QT_CONF
endif

ifeq '$(OPENGL)' '1'
QT_CONF += QT+=opengl
endif

ifeq '$(PHONON)' '1'
QT_CONF += QT+=phonon
endif

# setup use of CCACHE or DISTCC (if applicable)
ifdef QMAKE_CXX_COMPILER
undef QMAKE_CXX_COMPILER
endif

ifneq '$(CCACHE)$(DISTCC)' '11'
ifeq '$(CCACHE)' '1'
QMAKE_CXX_COMPILER += QMAKE_CXX=$(CCACHE_CXX) QMAKE_CC=$(CCACHE_CC)
endif
ifeq '$(DISTCC)' '1'
QMAKE_CXX_COMPILER += QMAKE_CXX=$(DISTCC_CXX) QMAKE_CC=$(DISTCC_CC)
endif
else
$(warning ### WARNING: you cannot mix DISTCC and CCACHE -- disabling both ###)
CCACHE = 0
DISTCC = 0
endif

# optional C++ compiler flags
ifdef QMAKE_CXX_FLAGS
undef QMAKE_CXX_FLAGS
endif
ifneq '$(CXX_FLAGS)' ''
QMAKE_CXX_FLAGS += QMAKE_CXXFLAGS='$(CXX_FLAGS)' QMAKE_CXXFLAGS_RELEASE='$(CXX_FLAGS)' QMAKE_CXXFLAGS_DEBUG='$(CXX_FLAGS)'
endif

# optional C compiler flags
ifdef QMAKE_CC_FLAGS
undef QMAKE_CC_FLAGS
endif
ifneq '$(CC_FLAGS)' ''
QMAKE_CC_FLAGS += QMAKE_CFLAGS='$(CC_FLAGS)' QMAKE_CFLAGS_RELEASE='$(CC_FLAGS)' QMAKE_CFLAGS_DEBUG='$(CC_FLAGS)'
endif

# optional linker flags
ifdef QMAKE_L_FLAGS
undef QMAKE_L_FLAGS
endif
ifneq '$(L_FLAGS)' ''
QMAKE_L_FLAGS += QMAKE_LFLAGS='$(L_FLAGS)' QMAKE_LFLAGS_RELEASE='$(L_FLAGS)' QMAKE_LFLAGS_DEBUG='$(L_FLAGS)'
endif

# optional libraries
ifdef QMAKE_L_LIBS
undef QMAKE_L_LIBS
endif
ifneq '$(L_LIBS)' ''
QMAKE_L_LIBS += QMAKE_LIBS='$(L_LIBS)' QMAKE_LIBS_RELEASE='$(L_LIBS)' QMAKE_LIBS_DEBUG='$(L_LIBS)'
endif

# optional library paths
ifdef QMAKE_L_LIBDIRS
undef QMAKE_L_LIBDIRS
endif
ifneq '$(L_LIBDIRS)' ''
QMAKE_L_LIBDIRS += QMAKE_LIBDIR='$(L_LIBDIRS)'
endif

# optional QMAKE_LIBDIR_FLAGS
ifdef QMAKE_L_LIBDIRFLAGS
undef QMAKE_L_LIBDIRFLAGS
endif
ifneq '$(L_LIBDIRFLAGS)' ''
QMAKE_L_LIBDIRFLAGS += QMAKE_LIBDIR_FLAGS='$(L_LIBDIRFLAGS)'
endif

# optional linker binary overwrite
ifdef QMAKE_LINKER
undef QMAKE_LINKER
endif
ifneq '$(LINKER)' ''
QMAKE_LINKER += QMAKE_LINK=$(LINKER)
endif

# targets/rules
all: $(PROJECT)-bin 

bin: $(PROJECT)-bin

$(PROJECT): $(PROJECT)-bin

ifeq '$(ARCH)' 'Darwin'
# put the version, SCM revision and icon resource in Info.plist on Mac OS X
%.plist: %.plist.in
	@$(SED) -e 's/@SHORT_VERSION@/$(subst /,\/,$(VERSION))/g' -e 's/@SCM_REVISION@/$(subst /,\/,$(SVN_REV))/g' -e 's/@ICON@/$(MYAPPICON)/g' < $< > $@
endif

# tools
ifeq '$(DEBUG)' '0'
QCHDMAN_CONF += CONFIG+=warn_off CONFIG+=release
QCHDMAN_DEFINES = DEFINES+=QCHDMAN_RELEASE QCHDMAN_SVN_REV=$(SVN_REV)
else
QCHDMAN_CONF += CONFIG+=warn_on CONFIG+=debug
QCHDMAN_DEFINES = DEFINES+=QCHDMAN_DEBUG QCHDMAN_SVN_REV=$(SVN_REV)
endif

ifeq '$(ARCH)' 'Darwin'
ifeq '$(MAC_UNIVERSAL)' '1'
QCHDMAN_DEFINES += QCHDMAN_MAC_UNIVERSAL
endif
endif

ifeq '$(WIP)' '1'
QCHDMAN_DEFINES += QCHDMAN_WIP_ENABLED
endif

ifeq '$(ARCH)' 'Windows'
qchdman:
	@$(CD) tools\qchdman && $(QMAKE) -makefile $(QT_MAKE_SPEC) $(QCHDMAN_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) "$(QCHDMAN_DEFINES)" && $(MAKE)

qchdman-clean:
	@$(CD) tools\qchdman && $(QMAKE) -makefile $(QT_MAKE_SPEC) $(QCHDMAN_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) "$(QCHDMAN_DEFINES)" && $(MAKE) distclean
	@$(RMDIR) /s /q tools\qchdman\release
	@$(RMDIR) /s /q tools\qchdman\debug
	@$(RM) tools\qchdman\object_script.qchdman.Release tools\qchdman\object_script.qchdman.Debug

tools: qchdman
tools-clean: qchdman-clean
else
qchdman:
	@$(CD) tools/qchdman && $(QMAKE) -makefile $(QT_MAKE_SPEC) $(QCHDMAN_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) '$(QCHDMAN_DEFINES)' && $(MAKE)

qchdman-clean:
	@$(CD) tools/qchdman && $(QMAKE) -makefile $(QT_MAKE_SPEC) $(QCHDMAN_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) '$(QCHDMAN_DEFINES)' && $(MAKE) distclean
ifeq '$(ARCH)' 'Darwin'
	@$(RM) tools/qchdman/Info.plist

QCHDMAN_VERSION=$(shell $(GREP) "VERSION =" tools/qchdman/qchdman.pro | $(AWK) '{ print $$3 }')
tools/qchdman/Info.plist: tools/qchdman/Info.plist.in
	@$(SED) -e 's/@SHORT_VERSION@/$(subst /,\/,$(QCHDMAN_VERSION))/g' -e 's/@SCM_REVISION@/$(subst /,\/,$(SVN_REV))/g' -e 's/@ICON@/qchdman.icns/g' < $< > $@
tools/qchdman/qchdman.app/Contents/Resources/qt.conf: tools/qchdman/Info.plist
	@$(MACDEPLOYQT) tools/qchdman/qchdman.app
qchdman-macdeployqt: tools/qchdman/qchdman.app/Contents/Resources/qt.conf
qchdman-install: qchdman qchdman-macdeployqt
	@$(RSYNC) --exclude '*svn*' "tools/qchdman/qchdman.app" "/Applications"
	@$(CHMOD) a+rx "/Applications/qchdman.app"
	@$(RSYNC) tools/qchdman/images/qchdman.icns /Applications/qchdman.app/Contents/Resources/
	@$(RSYNC) tools/qchdman/Info.plist /Applications/qchdman.app/Contents/
else
qchdman-install: qchdman
	@$(MKDIR) "$(DESTDIR)/$(BINDIR)" "$(DESTDIR)/$(DATADIR)/$(PROJECT)"
	@$(RSYNC) --exclude '*svn*' "tools/qchdman/qchdman" "$(DESTDIR)/$(BINDIR)"
	@$(RSYNC) --exclude '*svn*' ./data/img "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(ECHO) "Installing qchdman.desktop to $(GLOBAL_DATADIR)/applications"
	@$(MKDIR) $(GLOBAL_DATADIR)/applications
	@$(CHMOD) a+rx $(GLOBAL_DATADIR)/applications
	@$(SED) -e "s*DATADIR*$(DATADIR)*g" < ./inst/qchdman.desktop.template > $(GLOBAL_DATADIR)/applications/qchdman.desktop
endif

tools: qchdman
tools-clean: qchdman-clean
tools-install: qchdman-install
endif

ifeq '$(DEBUG)' '0'
ARCADE_CONF = CONFIG+=warn_off CONFIG+=release
ARCADE_DEFINES = DEFINES+=QMC2_ARCADE_RELEASE QMC2_ARCADE_SVN_REV=$(SVN_REV)
else
ARCADE_CONF = CONFIG+=warn_on CONFIG+=debug
ARCADE_DEFINES = DEFINES+=QMC2_ARCADE_DEBUG QMC2_ARCADE_SVN_REV=$(SVN_REV)
endif
ifeq '$(FORCE_MINGW)' '1'
ARCADE_DEFINES += QMC2_ARCADE_MINGW
endif
ifeq '$(ARCH)' 'Darwin'
ifeq '$(MAC_UNIVERSAL)' '1'
ARCADE_DEFINES += QMC2_ARCADE_MAC_UNIVERSAL
endif
endif
ARCADE_VERSION=$(shell $(GREP) "VERSION =" arcade/qmc2-arcade.pro | $(AWK) '{ print $$3 }')
ARCADE_QMAKE_DEFS = QMC2_ARCADE_QML_IMPORT_PATH=$(LOCAL_QML_IMPORT_PATH) QMC2_ARCADE_JOYSTICK=$(JOYSTICK) SDL=$(SDL) $(ARCADE_QMAKE_CONF)

arcade: arcade-bin
arcade-bin:
	@$(CD) arcade && $(QMAKE) -makefile $(QT_MAKE_SPEC) $(ARCADE_CONF) $(ARCADE_QMAKE_DEFS) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) "$(ARCADE_DEFINES)" && $(MAKE)

arcade-clean:
	@$(CD) arcade && $(QMAKE) -makefile $(QT_MAKE_SPEC) $(ARCADE_CONF) $(ARCADE_QMAKE_DEFS) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) "$(ARCADE_DEFINES)" && $(MAKE) distclean
ifeq '$(ARCH)' 'Windows'
	@$(RMDIR) /s /q arcade\release
	@$(RMDIR) /s /q arcade\debug
	@$(RM) arcade\object_script.qmc2-arcade.Release arcade\object_script.qmc2-arcade.Debug
endif
ifeq '$(ARCH)' 'Darwin'
	@$(RM) arcade/Info.plist
endif

ifeq '$(ARCH)' 'Darwin'
ARCADE_VERSION=$(shell $(GREP) "VERSION =" arcade/qmc2-arcade.pro | $(AWK) '{ print $$3 }')
arcade/Info.plist: arcade/Info.plist.in
	@$(SED) -e 's/@SHORT_VERSION@/$(subst /,\/,$(ARCADE_VERSION))/g' -e 's/@SCM_REVISION@/$(subst /,\/,$(SVN_REV))/g' -e 's/@ICON@/qmc2-arcade.icns/g' < $< > $@
arcade/qmc2-arcade.app/Contents/Resources/qt.conf: arcade/Info.plist
	@$(MACDEPLOYQT) arcade/qmc2-arcade.app
	@arch/Darwin/arcade_macdeployimports.sh $(QT_LIBMAJ)
arcade-macdeployqt: arcade/qmc2-arcade.app/Contents/Resources/qt.conf
endif

ifeq '$(ARCH)' 'Windows'
else
ifeq '$(ARCH)' 'Darwin'
arcade-install: arcade arcade-macdeployqt
	@$(RSYNC) --exclude '*svn*' "arcade/qmc2-arcade.app" "/Applications/qmc2"
	@$(CHMOD) a+rx "/Applications/qmc2/qmc2-arcade.app"
	@$(RSYNC) arcade/images/qmc2-arcade.icns /Applications/qmc2/qmc2-arcade.app/Contents/Resources/
	@$(RSYNC) arcade/Info.plist /Applications/qmc2/qmc2-arcade.app/Contents/
else
arcade-install: arcade
	@$(MKDIR) "$(DESTDIR)/$(BINDIR)" "$(DESTDIR)/$(DATADIR)/$(PROJECT)"
	@$(RSYNC) --exclude '*svn*' "arcade/qmc2-arcade" "$(DESTDIR)/$(BINDIR)"
	@$(RSYNC) --exclude '*svn*' ./data/img "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(ECHO) "Installing qmc2-arcade.desktop to $(GLOBAL_DATADIR)/applications"
	@$(MKDIR) $(GLOBAL_DATADIR)/applications
	@$(CHMOD) a+rx $(GLOBAL_DATADIR)/applications
	@$(SED) -e "s*DATADIR*$(DATADIR)*g" < ./inst/qmc2-arcade.desktop.template > $(GLOBAL_DATADIR)/applications/qmc2-arcade.desktop
endif
endif

ifeq '$(QUIET)' '1'
ifeq '$(ARCH)' 'Windows'
rcgen: qmc2-mame.rc qmc2-mess.rc qmc2-ume.rc

qmc2-mame.rc:
	@arch\Windows\rcgen.bat

qmc2-mess.rc:
	@arch\Windows\rcgen.bat

qmc2-ume.rc:
	@arch\Windows\rcgen.bat

$(PROJECT)-bin: lang $(QMAKEFILE) rcgen
else
$(PROJECT)-bin: lang $(QMAKEFILE) 
endif
	@$(ECHO) "Updating build of QMC2 v$(VERSION)"
ifeq '$(ARCH)' 'Darwin'
ifeq '$(QT_LIB48PLUS)' 'true'
	+@$(MAKESILENT) -f $(QMAKEFILE) > /dev/null
else
ifeq '$(CTIME)' '0'
	+@xcodebuild -project Makefile.qmake.xcodeproj -configuration Release > /dev/null
else
	+@$(TIME) (xcodebuild -project Makefile.qmake.xcodeproj -configuration Release > /dev/null)
endif
endif
else
ifeq '$(ARCH)' 'Windows'
	+@$(MAKESILENT) -f $(QMAKEFILE) > NUL
else
ifeq '$(CTIME)' '0'
	+@$(MAKESILENT) -f $(QMAKEFILE) > /dev/null
else
	+@$(TIME) ($(MAKESILENT) -f $(QMAKEFILE) > /dev/null)
endif
endif
endif
	@$(ECHO) "Build of QMC2 v$(VERSION) complete"

ifeq '$(ARCH)' 'Darwin'
$(QMAKEFILE): $(PROJECT).pro arch/Darwin/Info.plist
else
$(QMAKEFILE): $(PROJECT).pro
endif
	@$(ECHO) "Configuring build of QMC2 v$(VERSION)"
ifneq '$(ARCH)' 'Windows'
	@$(shell scripts/setup_imgset.sh "$(IMGSET)" "$(RM)" "$(LN)" "$(BASENAME)" > /dev/null) 
	@$(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) VERSION=$(VERSION) QMC2_MINGW=$(FORCE_MINGW) SDL=$(SDL) $(QMAKE_CONF) $(SDL_LIBS) $(SDL_INCLUDEPATH) $(QT_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) '$(DEFINES)' $< > /dev/null
else
	@$(shell scripts\\setup_imgset.bat $(IMGSET)) 
	@$(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) VERSION=$(VERSION) QMC2_MINGW=$(FORCE_MINGW) SDL=$(SDL) $(QMAKE_CONF) $(SDL_LIBS) $(SDL_INCLUDEPATH) $(QT_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) '$(DEFINES)' $<
endif
ifeq '$(ARCH)' 'Darwin'
ifneq '$(QT_LIB48PLUS)' 'true'
	@$(SED) -e "s/-c /cc -c /" < ./Makefile.qmake.xcodeproj/qt_preprocess.mak > "./Makefile.qmake.xcodeproj/qt_preprocess.mak.new"
	@$(RM) ./Makefile.qmake.xcodeproj/qt_preprocess.mak
	@$(MV) ./Makefile.qmake.xcodeproj/qt_preprocess.mak.new ./Makefile.qmake.xcodeproj/qt_preprocess.mak
endif
endif
else
ifeq '$(ARCH)' 'Windows'
rcgen: qmc2-mame.rc qmc2-mess.rc qmc2-ume.rc

qmc2-mame.rc:
	@arch\Windows\rcgen.bat

qmc2-mess.rc:
	@arch\Windows\rcgen.bat

qmc2-ume.rc:
	@arch\Windows\rcgen.bat

$(PROJECT)-bin: lang $(QMAKEFILE) rcgen
else
$(PROJECT)-bin: lang $(QMAKEFILE)
endif
	@$(ECHO) "Updating build of QMC2 v$(VERSION)"
ifeq '$(ARCH)' 'Darwin'
ifeq '$(QT_LIB48PLUS)' 'true'
	+@$(MAKE) -f $(QMAKEFILE)
else
ifeq '$(CTIME)' '0'
	+@xcodebuild -project Makefile.qmake.xcodeproj -configuration Release
else
	+@$(TIME) (xcodebuild -project Makefile.qmake.xcodeproj -configuration Release)
endif
endif
else
ifeq '$(ARCH)' 'Windows'
	+@$(MAKE) -f $(QMAKEFILE)
else
ifeq '$(CTIME)' '0'
	+@$(MAKE) -f $(QMAKEFILE)
else
	+@$(TIME) ($(MAKE) -f $(QMAKEFILE))
endif
endif
endif
	@$(ECHO) "Build of QMC2 v$(VERSION) complete"

ifeq '$(ARCH)' 'Darwin'
$(QMAKEFILE): $(PROJECT).pro arch/Darwin/Info.plist
else
$(QMAKEFILE): $(PROJECT).pro
endif
	@$(ECHO) "Configuring build of QMC2 v$(VERSION)"
ifneq '$(ARCH)' 'Windows'
	@$(shell scripts/setup_imgset.sh "$(IMGSET)" "$(RM)" "$(LN)" "$(BASENAME)") 
else
	@$(shell scripts\\setup_imgset.bat $(IMGSET)) 
endif
ifeq '$(ARCH)' 'Darwin'
ifneq '$(QT_LIB48PLUS)' 'true'
	$(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) VERSION=$(VERSION) QMC2_MINGW=$(FORCE_MINGW) SDL=$(SDL) $(QMAKE_CONF) $(SDL_LIBS) $(SDL_INCLUDEPATH) $(QT_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) '$(DEFINES)' $<
	@$(SED) -e "s/-c /cc -c /" < ./Makefile.qmake.xcodeproj/qt_preprocess.mak > "./Makefile.qmake.xcodeproj/qt_preprocess.mak.new"
	@$(RM) ./Makefile.qmake.xcodeproj/qt_preprocess.mak
	@$(MV) ./Makefile.qmake.xcodeproj/qt_preprocess.mak.new ./Makefile.qmake.xcodeproj/qt_preprocess.mak
else
	$(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) VERSION=$(VERSION) QMC2_MINGW=$(FORCE_MINGW) SDL=$(SDL) $(QMAKE_CONF) $(SDL_LIBS) $(SDL_INCLUDEPATH) $(QT_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) '$(DEFINES)' $<
endif
else
	$(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) VERSION=$(VERSION) QMC2_MINGW=$(FORCE_MINGW) SDL=$(SDL) $(QMAKE_CONF) $(SDL_LIBS) $(SDL_INCLUDEPATH) $(QT_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_L_LIBDIRFLAGS) $(QMAKE_LINKER) '$(DEFINES)' $<
endif
endif

ifneq '$(ARCH)' 'Windows'

ifeq '$(ARCH)' 'Darwin'
$(TARGET_NAME).app/Contents/Resources/qt.conf:
	@$(MACDEPLOYQT) $(TARGET_NAME).app
macdeployqt: $(TARGET_NAME).app/Contents/Resources/qt.conf

install: bin macdeployqt
else
install: bin
endif
	@$(ECHO) "Installing QMC2 v$(VERSION)"
	@$(MKDIR) "$(DESTDIR)/$(BINDIR)" "$(DESTDIR)/$(DATADIR)/$(PROJECT)" "$(DESTDIR)/$(SYSCONFDIR)/$(PROJECT)"
ifeq '$(ARCH)' 'Darwin'
	@$(MKDIR) "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
	@$(CHMOD) a+rx "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
	@$(RSYNC) --exclude '*svn*' "./$(TARGET_NAME).app" "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
else
	@$(RM) -f "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
	@$(RSYNC) --exclude '*svn*' "./$(TARGET_NAME)" "$(DESTDIR)/$(BINDIR)"
	@(cd "$(DESTDIR)/$(BINDIR)" && $(LN) -s "$(TARGET_NAME)" "$(PROJECT)")
endif
	@$(RSYNC) --exclude '*svn*' ./data/lng/qmc2_*.qm "$(GLOBAL_DATADIR)/$(PROJECT)/lng/"
	@if [ "$(QT_TRANSLATION)" = "qmc2" ] ; then \
	  $(RSYNC) --exclude '*svn*' ./data/lng/qt_*.qm "$(GLOBAL_DATADIR)/$(PROJECT)/lng/" ; \
	else \
	  $(ECHO) "Using Qt translation files from $(QT_TRANSLATION)" ; \
	  $(RM) -f "$(GLOBAL_DATADIR)/$(PROJECT)/lng/"qt_*.qm ; \
	  for i in $(QT_TRANSLATIONS) ; do \
	    $(LN) -s "$(QT_TRANSLATION)/qt_$$i.qm" "$(GLOBAL_DATADIR)/$(PROJECT)/lng/qt_$$i.qm" ; \
	  done \
	fi
	@$(RSYNC) --exclude '*svn*' ./data/opt "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/img "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/doc "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/prv "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/fly "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/ico "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/cat "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/cab "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/ctl "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/mrq "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/ttl "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/sws "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/swn "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/gmn "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@$(RSYNC) --exclude '*svn*' ./data/js "$(GLOBAL_DATADIR)/$(PROJECT)/"
	@if [ -f "$(GLOBAL_QMC2_INI)" ] ; then \
	  $(ECHO) "Preserving system-wide configuration in $(GLOBAL_QMC2_INI)" ; \
	  $(ECHO) "Installing new system-wide configuration as $(GLOBAL_QMC2_INI).new" ; \
	  $(SED) -e "s*DATADIR*$(DATADIR)*g" < ./inst/$(PROJECT).ini.template > "$(GLOBAL_QMC2_INI).new" ; \
	else \
	  $(ECHO) "Installing system-wide configuration as $(GLOBAL_QMC2_INI)" ; \
	  $(SED) -e "s*DATADIR*$(DATADIR)*g" < ./inst/$(PROJECT).ini.template > "$(GLOBAL_QMC2_INI)" ; \
	fi
ifneq '$(ARCH)' 'Darwin'
	@$(ECHO) "Installing $(TARGET_NAME).desktop to $(GLOBAL_DATADIR)/applications"
	@$(MKDIR) $(GLOBAL_DATADIR)/applications
	@$(CHMOD) a+rx $(GLOBAL_DATADIR)/applications
	@$(SED) -e "s*DATADIR*$(DATADIR)*g; s*EMULATOR*$(QMC2_EMULATOR)*g; s*TARGET*$(TARGET_NAME)*g; s*EMUICO*$(EMUICO)*g; s*GENERICNAME*$(GENERICNAME)*g" < ./inst/$(PROJECT).desktop.template > $(GLOBAL_DATADIR)/applications/$(TARGET_NAME).desktop
endif
	@$(ECHO) "Installation complete"

endif

ifneq '$(ARCH)' 'Windows'
distclean: clean tools-clean arcade-clean doc-clean
else
distclean: clean tools-clean arcade-clean
endif

clean: $(QMAKEFILE)
	@$(ECHO) "Cleaning up build of QMC2 v$(VERSION)"
ifeq '$(QUIET)' '0'
ifneq '$(ARCH)' 'Windows'
	@$(RM) error.log exclude.list
	@$(RM) -Rf tools/qmc2_options_editor_java/bin
	@$(RM) -f tools/qmc2_options_editor_java/*.jar
endif
ifeq '$(ARCH)' 'Darwin'
ifneq '$(QT_LIB48PLUS)' 'true'
	@xcodebuild -project Makefile.qmake.xcodeproj -alltargets -configuration Release clean
	@xcodebuild -project Makefile.qmake.xcodeproj -alltargets -configuration Debug clean
	@$(RM) -r build $(TARGET_NAME).app
	@$(RM) $(wildcard $(dir $(QMAKEFILE))*.mode* $(dir $(QMAKEFILE))*.pbxuser)
	@# this shouldn't be necessary, but qmake doesn't add a proper clean target to the project
	@$(RM) $(patsubst %.ui,ui_%.h,$(wildcard *.ui) $(notdir $(wildcard */*.ui)))
	@$(RM) $(wildcard moc_*.cpp) qrc_qmc2.cpp arch/Darwin/Info.plist Info.plist Makefile.qmake.xcodeproj/*
	@$(RMDIR) Makefile.qmake.xcodeproj
else
	@$(MAKE) -f $(QMAKEFILE) distclean
	@$(RM) arch/Darwin/Info.plist Info.plist
endif
else
ifneq '$(ARCH)' 'Windows'
	@$(MAKE) -f $(QMAKEFILE) distclean
else
	@$(MAKE) -f $(QMAKEFILE) distclean
	@$(RM) object_script.$(TARGET_NAME).Release object_script.$(TARGET_NAME).Debug $(TARGET_NAME).exe_resource.rc scripts\subwcrev.out qmc2-mame.rc qmc2-mess.rc qmc2-ume.rc
	@$(RMDIR) /s /q release > NUL
	@$(RMDIR) /s /q debug > NUL
endif
endif
ifneq '$(ARCH)' 'Windows'
	@$(shell scripts/setup_imgset.sh "classic" "$(RM)" "$(LN)" "$(BASENAME)") 
else
	@$(shell scripts\\setup_imgset.bat classic)
endif
else
ifneq '$(ARCH)' 'Windows'
	@$(RM) error.log exclude.list > /dev/null
	@$(RM) -Rf tools/qmc2_options_editor_java/bin > /dev/null
	@$(RM) -f tools/qmc2_options_editor_java/*.jar > /dev/null
endif
ifeq '$(ARCH)' 'Darwin'
ifneq '$(QT_LIB48PLUS)' 'true'
	@xcodebuild -project Makefile.qmake.xcodeproj -alltargets -configuration Release clean > /dev/null
	@xcodebuild -project Makefile.qmake.xcodeproj -alltargets -configuration Debug clean > /dev/null
	@$(RM) -r $(TARGET_NAME).app build > /dev/null
	@$(RM) $(wildcard $(dir $(QMAKEFILE))*.mode* $(dir $(QMAKEFILE))*.pbxuser) > /dev/null
	@# this shouldn't be necessary, but qmake doesn't add a proper clean target to the project
	@$(RM) $(patsubst %.ui,ui_%.h,$(wildcard *.ui) $(notdir $(wildcard */*.ui))) > /dev/null
	@$(RM) $(wildcard moc_*.cpp) qrc_qmc2.cpp arch/Darwin/Info.plist Info.plist Makefile.qmake.xcodeproj/* > /dev/null
	@$(RMDIR) Makefile.qmake.xcodeproj > /dev/null
else
	@$(MAKE) -f $(QMAKEFILE) distclean > /dev/null
	@$(RM) arch/Darwin/Info.plist Info.plist > /dev/null
endif
else
ifneq '$(ARCH)' 'Windows'
	@$(MAKE) -f $(QMAKEFILE) distclean > /dev/null
else
	@$(MAKE) -f $(QMAKEFILE) distclean > NUL
	@$(RM) object_script.$(TARGET_NAME).Release object_script.$(TARGET_NAME).Debug $(TARGET_NAME).exe_resource.rc scripts\subwcrev.out > NUL
	@$(RMDIR) /s /q release > NUL
	@$(RMDIR) /s /q debug > NUL
endif
endif
ifneq '$(ARCH)' 'Windows'
	@$(shell scripts/setup_imgset.sh "classic" "$(RM)" "$(LN)" "$(BASENAME)" > /dev/null) 
else
	@$(shell scripts\\setup_imgset.bat classic)
endif
endif

ifneq '$(ARCH)' 'Windows'

# rules for creation of distribution archives
NOW = $(shell $(DATE))
SRCDIR = $(shell $(BASENAME) `pwd`)
snapshot: snap
snap: distclean
	@$(MAKE) xl
	@$(ECHO) -n "Creating source distribution snapshot for QMC2 v$(VERSION)-$(NOW)... "
	@$(CD) .. ; \
	$(TAR) -c -f - -X $(SRCDIR)/exclude.list $(SRCDIR) | bzip2 -9 > $(PROJECT)-$(VERSION)-$(NOW).tar.bz2 ; \
	$(TAR) -c -f - -X $(SRCDIR)/exclude.list $(SRCDIR) | gzip -9 > $(PROJECT)-$(VERSION)-$(NOW).tar.gz
	@$(RM) exclude.list
	@$(ECHO) "done"

distribution: dist
dist: distclean
	@$(MAKE) xl
	@$(ECHO) -n "Creating source distribution archive for QMC2 v$(VERSION)... "
	@$(CD) .. ; \
	$(TAR) -c -f - -X $(SRCDIR)/exclude.list $(SRCDIR) | bzip2 -9 > $(PROJECT)-$(VERSION).tar.bz2 ; \
	$(TAR) -c -f - -X $(SRCDIR)/exclude.list $(SRCDIR) | gzip -9 > $(PROJECT)-$(VERSION).tar.gz
	@$(RM) exclude.list
	@$(ECHO) "done"

endif

else
distribution: all
dist: all
snapshot: all
snap: all
lang: all
clean: all
install: all
bin: all
all:
	@$(ECHO) "Error: Wrong qmake version. Version 2 (Qt 4) or 3 (Qt 5) required!"
endif

ifneq '$(ARCH)' 'Windows'

xl: exclude-list
xlist: exclude-list
svn-exclude-list: exclude-list
exclude.list: exclude-list
exclude-list:
	@cd .. ; \
	$(FIND) $(PROJECT) -name "*svn*" > $(PROJECT)/exclude.list.new ; \
	$(ECHO) "$(PROJECT)/exclude.list" >> $(PROJECT)/exclude.list.new ; \
	$(CAT) $(PROJECT)/exclude.list.new | env LOCALE=C sort > $(PROJECT)/exclude.list ; \
	$(RM) $(PROJECT)/exclude.list.new

detect-os: os-detect
os-detect:
	@scripts/os-detect.sh

endif

?: help
help:
	@$(ECHO) "Usage: $(MAKE) [<targets>] [<configuration_options>]"
	@$(ECHO) ""
	@$(ECHO) "### Target ###   ### Description ###"
	@$(ECHO) "all (default)    Build the QMC2 main GUI, alias: $(PROJECT)"
	@$(ECHO) "clean            Clean up the build tree (QMC2 main GUI)"
	@$(ECHO) "config           Show build configuration options and their current values"
	@$(ECHO) "configure        Create qmake output and stop, alias: qmake"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "dist             Create source distribution archives (tar.gz and tar.bz2)"
endif
	@$(ECHO) "distclean        Clean up the complete source tree for 'distribution'"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "doc              Convert man-pages to troff/nroff format, alias: man"
	@$(ECHO) "doc-clean        Clean up converted man-pages, alias: man-clean"
	@$(ECHO) "doc-install      Install man-pages system-wide, alias: man-install"
endif
	@$(ECHO) "help | ?         Show this help"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "install          Install QMC2 main GUI binaries and data files for system-wide use"
endif
	@$(ECHO) "lang             Recreate QMC2 main GUI translation files only (if not up to date)"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "os-detect        Detect host OS and distribution / version"
	@$(ECHO) "snap             Create source distribution archives with date and time stamp"
endif
	@$(ECHO) "arcade           Build the QMC2 Arcade binary (qmc2-arcade)"
	@$(ECHO) "arcade-clean     Clean up the QMC2 Arcade build"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "arcade-install   Install QMC2 Arcade"
endif
	@$(ECHO) "qchdman          Build Qt CHDMAN GUI binary (qchdman)"
	@$(ECHO) "qchdman-clean    Clean up Qt CHDMAN GUI build"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "qchdman-install  Install Qt CHDMAN GUI"
endif
	@$(ECHO) "tools            Build tools: qchdman"
	@$(ECHO) "tools-clean      Clean up tools: qchdman-clean"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "tools-install    Install tools: qchdman-install"
endif
	@$(ECHO) ""
	@$(ECHO) "Run 'make config' for build configuration options."

config:
	@$(ECHO) "Current build configuration:"
	@$(ECHO) ""
	@$(ECHO) "### Option ###         ### Description ###                           ### Value ###" 
	@$(ECHO) "ARCH                   Target system's OS / architecture name        $(ARCH)"
	@$(ECHO) "AUDIOEFFECTDIALOGS     Enable audio-effect dialogs (0, 1)            $(AUDIOEFFECTDIALOGS)"
	@$(ECHO) "AWK                    UNIX command awk                              $(AWK)"
	@$(ECHO) "BASENAME               UNIX command basename                         $(BASENAME)"
	@$(ECHO) "BINDIR                 Binary directory for installation             $(BINDIR)"
	@$(ECHO) "BROWSER_EXTRAS         Enable browser extra features (0, 1)          $(BROWSER_EXTRAS)"
	@$(ECHO) "BROWSER_JAVA           Enable Java in web browsers (0, 1)            $(BROWSER_JAVA)"
	@$(ECHO) "BROWSER_JAVASCRIPT     Enable JavaScript in web browsers (0, 1)      $(BROWSER_JAVASCRIPT)"
	@$(ECHO) "BROWSER_PLUGINS        Enable browser plugins (0, 1)                 $(BROWSER_PLUGINS)"
	@$(ECHO) "BROWSER_PREFETCH_DNS   Enable prefetching of DNS lookups (0, 1)      $(BROWSER_PREFETCH_DNS)"
	@$(ECHO) "CC_FLAGS               Additional flags passed to the C compiler     $(CC_FLAGS)"
	@$(ECHO) "CXX_FLAGS              Additional flags passed to the C++ compiler   $(CXX_FLAGS)"
	@$(ECHO) "CCACHE                 Use a compiler cache (0, 1)                   $(CCACHE)"
	@$(ECHO) "CCACHE_CC              Command used for cached cc                    $(CCACHE_CC)"
	@$(ECHO) "CCACHE_CXX             Command used for cached c++                   $(CCACHE_CXX)"
	@$(ECHO) "CD                     UNIX command cd                               $(CD)"
	@$(ECHO) "CHMOD                  UNIX command chmod                            $(CHMOD)"
	@$(ECHO) "CHOWN                  UNIX command chown                            $(CHOWN)"
	@$(ECHO) "COLRM                  UNIX command colrm                            $(COLRM)"
	@$(ECHO) "CP                     UNIX command cp                               $(CP)"
	@$(ECHO) "CTIME                  Measure compilation & linkage time (0, 1)     $(CTIME)"
	@$(ECHO) "DATADIR                Data directory for installation               $(DATADIR)"
	@$(ECHO) "DATE                   UNIX command date                             $(DATE)"
	@$(ECHO) "DEBUG                  Choose debugging level (0, 1, 2)              $(DEBUG)"
	@$(ECHO) "DISTCC                 Use a distributed compiler (0, 1)             $(DISTCC)"
	@$(ECHO) "DISTCC_CC              Command used for distributed cc               $(DISTCC_CC)"
	@$(ECHO) "DISTCC_CXX             Command used for distributed c++              $(DISTCC_CXX)"
	@$(ECHO) "DISTCFG                Use distribution-specific config (0, 1)       $(DISTCFG)"
	@$(ECHO) "FADER_SPEED            Audio fading speed (0: fastest, >0: slower)   $(FADER_SPEED)"
	@$(ECHO) "FIND                   UNIX command find                             $(FIND)"
ifeq '$(ARCH)' 'Windows'
	@$(ECHO) "FORCE_MINGW            Force use of MinGW on Windows (0, 1)          $(FORCE_MINGW)"
endif
	@$(ECHO) "FORCE_PHONON           Force using Phonon even with Qt 5 (0, 1)      $(FORCE_PHONON)"
	@$(ECHO) "GREP                   UNIX command grep                             $(GREP)"
	@$(ECHO) "IMGSET                 Image set to be used                          $(IMGSET)"
	@$(ECHO) "JOYSTICK               Compile with SDL joystick support (0, 1)      $(JOYSTICK)"
	@$(ECHO) "L_FLAGS                Additional flags passed to the linker         $(L_FLAGS)"
	@$(ECHO) "L_LIBS                 Additional libraries passed to the linker     $(L_LIBS)"
	@$(ECHO) "L_LIBDIRS              Additional library paths for the linker       $(L_LIBDIRS)"
	@$(ECHO) "L_LIBDIRFLAGS          Optional value for QMAKE_LIBDIR_FLAGS         $(L_LIBDIRFLAGS)"
	@$(ECHO) "LINKER                 The linker to be used (empty = default)       $(LINKER)"
	@$(ECHO) "LN                     UNIX command ln                               $(LN)"
	@$(ECHO) "LOCAL_QML_IMPORT_PATH  Additional QML import path to use             $(LOCAL_QML_IMPORT_PATH)"
	@$(ECHO) "LRELEASE               Qt language release (lrelease) command        $(LRELEASE)"
	@$(ECHO) "LUPDATE                Qt language update (lupdate) command          $(LUPDATE)"
ifeq '$(ARCH)' 'Darwin'
	@$(ECHO) "MACDEPLOYQT            Qt's Mac OS X deployment tool                 $(MACDEPLOYQT)"
	@$(ECHO) "MAC_UNIVERSAL          Build a universal application bundle (0, 1)   $(MAC_UNIVERSAL)"
endif
	@$(ECHO) "MACHINE                Target system's machine type                  $(MACHINE)"
	@$(ECHO) "MAKE                   GNU make command                              $(MAKE)"
	@$(ECHO) "MAKESILENT             GNU make command (silent mode)                $(MAKESILENT)"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "MAN_DIR                Base directory for man-page installation      $(MAN_DIR)"
endif
	@$(ECHO) "MKDIR                  UNIX command mkdir                            $(MKDIR)"
	@$(ECHO) "MKSPEC                 Qt mkspec to be used (empty = default)        $(MKSPEC)"
	@$(ECHO) "MV                     UNIX command mv                               $(MV)"
	@$(ECHO) "OPENGL                 Enable miscellaneous OpenGL features (0, 1)   $(OPENGL)"
	@$(ECHO) "OSCFG                  Use global OS configuration (0, 1)            $(OSCFG)"
	@$(ECHO) "OSNAME                 Target system's OS name                       $(OSNAME)"
	@$(ECHO) "OSREL                  Target system's OS release                    $(OSREL)"
	@$(ECHO) "PHONON                 Enable Phonon based features (0, 1)           $(PHONON)"
	@$(ECHO) "PREFIX                 Prefix directory for install target           $(PREFIX)"
	@$(ECHO) "QMAKE                  Qt make (qmake) command                       $(QMAKE)"
	@$(ECHO) "QMAKEFILE              Qt generated Makefile name                    $(QMAKEFILE)"
	@$(ECHO) "QT_TRANSLATION         Specify path to Qt translations or 'qmc2'     $(QT_TRANSLATION)"
ifneq '$(ARCH)' 'Windows'
	@$(ECHO) "QT_LIBVERSION          Version of the Qt library in use              $(QT_LIBVERSION)"
endif
	@$(ECHO) "QUIET                  Suppress output of compile/link (0, 1)        $(QUIET)"
	@$(ECHO) "RM                     UNIX command rm                               $(RM)"
	@$(ECHO) "RMDIR                  UNIX command rmdir                            $(RMDIR)"
	@$(ECHO) "RSYNC                  UNIX command rsync                            $(RSYNC)"
	@$(ECHO) "SDL                    SDL major version to be used (1, 2)           $(SDL)"
	@$(ECHO) "SED                    UNIX command sed                              $(SED)"
	@$(ECHO) "SVNVERSION             UNIX command svnversion (optional)            $(SVNVERSION)"
	@$(ECHO) "SYSCONFDIR             System configuration directory                $(SYSCONFDIR)"
	@$(ECHO) "TAR                    UNIX command tar                              $(TAR)"
	@$(ECHO) "TARGET                 Name of the QMC2 'target executable'          $(TARGET_NAME)"
	@$(ECHO) "TR                     UNIX command tr                               $(TR)"
	@$(ECHO) "TIME                   UNIX command time                             $(TIME)"
	@$(ECHO) "WIP                    Enable unfinished code (0, 1)                 $(WIP)"
	@$(ECHO) "YOUTUBE                Enable support for YouTube videos (0, 1)      $(YOUTUBE)"
ifneq '$(SVN_REV)' ''
	@$(ECHO) ""
ifneq '$(SVN_REV)' '0'
	@$(ECHO) "The SVN revision of your working copy is $(SVN_REV)."
else
	@$(ECHO) "The SVN revision of your working copy could not be determined."
endif
endif

ifeq '$(ARCH)' 'Darwin'
xcp: $(QMAKEFILE)
xcode-project: $(QMAKEFILE)
configure: $(QMAKEFILE)
qmake: $(QMAKEFILE)
else
ifeq '$(ARCH)' 'Windows'
configure: $(QMAKEFILE) rcgen
qmake: $(QMAKEFILE) rcgen
else
configure: $(QMAKEFILE)
qmake: $(QMAKEFILE)
endif
endif

ifneq '$(ARCH)' 'Windows'
ifeq '$(ARCH)' 'Darwin'
DEFAULT_CONFIG_PATH=~/Library/Application Support/qmc2
else
DEFAULT_CONFIG_PATH=~/.qmc2
endif
doc: man
man:
	@scripts/make-man-pages.sh data/doc/man $(VERSION) $(ARCH) "$(DEFAULT_CONFIG_PATH)" "$(SYSCONFDIR)"

doc-clean: man-clean
man-clean:
	$(RM) data/doc/man/*.gz

doc-install: man-install
man-install:
	@$(ECHO) "Installing man-pages to $(DESTDIR)$(MAN_DIR)/man6"
	@$(MKDIR) $(DESTDIR)$(MAN_DIR)/man6
	@$(RSYNC) --exclude '*svn*' ./data/doc/man/*.gz $(DESTDIR)$(MAN_DIR)/man6/
endif

# process translations
ifneq '$(ARCH)' 'Windows'
LBINARIES = $(addsuffix .qm, $(addprefix data/lng/qmc2_, $(QMC2_TRANSLATIONS))) $(addsuffix .qm, $(addprefix data/lng/qt_, $(QT_TRANSLATIONS)))
else
LBINARIES = $(addsuffix .qm, $(addprefix data\lng\qmc2_, $(QMC2_TRANSLATIONS))) $(addsuffix .qm, $(addprefix data\lng\qt_, $(QT_TRANSLATIONS)))
endif
LREL = $(LRELEASE) $<

lang: $(LBINARIES)

ifneq '$(ARCH)' 'Windows'

# QMC2 translations

data/lng/qmc2_de.qm: data/lng/qmc2_de.ts
	$(LREL)

data/lng/qmc2_el.qm: data/lng/qmc2_el.ts
	$(LREL)

data/lng/qmc2_es.qm: data/lng/qmc2_es.ts
	$(LREL)

data/lng/qmc2_fr.qm: data/lng/qmc2_fr.ts
	$(LREL)

data/lng/qmc2_it.qm: data/lng/qmc2_it.ts
	$(LREL)

data/lng/qmc2_pl.qm: data/lng/qmc2_pl.ts
	$(LREL)

data/lng/qmc2_pt.qm: data/lng/qmc2_pt.ts
	$(LREL)

data/lng/qmc2_ro.qm: data/lng/qmc2_ro.ts
	$(LREL)

data/lng/qmc2_sv.qm: data/lng/qmc2_sv.ts
	$(LREL)

data/lng/qmc2_us.qm: data/lng/qmc2_us.ts
	$(LREL)

# Qt translations

data/lng/qt_de.qm: data/lng/qt_de.ts
	$(LREL)

#data/lng/qt_el.qm: data/lng/qt_el.ts
#	$(LREL)

data/lng/qt_es.qm: data/lng/qt_es.ts
	$(LREL)

data/lng/qt_fr.qm: data/lng/qt_fr.ts
	$(LREL)

#data/lng/qt_it.qm: data/lng/qt_it.ts
#	$(LREL)

data/lng/qt_pl.qm: data/lng/qt_pl.ts
	$(LREL)

data/lng/qt_pt.qm: data/lng/qt_pt.ts
	$(LREL)

#data/lng/qt_ro.qm: data/lng/qt_ro.ts
#	$(LREL)

data/lng/qt_sv.qm: data/lng/qt_sv.ts
	$(LREL)

#data/lng/qt_us.qm: data/lng/qt_us.ts
#	$(LREL)

else

# QMC2 translations

data\lng\qmc2_de.qm: data\lng\qmc2_de.ts
	$(LREL)

data\lng\qmc2_el.qm: data\lng\qmc2_el.ts
	$(LREL)

data\lng\qmc2_es.qm: data\lng\qmc2_es.ts
	$(LREL)

data\lng\qmc2_fr.qm: data\lng\qmc2_fr.ts
	$(LREL)

data\lng\qmc2_it.qm: data\lng\qmc2_it.ts
	$(LREL)

data\lng\qmc2_pl.qm: data\lng\qmc2_pl.ts
	$(LREL)

data\lng\qmc2_pt.qm: data\lng\qmc2_pt.ts
	$(LREL)

data\lng\qmc2_ro.qm: data\lng\qmc2_ro.ts
	$(LREL)

data\lng\qmc2_sv.qm: data\lng\qmc2_sv.ts
	$(LREL)

data\lng\qmc2_us.qm: data\lng\qmc2_us.ts
	$(LREL)

# Qt translations

data\lng\qt_de.qm: data\lng\qt_de.ts
	$(LREL)

#data\lng\qt_el.qm: data\lng\qt_el.ts
#	$(LREL)

data\lng\qt_es.qm: data\lng\qt_es.ts
	$(LREL)

data\lng\qt_fr.qm: data\lng\qt_fr.ts
	$(LREL)

#data\lng\qt_it.qm: data\lng\qt_it.ts
#	$(LREL)

data\lng\qt_pl.qm: data\lng\qt_pl.ts
	$(LREL)

data\lng\qt_pt.qm: data\lng\qt_pt.ts
	$(LREL)

#data\lng\qt_ro.qm: data\lng\qt_ro.ts
#	$(LREL)

data\lng\qt_sv.qm: data\lng\qt_sv.ts
	$(LREL)

#data\lng\qt_us.qm: data\lng\qt_us.ts
#	$(LREL)


endif

# end of file
