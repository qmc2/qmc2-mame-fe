# >>> START OF MAKE OPTIONS <<<
#
# You can either edit the default values below or (recommended) specify them on
# the 'make' command line (non-string options could alternatively be defined as
# environment variables).
#
# Please don't change anything below the line containing END OF MAKE OPTIONS!
#

# >>> EMULATOR <<<
#
# Specifies the target emulator to be used (important: you need to build QMC2
# for each target emulator separately; do a "make clean" between the builds!)
#
# Supported emulators:
#
# UNIX and Mac OS X .... SDLMAME or SDLMESS
# Windows .............. MAME, MAMEUIFX32 or MESS
#
ifndef EMULATOR
EMULATOR = SDLMAME
endif

# >>> PREFIX <<<
#
# The prefix directory used by the 'make install' target.
#
ifndef PREFIX
PREFIX = /usr/local
endif

# >>> QUIET <<<
#
# Compile and link QMC2 'quietly' (1) or show all command lines completely (0)?
#
# See also PRETTY below!
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
ARCH = $(shell uname)
endif

# >>> OSREL <<<
#
# Target system's OS-release -- please only change this if you really know what
# you're doing :)!
#
ifndef OSREL
OSREL = $(shell uname -r)
endif

# >>> MACHINE <<<
#
# Target system's machine/CPU type -- please only change this if you really know
# what you're doing :)!
ifndef MACHINE
MACHINE = $(shell uname -m)
endif

# >>> DATADIR <<<
#
# The data directory used by the 'make install' target.
#
ifndef DATADIR
ifeq '$(ARCH)' 'Darwin'
DATADIR = /Library/Application Support
else
DATADIR = $(PREFIX)/share
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
SYSCONFDIR = /etc
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
BINDIR = $(PREFIX)/bin
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

# >>> ARCADE_OPENGL <<<
#
# Enable (1) or disable (0) OpenGL support for the 'arcade mode'.
#
# Requires OpenGL if enabled (hardware acceleration recommended).
#
# In contrast to the other OpenGL features (see OPENGL above), this is really
# useful! If you disable it, the pure software renderer will be used (which is
# remarkably slower).
#
# Note that the arcade mode is still a 'work in progress' (see WIP below) and is
# thus disabled by default. So if you don't enable WIP as well, this option will
# be ignored.
#
ifndef ARCADE_OPENGL
ARCADE_OPENGL = 1
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

# >>> PHONON <<<
#
# Enable Phonon based features (1) or leave them out of the build (0).
#
# Requires libphonon and a working Phonon backend (such as gstreamer or xine on
# Linux, DirectX 9+ on Windows and QuickTime 7+ on Mac OS X).
#
# Phonon is currently only used by the built-in MP3 player.
#
ifndef PHONON
PHONON = 1
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

# >>> PRETTY <<<
#
# Enable (1) or disable (0) 'pretty' compilation output.
#
ifndef PRETTY
PRETTY = 1
endif

# >>> SDLLOCAL <<<
#
# Enable (1) or disable (0) the use of a 'local' SDL library installation.
#
# Make sure to also set SDLLOCAL_INC and SDLLOCAL_LIB if you enable this!
#
ifndef SDLLOCAL
SDLLOCAL = 0
endif

# >>> SDLLOCAL_INC <<<
#
# Base include directory of the 'local' SDL headers.
#
ifndef SDLLOCAL_INC
SDLLOCAL_INC = /usr/local/include
endif

# >>> SDLLOCAL_LIB <<<
#
# Base library directory of the 'local' SDL installation.
#
ifndef SDLLOCAL_LIB
SDLLOCAL_LIB = /usr/local/lib
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

# >>> VARIANT_LAUNCHER <<<
#
# Enable (1) or disable (0) QMC2's 'variant launching'.
#
ifndef VARIANT_LAUNCHER
VARIANT_LAUNCHER = 1
endif

# >>> AUDIT_WILDCARD <<<
#
# Use wildcards (1) on 'full audit' calls (i.e. 'mame -listxml *') or not (0)?
#
# You normally don't need to enable this! It has been added for a single user's
# strangely behaving system where this fixed his issues :)...
#
ifndef AUDIT_WILDCARD
AUDIT_WILDCARD = 0
endif

# >>> BROWSER_EXTRAS <<<
#
# Enable (1) or disable (0) extra browser features such as QtWebKit's 'Web
# Inspector'.
#
# Caution: these features may be buggy and can easily make QMC2 crash! However,
# when Qt 4.6+ is used, the extra browser features should work fine.
#
ifndef BROWSER_EXTRAS
BROWSER_EXTRAS = 0
endif

