# >>> START OF MAKE OPTIONS <<<

# You can either edit the defaults below, specify them on the make command line,
# or pre-define them as environment variables.

# EMULATOR: specifies the target emulator to be used (important: you need to
#           build each target separately; do a "make clean" between the builds)
#           - UNIX and Mac: SDLMAME or SDLMESS
#           - Windows: MAME, MAMEUIFX32 or MESS
ifndef EMULATOR
EMULATOR = SDLMAME
endif

# PREFIX: prefix directory for installation target
ifndef PREFIX
PREFIX = /usr/local
endif

# QUIET: compile and link in "quiet mode" (1) or show all g++ command lines (0)?
ifndef QUIET
QUIET = 0
endif

# CTIME: measure compilation time? 1 means yes, 0 means no.
ifndef CTIME
CTIME = 0
endif

# DEBUG: choose debugging options
#        0 = generate no debugging code at all (default)
#        1 = add symbols for debugger, show compiler warnings
#        2 = add symbols for debugger, show compiler warnings and include QMC2's
#            debugging code (not recommended)
ifndef DEBUG
DEBUG = 0
endif

# ARCH: target system's OS name - this should normally not be changed, only if
#       you want to compile a specific OS's code branch or if the "uname"
#       command doesn't tell the correct OS name of your system (see also OSREL
#       and MACHINE)
ifndef ARCH
ARCH = $(shell uname)
endif

# OSREL: target system's OS-release - please only change this if you know what
#        you are doing :).
ifndef OSREL
OSREL = $(shell uname -r)
endif

# MACHINE: target system's machine type - please only change this if you know
#          what you are doing :).
ifndef MACHINE
MACHINE = $(shell uname -m)
endif

# DATADIR: data directory
ifndef DATADIR
ifeq '$(ARCH)' 'Darwin'
DATADIR = /Library/Application Support
else
DATADIR = $(PREFIX)/share
endif
endif

# SYSCONFDIR: system configuration directory
ifndef SYSCONFDIR
ifeq '$(ARCH)' 'Darwin'
SYSCONFDIR = /Library/Application Support
else
SYSCONFDIR = /etc
endif
endif

# BINDIR: directory containing binaries/application bundles
ifndef BINDIR
ifeq '$(ARCH)' 'Darwin'
BINDIR = /Applications
else
BINDIR = $(PREFIX)/bin
endif
endif

# OSCFG: use global OS build configuration (1, default) or not (0)
ifndef OSCFG
OSCFG = 1
endif

# DISTCFG: use distribution-specific build configuration (1) or just the OS
#          name (0, default)
ifndef DISTCFG
DISTCFG = 0
endif

# IMGSET: select image set (default "classic", see data/img/<image-set-dirs>/
#         for available image sets)
ifndef IMGSET
IMGSET = classic
endif

# JOYSTICK: compile with joystick support (1, default) or without (0)
#           - requires SDL (Simple Directmedia Layer) development library
ifndef JOYSTICK
JOYSTICK = 1
endif

# OPENGL: enable OpenGL features (1) or use standard features only (0, default)
#         - requires OpenGL if enabled (hardware acceleration recommended)
#         - currently only used to enable drawing of images through OpenGL
ifndef OPENGL
OPENGL = 0
endif

# ARCADE_OPENGL: enable (1, default) or disable (0) OpenGL support for arcade
#                mode; if disabled, the pure software renderer will be used
#                - requires OpenGL if enabled (HW acceleration is recommended)
ifndef ARCADE_OPENGL
ARCADE_OPENGL = 1
endif

# WIP: enable (1) or disable (0, default) unfinished code (work in progress)
ifndef WIP
WIP = 0
endif

# PHONON: enable Phonon based features (1, default) or leave them out of the
#         build (0)
#         - requires libphonon and a working backend (such as gstreamer)
#         - currently only used by the built-in audio player
ifndef PHONON
PHONON = 1
endif

# CCACHE: enable (1) or disable (0, default) the use of the ccache compiler
#         cache utility (see also CCACHE_CC and CCACHE_CXX in arch/*.cfg)
ifndef CCACHE
CCACHE = 0
endif

# DISTCC: enable (1) or disable (0, default) the use of a distributed compiler
# (such as distcc or icecc; see also DISTCC_CC and DISTCC_CXX in arch/*.cfg)
ifndef DISTCC
DISTCC = 0
endif

# PRETTY: enable (1, default) or disable (0) pretty compilation output
ifndef PRETTY
PRETTY = 1
endif

# SDLLOCAL: enable (1) or disable (0, default) the use of a "local" SDL library
#           (make sure to also set SDLLOCAL_INC and SDLLOCAL_LIB if you enable
#           this option!)
ifndef SDLLOCAL
SDLLOCAL = 0
endif

# SDLLOCAL_INC: base include directory of the "local" SDL headers
ifndef SDLLOCAL_INC
SDLLOCAL_INC = /usr/local/include
endif

# SDLLOCAL_LIB: base library directory of the "local" SDL library
ifndef SDLLOCAL_LIB
SDLLOCAL_LIB = /usr/local/lib
endif

# QT_TRANSLATION: only used by the install target ("make install"):
#                 if set to "qmc2" (default), the Qt translation files which are
#                 redistributed with QMC2 will be installed and used; if you
#                 would like to use different "qt_<lang>.qm" files, set this to
#                 the absolute path where those translation files are installed
#                 (without the trailing "/") - the install target will then
#                 create symbolic links instead of installing files
ifndef QT_TRANSLATION
QT_TRANSLATION = qmc2
endif

# VARIANT_LAUNCHER: enable (1, default) or disable (0) QMC2 variant launching
ifndef VARIANT_LAUNCHER
VARIANT_LAUNCHER = 1
endif

# AUDIT_WILDCARD: use wildcards on full audit calls (i.e. "mame -listxml '*'");
#                 set to 1 to enable "audit wildcard", 0 (default) disables it
ifndef AUDIT_WILDCARD
AUDIT_WILDCARD = 0
endif

# BROWSER_EXTRAS: enable (1) or disable (0, default) extra browser features such
#                 as QtWebKit's "Web Inspector" (caution: these features may be
#                 buggy!)
ifndef BROWSER_EXTRAS
BROWSER_EXTRAS = 0
endif

# WC_COMPRESSION: enable (1, default) or disable (0) web-cache compression when
#                 storing HTML data to disk (i.e. used by the MAWS web-cache --
#                 has nothing to do with a toilet :)
#                 - this feature doesn't work on Windows yet and will be
#                   disabled automatically on that platform
ifndef WC_COMPRESSION
WC_COMPRESSION = 1
endif

# FADER_SPEED: select the audio fading speed (0: fastest/instantly, >0: slower,
#              default: 500)
ifndef FADER_SPEED
FADER_SPEED = 500
endif

# >>> END OF MAKE OPTIONS <<<

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

# QMAKEV: qmake version (major release)
ifndef QMAKEV
QMAKEV = $(shell echo `$(QMAKE) -v` | $(AWK) '{print $$3}' | $(COLRM) 2)
endif

# version
VERSION_MAJOR = 0
VERSION_MINOR = 2
VERSION_BETA  = 13

# global QMC2 configuration file
GLOBAL_QMC2_INI=$(shell echo $(DESTDIR)/$(SYSCONFDIR)/$(PROJECT)/$(PROJECT).ini | $(SED) -e "s*//*/*g")

# global data directory
GLOBAL_DATADIR=$(shell echo $(DESTDIR)/$(DATADIR) | $(SED) -e "s*//*/*g")

# check Qt version
ifeq '$(QMAKEV)' '2'

# for "make dist" and "make snap" target(s)
PROJECT = qmc2
ifneq '$(VERSION_BETA)' '0'
VERSION     = $(VERSION_MAJOR).$(VERSION_MINOR).b$(VERSION_BETA)
VERSIONDEFS = MAJOR=$(VERSION_MAJOR) MINOR=$(VERSION_MINOR) BETA=$(VERSION_BETA)
else
VERSION     = $(VERSION_MAJOR).$(VERSION_MINOR)
VERSIONDEFS = MAJOR=$(VERSION_MAJOR) MINOR=$(VERSION_MINOR)
endif