# >>> BROWSER_PLUGINS <<<
#
# Enable (1) or disable (0) Netscape/Mozilla plugins in the 'MiniWebBrowser'?
#
# Caution: browser plugins may be buggy and can cause crashes! However, when
# Qt 4.7+ is used, browser plugins should work fine.
#
ifndef BROWSER_PLUGINS
BROWSER_PLUGINS = 0
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

# >>> WC_COMPRESSION <<<
#
# Enable (1) or disable (0) web-cache compression when storing HTML pages on
# disk?
#
# This is used by the MAWS web-cache -- and has nothing to do with a toilet :).
#
# Note that web-cache compression doesn't work on Windows yet and will thus be
# automatically disabled on that platform, no matter what value this option has
# been set to.
#
ifndef WC_COMPRESSION
WC_COMPRESSION = 1
endif

# >>> FADER_SPEED <<<
#
# Select the audio fading speed of the MP3 player.
#
# FADER_SPEED = 0 .... Pause/resume instantly (fastest)
# FADER_SPEED > 0 .... Fading pause/resume (slower)
#
ifndef FADER_SPEED
FADER_SPEED = 500
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
# Specify additional libraries directories passed to the linker.
#
ifndef L_LIBDIRS
L_LIBDIRS =
endif

# >>> LINKER <<<
#
# Specify (overwrite) the linker to be used (useful for cross-compilation).
#
ifndef LINKER
LINKER =
endif

# >>> MKSPEC <<<
#
# Specify (overwrite) the Qt mkspec (qmake spec) to be used.
#
ifndef MKSPEC
MKSPEC =
endif

# >>> XWININFO <<<
#
# Specify the command to be used to run 'xwininfo'.
#
# This is used by the emulator embedder which is only available on Qt platforms
# that utilize X11.
#
ifndef XWININFO
XWININFO = xwininfo
endif

# >>> DATABASE <<<
#
# Enable (1) or disable (0) the use of any optional features that make use of
# relational databases. The only supported DB type is MySQL (SQLite is planned
# as well).
#
# WARNING: this is a WIP feature under development right now - use at your own
# risk! 
#
ifndef DATABASE
DATABASE = 0
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

# >>> END OF MAKE OPTIONS <<<

# project name
PROJECT = qmc2

# version
VERSION_MAJOR = 0
VERSION_MINOR = 2
VERSION_BETA  = 17

# emulator target fallback for mameuifx32
ifeq '$(EMULATOR)' 'MAMEUIFX32'
EMULATOR = MAME
endif

# commands are platform/distribution-specific
include arch/default.cfg
ifeq '$(OSCFG)' '1'
OSCFGFILEEXISTS = $(shell ls "arch/$(ARCH).cfg")
ifeq 'arch/$(ARCH).cfg' '$(OSCFGFILEEXISTS)'
include arch/$(ARCH).cfg
endif
endif
ifeq '$(DISTCFG)' '1'
DISTCFGFILE = $(shell scripts/os-detect.sh | $(GREP) "Distribution cfg-file" | $(COLRM) 1 30)
DISTCFGFILEEXISTS = $(shell ls "$(DISTCFGFILE)")
ifeq '$(DISTCFGFILE)' '$(DISTCFGFILEEXISTS)'
include $(DISTCFGFILE)
endif
endif

# global QMC2 configuration file
GLOBAL_QMC2_INI=$(shell echo $(DESTDIR)/$(SYSCONFDIR)/$(PROJECT)/$(PROJECT).ini | $(SED) -e "s*//*/*g")

# global data directory
GLOBAL_DATADIR=$(shell echo $(DESTDIR)/$(DATADIR) | $(SED) -e "s*//*/*g")

# qmake version check (major release)
ifndef QMAKEV
QMAKEV = $(shell echo `$(QMAKE) -v` | $(AWK) '{print $$3}' | $(COLRM) 2)
endif
ifeq '$(QMAKEV)' '2'

# version strings used by 'make dist' and 'make snap'
ifneq '$(VERSION_BETA)' '0'
VERSION     = $(VERSION_MAJOR).$(VERSION_MINOR).b$(VERSION_BETA)
VERSIONDEFS = MAJOR=$(VERSION_MAJOR) MINOR=$(VERSION_MINOR) BETA=$(VERSION_BETA)
else
VERSION     = $(VERSION_MAJOR).$(VERSION_MINOR)
VERSIONDEFS = MAJOR=$(VERSION_MAJOR) MINOR=$(VERSION_MINOR)
endif

# work around for qmake's issues with spaces in values
blank =
space = $(blank) $(blank)

# pre-compiler definitions (passed to qmake)
DEFINES = DEFINES+=$(VERSIONDEFS) BUILD_OS_NAME=$(OSNAME) BUILD_OS_RELEASE=$(OSREL) BUILD_MACHINE=$(MACHINE) PREFIX=$(PREFIX) DATADIR="$(subst $(space),:,$(DATADIR))" SYSCONFDIR="$(subst $(space),:,$(SYSCONFDIR))" QMC2_JOYSTICK=$(JOYSTICK) QMC2_OPENGL=$(OPENGL) QMC2_ARCADE_OPENGL=$(ARCADE_OPENGL) QMC2_WIP_CODE=$(WIP) QMC2_PHONON=$(PHONON) QMC2_FADER_SPEED=$(FADER_SPEED) QMC2_XWININFO=$(XWININFO)

# process make options
ifeq '$(DEBUG)' '2'
DEFINES += QMC2_DEBUG
endif
DEFINES += QMC2_$(EMULATOR)

ifeq '$(ARCH)' 'Darwin'
DEFINES += QMC2_MAC_UNIVERSAL=$(MAC_UNIVERSAL)
endif

ifeq '$(VARIANT_LAUNCHER)' '1'
DEFINES += QMC2_VARIANT_LAUNCHER
endif

ifeq '$(AUDIT_WILDCARD)' '1'
DEFINES += QMC2_AUDIT_WILDCARD
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

ifeq '$(WC_COMPRESSION)' '1'
DEFINES += QMC2_WC_COMPRESSION_ENABLED
endif

ifeq '$(DATABASE)' '1'
DEFINES += QMC2_DATABASE_ENABLED
endif

# setup SDL library and include paths
ifdef SDL_LIBS
undef SDL_LIBS
endif
ifeq '$(JOYSTICK)' '1'
ifeq '$(SDLLOCAL)' '1'
SDL_LIBS += INCLUDEPATH+=$(SDLLOCAL_INC) LIBS+=-L$(SDLLOCAL_LIB)
endif
endif

# setup additional qmake features for release or debug builds
ifdef QMAKE_CONF
undef QMAKE_CONF
endif

# setup TARGET's application icon and generic name
ifeq '$(EMULATOR)' 'SDLMESS'
EMUICO = mess.png
GENERICNAME = M.E.S.S. Catalog / Launcher II
else
EMUICO = mame.png
GENERICNAME = M.A.M.E. Catalog / Launcher II
endif

TARGET_NAME = $(PROJECT)-$(shell echo $(EMULATOR) | $(TR) [A-Z] [a-z])
QMAKE_CONF = TARGET=$(TARGET_NAME)
ifeq '$(DEBUG)' '0'
QMAKE_CONF += CONFIG+=warn_off CONFIG+=release
else
QMAKE_CONF += CONFIG+=warn_on CONFIG+=debug
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
else
ifeq '$(ARCADE_OPENGL)' '1'
QT_CONF += QT+=opengl
endif
endif

ifeq '$(PHONON)' '1'
QT_CONF += QT+=phonon
endif

ifeq '$(DATABASE)' '1'
QT_CONF += QT+=sql
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

# put the version and SCM revision in the Info.plist for OS X
%.plist: %.plist.in
	@$(SED) -e 's/@SHORT_VERSION@/$(subst /,\/,$(VERSION))/g' -e 's/@SCM_REVISION@/$(subst /,\/,$(shell svnversion | $(SED) -e 's/[MS]//g' -e 's/^[[:digit:]]*://'))/g' < $< > $@

ifeq '$(ARCH)' 'Darwin'
$(QMAKEFILE): macx/Info.plist
endif

ifeq '$(QUIET)' '1'
$(PROJECT)-bin: lang $(QMAKEFILE)
ifeq '$(ARCH)' 'Darwin'
ifeq '$(CTIME)' '0'
	+@xcodebuild -project Makefile.qmake.xcodeproj -configuration Release > /dev/null && cd runonce && $(QMAKE) -makefile QMC2_PRETTY_COMPILE=$(PRETTY) -o Makefile.qmake runonce.pro > /dev/null && xcodebuild -project Makefile.qmake.xcodeproj -configuration Release > /dev/null
else
	+@$(TIME) (xcodebuild -project Makefile.qmake.xcodeproj -configuration Release > /dev/null && cd runonce && $(QMAKE) -makefile QMC2_PRETTY_COMPILE=$(PRETTY) -o Makefile.qmake runonce.pro > /dev/null && xcodebuild -project Makefile.qmake.xcodeproj -configuration Release > /dev/null)
endif
else
ifeq '$(CTIME)' '0'
	+@$(MAKESILENT) -f $(QMAKEFILE) > /dev/null && cd runonce && $(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) QMC2_PRETTY_COMPILE=$(PRETTY) runonce.pro > /dev/null && $(MAKESILENT) -f $(QMAKEFILE) > /dev/null