# pre-compiler definitions (passed to qmake)
# This needs to work around Qmake's issues with spaces in values
blank =
space = $(blank) $(blank)
DEFINES = DEFINES+=$(VERSIONDEFS) TARGET_OS_NAME=$(OSNAME) TARGET_OS_RELEASE=$(OSREL) TARGET_MACHINE=$(MACHINE) PREFIX=$(PREFIX) DATADIR="$(subst $(space),:,$(DATADIR))" SYSCONFDIR="$(subst $(space),:,$(SYSCONFDIR))" QMC2_JOYSTICK=$(JOYSTICK) QMC2_OPENGL=$(OPENGL) QMC2_ARCADE_OPENGL=$(ARCADE_OPENGL) QMC2_WIP_CODE=$(WIP) QMC2_PHONON=$(PHONON) QMC2_FADER_SPEED=$(FADER_SPEED)

# process make options
ifeq '$(DEBUG)' '2'
DEFINES += QMC2_DEBUG
endif
DEFINES += QMC2_$(EMULATOR)

ifeq '$(VARIANT_LAUNCHER)' '1'
DEFINES += QMC2_VARIANT_LAUNCHER
endif

ifeq '$(AUDIT_WILDCARD)' '1'
DEFINES += QMC2_AUDIT_WILDCARD
endif

ifeq '$(BROWSER_EXTRAS)' '1'
DEFINES += QMC2_BROWSER_EXTRAS_ENABLED
endif

ifeq '$(WC_COMPRESSION)' '1'
DEFINES += QMC2_WC_COMPRESSION_ENABLED
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
GENERICNAME= M.E.S.S. Catalog / Launcher II
else
EMUICO = mame.png
GENERICNAME= M.A.M.E. Catalog / Launcher II
endif