else
	+@$(TIME) ($(MAKESILENT) -f $(QMAKEFILE) > /dev/null && cd runonce && $(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) QMC2_PRETTY_COMPILE=$(PRETTY) runonce.pro > /dev/null && $(MAKESILENT) -f $(QMAKEFILE) > /dev/null)
endif
endif
	@$(RM) data/opt/template.xml > /dev/null
ifeq '$(EMULATOR)' 'SDLMAME'
	@$(LN) -s SDLMAME/template.xml data/opt/template.xml > /dev/null
endif
ifeq '$(EMULATOR)' 'SDLMESS'
	@$(LN) -s SDLMESS/template.xml data/opt/template.xml > /dev/null
endif
	@echo "Build of QMC2 v$(VERSION) complete"
	@echo "Target emulator: $(EMULATOR)"

$(QMAKEFILE): $(PROJECT).pro
	@echo "Configuring build of QMC2 v$(VERSION)"
	@$(shell scripts/setup_imgset.sh "$(IMGSET)" "$(RM)" "$(LN)" "$(BASENAME)" > /dev/null) 
	@$(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) VERSION=$(VERSION) VER_MAJ=$(VERSION_MAJOR) VER_MIN=$(VERSION_MINOR) QMC2_PRETTY_COMPILE=$(PRETTY) $(QMAKE_CONF) $(SDL_LIBS) $(QT_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_LINKER) '$(DEFINES)' $< > /dev/null
ifeq '$(ARCH)' 'Darwin'
	@$(SED) -e "s/-c /cc -c /" < ./Makefile.qmake.xcodeproj/qt_preprocess.mak > "./Makefile.qmake.xcodeproj/qt_preprocess.mak.new"
	@$(RM) ./Makefile.qmake.xcodeproj/qt_preprocess.mak
	@$(MV) ./Makefile.qmake.xcodeproj/qt_preprocess.mak.new ./Makefile.qmake.xcodeproj/qt_preprocess.mak
endif
else
$(PROJECT)-bin: lang $(QMAKEFILE)
ifeq '$(ARCH)' 'Darwin'
ifeq '$(CTIME)' '0'
	+@xcodebuild -project Makefile.qmake.xcodeproj -configuration Release && cd runonce && $(QMAKE) -makefile QMC2_PRETTY_COMPILE=$(PRETTY) -o Makefile.qmake runonce.pro && xcodebuild -project Makefile.qmake.xcodeproj -configuration Release
else
	+@$(TIME) (xcodebuild -project Makefile.qmake.xcodeproj -configuration Release && cd runonce && $(QMAKE) -makefile QMC2_PRETTY_COMPILE=$(PRETTY) -o Makefile.qmake runonce.pro && xcodebuild -project Makefile.qmake.xcodeproj -configuration Release)
endif
else
ifeq '$(CTIME)' '0'
	+@$(MAKE) -f $(QMAKEFILE) && cd runonce && $(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_LINKER) QMC2_PRETTY_COMPILE=$(PRETTY) runonce.pro && $(MAKE) -f $(QMAKEFILE)
else
	+@$(TIME) ($(MAKE) -f $(QMAKEFILE) && cd runonce && $(QMAKE) -makefile -o Makefile.qmake $(QT_MAKE_SPEC) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_LINKER) QMC2_PRETTY_COMPILE=$(PRETTY) runonce.pro && $(MAKE) -f $(QMAKEFILE))
endif
endif
	@$(RM) data/opt/template.xml
ifeq '$(EMULATOR)' 'SDLMAME'
	@$(LN) -s SDLMAME/template.xml data/opt/template.xml
endif
ifeq '$(EMULATOR)' 'SDLMESS'
	@$(LN) -s SDLMESS/template.xml data/opt/template.xml
endif
	@echo "Build of QMC2 v$(VERSION) complete"
	@echo "Target emulator: $(EMULATOR)"

$(QMAKEFILE): $(PROJECT).pro
	@echo "Configuring build of QMC2 v$(VERSION)"
	@$(shell scripts/setup_imgset.sh "$(IMGSET)" "$(RM)" "$(LN)" "$(BASENAME)") 
	$(QMAKE) -makefile $(QT_MAKE_SPEC) -o Makefile.qmake VERSION=$(VERSION) VER_MAJ=$(VERSION_MAJOR) VER_MIN=$(VERSION_MINOR) QMC2_PRETTY_COMPILE=$(PRETTY) $(QMAKE_CONF) $(SDL_LIBS) $(QT_CONF) $(QMAKE_CXX_COMPILER) $(QMAKE_CXX_FLAGS) $(QMAKE_CC_FLAGS) $(QMAKE_L_FLAGS) $(QMAKE_L_LIBS) $(QMAKE_L_LIBDIRS) $(QMAKE_LINKER) '$(DEFINES)' $<
ifeq '$(ARCH)' 'Darwin'
	@$(SED) -e "s/-c /cc -c /" < ./Makefile.qmake.xcodeproj/qt_preprocess.mak > "./Makefile.qmake.xcodeproj/qt_preprocess.mak.new"
	@$(RM) ./Makefile.qmake.xcodeproj/qt_preprocess.mak
	@$(MV) ./Makefile.qmake.xcodeproj/qt_preprocess.mak.new ./Makefile.qmake.xcodeproj/qt_preprocess.mak
endif
endif

ifeq '$(ARCH)' 'Darwin'
$(TARGET_NAME).app/Contents/Resources/qt.conf:
	@$(MACDEPLOYQT) $(TARGET_NAME).app
macdeployqt: $(TARGET_NAME).app/Contents/Resources/qt.conf

install: bin macdeployqt
else
install: bin
endif
	@echo "Installing QMC2 v$(VERSION)"
	@$(MKDIR) "$(DESTDIR)/$(BINDIR)" "$(DESTDIR)/$(DATADIR)/$(PROJECT)" "$(DESTDIR)/$(SYSCONFDIR)/$(PROJECT)"
ifeq '$(ARCH)' 'Darwin'
	@$(MKDIR) "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
	@$(CHMOD) a+rx "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
	@$(RSYNC) --exclude '*svn*' "./$(TARGET_NAME).app" "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
else
	@$(RM) -f "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
	@$(RSYNC) --exclude '*svn*' "./$(TARGET_NAME)" "$(DESTDIR)/$(BINDIR)"
	@$(RSYNC) --exclude '*svn*' "./runonce/runonce" "$(DESTDIR)/$(BINDIR)"
	@(cd "$(DESTDIR)/$(BINDIR)" && $(LN) -s "$(TARGET_NAME)" "$(PROJECT)")
endif
	@$(RSYNC) --exclude '*svn*' ./data/lng/qmc2_*.qm "$(GLOBAL_DATADIR)/$(PROJECT)/lng/"
	@if [ "$(QT_TRANSLATION)" = "qmc2" ] ; then \
	  $(RSYNC) --exclude '*svn*' ./data/lng/qt_*.qm "$(GLOBAL_DATADIR)/$(PROJECT)/lng/" ; \
	else \
	  echo "Using Qt translation files from $(QT_TRANSLATION)" ; \
	  $(RM) -f "$(GLOBAL_DATADIR)/$(PROJECT)/lng/"qt_*.qm ; \
	  $(LN) -s "$(QT_TRANSLATION)/qt_de.qm" "$(GLOBAL_DATADIR)/$(PROJECT)/lng/qt_de.qm" ; \
	  $(LN) -s "$(QT_TRANSLATION)/qt_pl.qm" "$(GLOBAL_DATADIR)/$(PROJECT)/lng/qt_pl.qm" ; \
	  $(LN) -s "$(QT_TRANSLATION)/qt_fr.qm" "$(GLOBAL_DATADIR)/$(PROJECT)/lng/qt_fr.qm" ; \
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
	@if [ -f "$(GLOBAL_QMC2_INI)" ] ; then \
	  echo "Preserving system-wide configuration in $(GLOBAL_QMC2_INI)" ; \
	  echo "Installing new system-wide configuration as $(GLOBAL_QMC2_INI).new" ; \
	  $(SED) -e "s*DATADIR*$(DATADIR)*g" < ./inst/$(PROJECT).ini.template > "$(GLOBAL_QMC2_INI).new" ; \
	else \
	  echo "Installing system-wide configuration as $(GLOBAL_QMC2_INI)" ; \
	  $(SED) -e "s*DATADIR*$(DATADIR)*g" < ./inst/$(PROJECT).ini.template > "$(GLOBAL_QMC2_INI)" ; \
	fi
ifneq '$(ARCH)' 'Darwin'
	@echo "Installing $(TARGET_NAME).desktop to $(GLOBAL_DATADIR)/applications"
	@$(MKDIR) $(GLOBAL_DATADIR)/applications
	@$(CHMOD) a+rx $(GLOBAL_DATADIR)/applications
	@$(SED) -e "s*DATADIR*$(DATADIR)*g; s*EMULATOR*$(EMULATOR)*g; s*TARGET*$(TARGET_NAME)*g; s*EMUICO*$(EMUICO)*g; s*GENERICNAME*$(GENERICNAME)*g" < ./inst/$(PROJECT).desktop.template > $(GLOBAL_DATADIR)/applications/$(TARGET_NAME).desktop
endif
	@echo "Installation complete"

distclean: clean
clean: $(QMAKEFILE)
	@echo "Cleaning up build of QMC2 v$(VERSION)"
ifeq '$(QUIET)' '0'
	@$(RM) data/opt/template.xml error.log
	@$(RM) runonce/runonce