TARGET_NAME = $(PROJECT)-$(shell echo $(EMULATOR) | $(TR) [A-Z] [a-z])
QMAKE_CONF = TARGET=$(TARGET_NAME)
ifeq '$(DEBUG)' '0'
QMAKE_CONF += CONFIG+=warn_off CONFIG+=release
else
QMAKE_CONF += CONFIG+=warn_on CONFIG+=debug
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
$(warning WARNING: you can't mix DISTCC and CCACHE, sorry -- disabling both)
CCACHE=0
DISTCC=0
endif

# targets/rules
all: $(PROJECT)-bin 

bin: $(PROJECT)-bin

$(PROJECT): $(PROJECT)-bin

# Put the version and SCM revision in the Info.plist for OS X
%.plist: %.plist.in
	@$(SED) -e 's/@SHORT_VERSION@/$(subst /,\/,$(VERSION))/g' -e 's/@SCM_REVISION@/$(subst /,\/,$(shell svnversion | $(SED) -e 's/[M:S].*//'))/g' < $< > $@

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
	+@$(MAKESILENT) -f $(QMAKEFILE) > /dev/null && cd runonce && $(QMAKE) -makefile QMC2_PRETTY_COMPILE=$(PRETTY) -o Makefile.qmake runonce.pro > /dev/null && $(MAKESILENT) -f $(QMAKEFILE) > /dev/null
else
	+@$(TIME) ($(MAKESILENT) -f $(QMAKEFILE) > /dev/null && cd runonce && $(QMAKE) -makefile QMC2_PRETTY_COMPILE=$(PRETTY) -o Makefile.qmake runonce.pro > /dev/null && $(MAKESILENT) -f $(QMAKEFILE) > /dev/null)
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
	@$(QMAKE) -makefile VERSION=$(VERSION) VER_MAJ=$(VERSION_MAJOR) VER_MIN=$(VERSION_MINOR) QMC2_PRETTY_COMPILE=$(PRETTY) $(QMAKE_CONF) $(SDL_LIBS) $(QT_CONF) $(QMAKE_CXX_COMPILER) '$(DEFINES)' -o Makefile.qmake $< > /dev/null
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
	+@$(MAKE) -f $(QMAKEFILE) && cd runonce && $(QMAKE) -makefile QMC2_PRETTY_COMPILE=$(PRETTY) -o Makefile.qmake runonce.pro && $(MAKE) -f $(QMAKEFILE)
else
	+@$(TIME) ($(MAKE) -f $(QMAKEFILE) && cd runonce && $(QMAKE) -makefile QMC2_PRETTY_COMPILE=$(PRETTY) -o Makefile.qmake runonce.pro && $(MAKE) -f $(QMAKEFILE))
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
	$(QMAKE) -makefile VERSION=$(VERSION) VER_MAJ=$(VERSION_MAJOR) VER_MIN=$(VERSION_MINOR) QMC2_PRETTY_COMPILE=$(PRETTY) $(QMAKE_CONF) $(SDL_LIBS) $(QT_CONF) $(QMAKE_CXX_COMPILER) '$(DEFINES)' -o Makefile.qmake $<
endif

install: bin
	@echo "Installing QMC2 v$(VERSION)"
	@$(MKDIR) "$(DESTDIR)/$(BINDIR)" "$(DESTDIR)/$(DATADIR)/$(PROJECT)" "$(DESTDIR)/$(SYSCONFDIR)/$(PROJECT)"
ifeq '$(ARCH)' 'Darwin'
	@$(MKDIR) "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
	@$(RSYNC) --exclude '*svn*' "./$(TARGET_NAME).app" "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
	@$(RSYNC) --exclude '*svn*' "./runonce/runonce" "$(DESTDIR)/$(BINDIR)/$(PROJECT)"
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
	@# This shouldn't be necessary, but qmake doesn't add a proper clean target to the project
	@$(RM) $(patsubst %.ui,ui_%.h,$(wildcard *.ui) $(notdir $(wildcard */*.ui)))
	@$(RM) $(wildcard moc_*.cpp) qrc_qmc2.cpp
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
	@# This shouldn't be necessary, but qmake doesn't add a proper clean target to the project
	@$(RM) $(patsubst %.ui,ui_%.h,$(wildcard *.ui) $(notdir $(wildcard */*.ui))) > /dev/null
	@$(RM) $(wildcard moc_*.cpp) qrc_qmc2.cpp > /dev/null
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
	@echo "Usage: $ $(MAKE) [<targets>] [<options>]"
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
	@echo "### Option ###         ### Description ###                         ### Value ###" 
	@echo "ARCADE_OPENGL=<aogl>   Enable use of OpenGL for arcade mode (0, 1) $(ARCADE_OPENGL)"
	@echo "ARCH=<arch-name>       Target system's OS / architecture name      $(ARCH)"
	@echo "AUDIT_WILDCARD=<vl>    Use wildcards on full audit calls (0, 1)    $(AUDIT_WILDCARD)"
	@echo "AWK=<awk>              UNIX command awk                            $(AWK)"
	@echo "BASENAME=<basename>    UNIX command basename                       $(BASENAME)"
	@echo "BINDIR=<bindir>        Binary directory for installation           $(BINDIR)"
	@echo "BROWSER_EXTRAS=<ena>   Enable browser extra features (0, 1)        $(BROWSER_EXTRAS)"
	@echo "CCACHE=<ccache>        Use a compiler cache (0, 1)                 $(CCACHE)"
	@echo "CCACHE_CC=<cc>         Command used for cached cc                  $(CCACHE_CC)"
	@echo "CCACHE_CXX=<cxx>       Command used for cached c++                 $(CCACHE_CXX)"
	@echo "CD=<cd>                UNIX command cd                             $(CD)"
	@echo "COLRM=<colrm>          UNIX command colrm                          $(COLRM)"
	@echo "CP=<cp>                UNIX command cp                             $(CP)"
	@echo "CTIME=<ctime-measure>  Measure compilation & linkage time (0, 1)   $(CTIME)"
	@echo "DATADIR=<data-dir>     Data directory for installation             $(DATADIR)"
	@echo "DATE=<date>            UNIX command date                           $(DATE)"
	@echo "DEBUG=<debug-level>    Choose debugging level (0, 1, 2)            $(DEBUG)"
	@echo "DISTCC=<distcc>        Use a distributed compiler (0, 1)           $(DISTCC)"
	@echo "DISTCC_CC=<cc>         Command used for distributed cc             $(DISTCC_CC)"
	@echo "DISTCC_CXX=<cxx>       Command used for distributed c++            $(DISTCC_CXX)"
	@echo "DISTCFG=<dist-cfg>     Use distribution-specific config (0, 1)     $(DISTCFG)"
	@echo "EMULATOR=<emulator>    Target emulator (SDLMAME, SDLMESS)          $(EMULATOR)"
	@echo "FADER_SPEED=<speed>    Audio fading speed (0: fastest, >0: slower) $(FADER_SPEED)"
	@echo "FIND=<find>            UNIX command find                           $(FIND)"
	@echo "GREP=<grep>            UNIX command grep                           $(GREP)"
	@echo "IMGSET=<imgset>        Image set to be used                        $(IMGSET)"
	@echo "JOYSTICK=<joystick>    Compile with SDL joystick support (0, 1)    $(JOYSTICK)"
	@echo "LN=<ln>                UNIX command ln                             $(LN)"
	@echo "LRELEASE=<lrelease>    Qt language release (lrelease) command      $(LRELEASE)"
	@echo "LUPDATE=<lupdate>      Qt language update (lupdate) command        $(LUPDATE)"
	@echo "MACHINE=<machine>      Target system's machine type                $(MACHINE)"
	@echo "MAKE=<make>            GNU make command                            $(MAKE)"
	@echo "MAKESILENT=<make-s>    GNU make command (silent mode)              $(MAKESILENT)"
	@echo "MKDIR=<mkdir>          UNIX command mkdir                          $(MKDIR)"
	@echo "OPENGL=<opengl>        Enable miscellaneous OpenGL features (0, 1) $(OPENGL)"
	@echo "OSCFG=<os-cfg>         Use global OS configuration (0, 1)          $(OSCFG)"
	@echo "OSNAME=<os-name>       Target system's OS name                     $(OSNAME)"
	@echo "OSREL=<os-release>     Target system's OS release                  $(OSREL)"
	@echo "PHONON=<phonon>        Enable Phonon based features (0, 1)         $(PHONON)"
	@echo "PREFIX=<prefix-dir>    Prefix directory for install target         $(PREFIX)"
	@echo "PRETTY=<pretty>        Use qmake hacks for pretty compile output   $(PRETTY)"
	@echo "QMAKE=<qmake>          Qt make (qmake) command                     $(QMAKE)"
	@echo "QMAKEFILE=<qmake>      Qt generated Makefile name                  $(QMAKEFILE)"
	@echo "QT_TRANSLATION=<trans> Specify path to Qt translations or 'qmc2'   $(QT_TRANSLATION)"
	@echo "QUIET=<quiet-mode>     Suppress output of compile/link (0, 1)      $(QUIET)"
	@echo "RM=<rm>                UNIX command rm                             $(RM)"
	@echo "RMDIR=<rmdir>          UNIX command rmdir                          $(RMDIR)"
	@echo "RSYNC=<rsync>          UNIX command rsync                          $(RSYNC)"
	@echo "SDLLOCAL=<sdllocal>    Enable use of a 'local' SDL library (0, 1)  $(SDLLOCAL)"
	@echo "SDLLOCAL_INC=<sdlinc>  Base include directory of the 'local' SDL   $(SDLLOCAL_INC)"
	@echo "SDLLOCAL_LIB=<sdllib>  Base library directory of the 'local' SDL   $(SDLLOCAL_LIB)"
	@echo "SED=<sed>              UNIX command sed                            $(SED)"
	@echo "SYSCONFDIR=<conf-dir>  System configuration directory              $(SYSCONFDIR)"
	@echo "TAR=<tar>              UNIX command tar                            $(TAR)"
	@echo "TR=<tr>                UNIX command tr                             $(TR)"
	@echo "TIME=<time>            UNIX command time                           $(TIME)"
	@echo "VARIANT_LAUNCHER=<vl>  Enable the QMC2 variant launcher (0, 1)     $(VARIANT_LAUNCHER)"
	@echo "WC_COMPRESSION=<wcc>   Compress MAWS web-cache data (0, 1)         $(WIP)"
	@echo "WIP=<wip>              Enable unfinished code (0, 1)               $(WIP)"

# process translations
QMC2_TRANSLATIONS = us de pl fr gr
QT_TRANSLATIONS = de pl fr
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

data/lng/qmc2_gr.qm: data/lng/qmc2_gr.ts
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

#data/lng/qt_gr.qm: data/lng/qt_gr.ts
#	$(LREL)

# end of file