ifeq '$(ARCH)' 'Darwin'
	@$(RM) -r runonce/Makefile.qmake.xcodeproj runonce/build
	@xcodebuild -project Makefile.qmake.xcodeproj -alltargets -configuration Release clean
	@xcodebuild -project Makefile.qmake.xcodeproj -alltargets -configuration Debug clean
	@$(RM) -r build $(TARGET_NAME).app
	@$(RM) $(wildcard $(dir $(QMAKEFILE))*.mode* $(dir $(QMAKEFILE))*.pbxuser)
	@# this shouldn't be necessary, but qmake doesn't add a proper clean target to the project
	@$(RM) $(patsubst %.ui,ui_%.h,$(wildcard *.ui) $(notdir $(wildcard */*.ui)))
	@$(RM) $(wildcard moc_*.cpp) qrc_qmc2.cpp macx/Info.plist Info.plist Makefile.qmake.xcodeproj/*
	@$(RMDIR) Makefile.qmake.xcodeproj
else
	@$(RM) runonce/runonce.o runonce/$(QMAKEFILE)
	@$(MAKE) -f $(QMAKEFILE) distclean
endif
	@$(shell scripts/setup_imgset.sh "classic" "$(RM)" "$(LN)" "$(BASENAME)") 
else
	@$(RM) data/log/* data/tmp/* data/cat/* data/opt/template.xml error.log > /dev/null
	@$(RM) runonce/runonce > /dev/null
ifeq '$(ARCH)' 'Darwin'
	@$(RM) -r runonce/Makefile.qmake.xcodeproj runonce/build > /dev/null
	@xcodebuild -project Makefile.qmake.xcodeproj -alltargets -configuration Release clean > /dev/null
	@xcodebuild -project Makefile.qmake.xcodeproj -alltargets -configuration Debug clean > /dev/null
	@$(RM) -r $(TARGET_NAME).app build > /dev/null
	@echo $(RM) $(dir $(QMAKEFILE))
	@$(RM) $(wildcard $(dir $(QMAKEFILE))*.mode* $(dir $(QMAKEFILE))*.pbxuser) > /dev/null
	@# this shouldn't be necessary, but qmake doesn't add a proper clean target to the project
	@$(RM) $(patsubst %.ui,ui_%.h,$(wildcard *.ui) $(notdir $(wildcard */*.ui))) > /dev/null
	@$(RM) $(wildcard moc_*.cpp) qrc_qmc2.cpp macx/Info.plist Info.plist Makefile.qmake.xcodeproj/* > /dev/null
	@$(RMDIR) Makefile.qmake.xcodeproj > /dev/null
else
	@$(RM) runonce/runonce.o runonce/$(QMAKEFILE) > /dev/null
	@$(MAKE) -f $(QMAKEFILE) distclean > /dev/null
endif
	@$(shell scripts/setup_imgset.sh "classic" "$(RM)" "$(LN)" "$(BASENAME)" > /dev/null) 
endif

# rules for creation of distribution archives
NOW = $(shell $(DATE))
snapshot: snap
snap: clean
	@echo "Creating source distribution snapshot for QMC2 v$(VERSION)-$(NOW)"
	$(CD) .. ; \
	$(TAR) -c -f - -X $(PROJECT)/exclude.list $(PROJECT) | bzip2 -9 > $(PROJECT)-$(VERSION)-$(NOW).tar.bz2 ; \
	$(TAR) -c -f - -X $(PROJECT)/exclude.list $(PROJECT) | gzip -9 > $(PROJECT)-$(VERSION)-$(NOW).tar.gz

distribution: dist
dist: clean
	@echo "Creating source distribution archive for QMC2 v$(VERSION)"
	$(CD) .. ; \
	$(TAR) -c -f - -X $(PROJECT)/exclude.list $(PROJECT) | bzip2 -9 > $(PROJECT)-$(VERSION).tar.bz2 ; \
	$(TAR) -c -f - -X $(PROJECT)/exclude.list $(PROJECT) | gzip -9 > $(PROJECT)-$(VERSION).tar.gz

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
	@echo "Error: Wrong Qmake version. Qmake version 2 (Qt 4) required!"
endif

svn-exclude-list: exclude-list
exclude-list:
	cd .. ; \
	$(FIND) qmc2 -name "*svn*" | env LOCALE=C sort > qmc2/exclude.list

detect-os: os-detect
os-detect:
	@scripts/os-detect.sh

?: help
help:
	@echo "Usage: $(MAKE) [<targets>] [<configuration_options>]"
	@echo ""
	@echo "### Target ###  ### Description ###"
	@echo "all (default)   Build QMC2, aliases: $(PROJECT), bin, $(PROJECT)-bin"
	@echo "clean           Clean up compilation & linkage binaries from source tree"
	@echo "config          Show current build configurion"
	@echo "dist            Create source distribution archives (tar.gz and tar.bz2)"
	@echo "exclude-list    Recreate SVN exclude-list (only needed by developers)"
	@echo "help | ?        Show make command line help and current build configuration"
	@echo "install         Install QMC2 binaries and data files for system-wide use"
	@echo "lang            Recreate binary translation files only (if not up to date)"
	@echo "os-detect       Detect host OS and distribution / version"
	@echo "snap            Create source distribution archives with date and time stamp"
	@echo ""
	@$(MAKE) config | $(GREP) -v "Entering directory" | $(GREP) -v "Leaving directory"

config:
	@echo "Current build configuration:"
	@echo ""
	@echo "### Option ###       ### Description ###                          ### Value ###" 
	@echo "ARCADE_OPENGL        Enable use of OpenGL for arcade mode (0, 1)  $(ARCADE_OPENGL)"
	@echo "ARCH                 Target system's OS / architecture name       $(ARCH)"
	@echo "AUDIT_WILDCARD       Use wildcards on full audit calls (0, 1)     $(AUDIT_WILDCARD)"
	@echo "AWK                  UNIX command awk                             $(AWK)"
	@echo "BASENAME             UNIX command basename                        $(BASENAME)"
	@echo "BINDIR               Binary directory for installation            $(BINDIR)"
	@echo "BROWSER_EXTRAS       Enable browser extra features (0, 1)         $(BROWSER_EXTRAS)"
	@echo "BROWSER_JAVA         Enable Java in web browsers (0, 1)           $(BROWSER_JAVA)"
	@echo "BROWSER_JAVASCRIPT   Enable JavaScript in web browsers (0, 1)     $(BROWSER_JAVASCRIPT)"
	@echo "BROWSER_PLUGINS      Enable browser plugins (0, 1)                $(BROWSER_PLUGINS)"
	@echo "CC_FLAGS             Additional flags passed to the C compiler    $(CC_FLAGS)"
	@echo "CXX_FLAGS            Additional flags passed to the C++ compiler  $(CXX_FLAGS)"
	@echo "CCACHE               Use a compiler cache (0, 1)                  $(CCACHE)"
	@echo "CCACHE_CC            Command used for cached cc                   $(CCACHE_CC)"
	@echo "CCACHE_CXX           Command used for cached c++                  $(CCACHE_CXX)"
	@echo "CD                   UNIX command cd                              $(CD)"
	@echo "CHMOD                UNIX command chmod                           $(CHMOD)"
	@echo "CHOWN                UNIX command chown                           $(CHOWN)"
	@echo "COLRM                UNIX command colrm                           $(COLRM)"
	@echo "CP                   UNIX command cp                              $(CP)"
	@echo "CTIME                Measure compilation & linkage time (0, 1)    $(CTIME)"
	@echo "DATABASE             Enable database features (0, 1)              $(DATABASE)"
	@echo "DATADIR              Data directory for installation              $(DATADIR)"
	@echo "DATE                 UNIX command date                            $(DATE)"
	@echo "DEBUG                Choose debugging level (0, 1, 2)             $(DEBUG)"
	@echo "DISTCC               Use a distributed compiler (0, 1)            $(DISTCC)"
	@echo "DISTCC_CC            Command used for distributed cc              $(DISTCC_CC)"
	@echo "DISTCC_CXX           Command used for distributed c++             $(DISTCC_CXX)"
	@echo "DISTCFG              Use distribution-specific config (0, 1)      $(DISTCFG)"
	@echo "EMULATOR             Target emulator (SDLMAME, SDLMESS)           $(EMULATOR)"
	@echo "FADER_SPEED          Audio fading speed (0: fastest, >0: slower)  $(FADER_SPEED)"
	@echo "FIND                 UNIX command find                            $(FIND)"
	@echo "GREP                 UNIX command grep                            $(GREP)"
	@echo "IMGSET               Image set to be used                         $(IMGSET)"
	@echo "JOYSTICK             Compile with SDL joystick support (0, 1)     $(JOYSTICK)"
	@echo "L_FLAGS              Additional flags passed to the linker        $(L_FLAGS)"
	@echo "L_LIBS               Additional libraries passed to the linker    $(L_LIBS)"
	@echo "L_LIBDIRS            Additional libraries paths for the linker    $(L_LIBDIRS)"
	@echo "LINKER               The linker to be used (empty = default)      $(LINKER)"
	@echo "LN                   UNIX command ln                              $(LN)"
	@echo "LRELEASE             Qt language release (lrelease) command       $(LRELEASE)"
	@echo "LUPDATE              Qt language update (lupdate) command         $(LUPDATE)"
ifeq '$(ARCH)' 'Darwin'
	@echo "MACDEPLOYQT          Qt's Mac OS X deployment tool                $(MACDEPLOYQT)"
	@echo "MAC_UNIVERSAL        Build a universal application bundle (0, 1)  $(MAC_UNIVERSAL)"
endif
	@echo "MACHINE              Target system's machine type                 $(MACHINE)"
	@echo "MAKE                 GNU make command                             $(MAKE)"
	@echo "MAKESILENT           GNU make command (silent mode)               $(MAKESILENT)"
	@echo "MKDIR                UNIX command mkdir                           $(MKDIR)"
	@echo "MKSPEC               Qt mkspec to be used (empty = default)       $(MKSPEC)"
	@echo "MV                   UNIX command mv                              $(MV)"
	@echo "OPENGL               Enable miscellaneous OpenGL features (0, 1)  $(OPENGL)"
	@echo "OSCFG                Use global OS configuration (0, 1)           $(OSCFG)"
	@echo "OSNAME               Target system's OS name                      $(OSNAME)"
	@echo "OSREL                Target system's OS release                   $(OSREL)"
	@echo "PHONON               Enable Phonon based features (0, 1)          $(PHONON)"
	@echo "PREFIX               Prefix directory for install target          $(PREFIX)"
	@echo "PRETTY               Use qmake hacks for pretty compile output    $(PRETTY)"
	@echo "QMAKE                Qt make (qmake) command                      $(QMAKE)"
	@echo "QMAKEFILE            Qt generated Makefile name                   $(QMAKEFILE)"
	@echo "QT_TRANSLATION       Specify path to Qt translations or 'qmc2'    $(QT_TRANSLATION)"
	@echo "QUIET                Suppress output of compile/link (0, 1)       $(QUIET)"
	@echo "RM                   UNIX command rm                              $(RM)"
	@echo "RMDIR                UNIX command rmdir                           $(RMDIR)"
	@echo "RSYNC                UNIX command rsync                           $(RSYNC)"
	@echo "SDLLOCAL             Enable use of a 'local' SDL library (0, 1)   $(SDLLOCAL)"
	@echo "SDLLOCAL_INC         Base include directory of the 'local' SDL    $(SDLLOCAL_INC)"
	@echo "SDLLOCAL_LIB         Base library directory of the 'local' SDL    $(SDLLOCAL_LIB)"
	@echo "SED                  UNIX command sed                             $(SED)"
	@echo "SYSCONFDIR           System configuration directory               $(SYSCONFDIR)"
	@echo "TAR                  UNIX command tar                             $(TAR)"
	@echo "TR                   UNIX command tr                              $(TR)"
	@echo "TIME                 UNIX command time                            $(TIME)"
	@echo "VARIANT_LAUNCHER     Enable the QMC2 variant launcher (0, 1)      $(VARIANT_LAUNCHER)"
	@echo "WC_COMPRESSION       Compress MAWS web-cache data (0, 1)          $(WC_COMPRESSION)"
	@echo "WIP                  Enable unfinished code (0, 1)                $(WIP)"
	@echo "XWININFO             X11 xwininfo command                         $(XWININFO)"

# process translations
QMC2_TRANSLATIONS = us de pl fr pt
QT_TRANSLATIONS = de pl fr pt
LBINARIES = $(addsuffix .qm, $(addprefix data/lng/qmc2_, $(QMC2_TRANSLATIONS))) $(addsuffix .qm, $(addprefix data/lng/qt_, $(QT_TRANSLATIONS)))
LREL = $(LRELEASE) $<
ifeq '$(PRETTY)' '1'
LREL = @echo [LREL] $< && $(LRELEASE) $< > /dev/null
endif

lang: $(LBINARIES)

# QMC2 translations

data/lng/qmc2_us.qm: data/lng/qmc2_us.ts
	$(LREL)

data/lng/qmc2_de.qm: data/lng/qmc2_de.ts
	$(LREL)

data/lng/qmc2_pl.qm: data/lng/qmc2_pl.ts
	$(LREL)

data/lng/qmc2_fr.qm: data/lng/qmc2_fr.ts
	$(LREL)

data/lng/qmc2_pt.qm: data/lng/qmc2_pt.ts
	$(LREL)

# Qt translations

#data/lng/qt_us.qm: data/lng/qt_us.ts
#	$(LREL)

data/lng/qt_de.qm: data/lng/qt_de.ts
	$(LREL)

data/lng/qt_pl.qm: data/lng/qt_pl.ts
	$(LREL)

data/lng/qt_fr.qm: data/lng/qt_fr.ts
	$(LREL)

data/lng/qt_pt.qm: data/lng/qt_pt.ts
	$(LREL)

# end of file
