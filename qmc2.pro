defineTest(haveMinimumQtVersion) {
	lessThan(QT_MAJOR_VERSION, $$1): return(false)
	count(ARGS, 1, greaterThan) {
		lessThan(QT_MINOR_VERSION, $$2): return(false)
		count(ARGS, 2, greaterThan) {
			lessThan(QT_PATCH_VERSION, $$3): return(false)
		}
	}
	return(true)
}

haveMinimumQtVersion(5, 4, 0) {
	# general project settings
	isEmpty(TARGET):TARGET = qmc2
	QT += core gui widgets xml xmlpatterns network sql svg testlib webkitwidgets
	win32 {
		QT += winextras
	}
	contains(DEFINES, "QMC2_MULTIMEDIA=1"): QT += multimedia multimediawidgets
	HEADERS += src/qftp/qftp.h \
		src/qftp/qurlinfo.h
	SOURCES += src/qftp/qftp.cpp \
		src/qftp/qurlinfo.cpp
	INCLUDEPATH += src/ \
		src/qftp \
		src/lzma
	TEMPLATE = app
	FORMS += ui/qmc2main.ui \
		ui/options.ui \
		ui/docbrowser.ui \
		ui/about.ui \
		ui/welcome.ui \
		ui/imagechecker.ui \
		ui/keyseqscan.ui \
		ui/joyfuncscan.ui \
		ui/toolexec.ui \
		ui/itemselect.ui \
		ui/romalyzer.ui \
		ui/romstatusexport.ui \
		ui/swlistexport.ui \
		ui/deviceconfigurator.ui \
		ui/softwarelist.ui \
		ui/direditwidget.ui \
		ui/fileeditwidget.ui \
		ui/floateditwidget.ui \
		ui/comboeditwidget.ui \
		ui/comboboxwidget.ui \
		ui/componentsetup.ui \
		ui/machinelistviewer.ui \
		ui/iconcachesetupdialog.ui \
		ui/miniwebbrowser.ui \
		ui/youtubevideoplayer.ui \
		ui/videoitemwidget.ui \
		ui/embedderopt.ui \
		ui/demomode.ui \
		ui/audioeffects.ui \
		ui/customidsetup.ui \
		ui/toolbarcustomizer.ui \
		ui/cookiemanager.ui \
		ui/emuoptactions.ui \
		ui/arcademodesetup.ui \
		ui/paletteeditor.ui \
		ui/colorwidget.ui \
		ui/brusheditor.ui \
		ui/gradientstopactions.ui \
		ui/softwarestatefilter.ui \
		ui/additionalartworksetup.ui \
		ui/imageformatsetup.ui \
		ui/rankitemwidget.ui \
		ui/checksumscannerlog.ui \
		ui/collectionrebuilder.ui \
		ui/missingdumpsviewer.ui \
		ui/movierecordersetup.ui \
		ui/romstatefilter.ui \
		ui/individualfallbacksettings.ui \
		ui/catverinioptimizer.ui \
		ui/filterconfigurationdialog.ui \
		ui/visiblecolumnsetup.ui \
		ui/manualscanner.ui \
		ui/rompathcleaner.ui \
		ui/setupwizard.ui \
		src/htmleditor/htmleditor.ui \
		src/htmleditor/inserthtmldialog.ui \
		src/htmleditor/tablepropertydialog.ui
	SOURCES += src/qmc2main.cpp \
		src/options.cpp \
		src/docbrowser.cpp \
		src/about.cpp \
		src/welcome.cpp \
		src/imagechecker.cpp \
		src/keyseqscan.cpp \
		src/toolexec.cpp \
		src/itemselect.cpp \
		src/romalyzer.cpp \
		src/machinelist.cpp \
		src/machinelistdbmgr.cpp \
		src/machinelistmodel.cpp \
		src/machinelistviewer.cpp \
		src/iconcachedbmgr.cpp \
		src/iconcachesetupdialog.cpp \
		src/processmanager.cpp \
		src/imagewidget.cpp \
		src/preview.cpp \
		src/flyer.cpp \
		src/cabinet.cpp \
		src/controller.cpp \
		src/marquee.cpp \
		src/title.cpp \
		src/pcb.cpp \
		src/emuopt.cpp \
		src/joystick.cpp \
		src/joyfuncscan.cpp \
		src/romstatusexport.cpp \
		src/swlistexport.cpp \
		src/deviceconfigurator.cpp \
		src/softwarelist.cpp \
		src/softwareimagewidget.cpp \
		src/softwaresnapshot.cpp \
		src/direditwidget.cpp \
		src/fileeditwidget.cpp \
		src/floateditwidget.cpp \
		src/comboeditwidget.cpp \
		src/comboboxwidget.cpp \
		src/componentsetup.cpp \
		src/miniwebbrowser.cpp \
		src/youtubevideoplayer.cpp \
		src/videoitemwidget.cpp \
		src/downloaditem.cpp \
		src/embedder.cpp \
		src/embedderopt.cpp \
		src/demomode.cpp \
		src/audioeffects.cpp \
		src/checksumdbmgr.cpp \
		src/xmldbmgr.cpp \
		src/userdatadbmgr.cpp \
		src/customidsetup.cpp \
		src/toolbarcustomizer.cpp \
		src/iconlineedit.cpp \
		src/cookiejar.cpp \
		src/cookiemanager.cpp \
		src/emuoptactions.cpp \
		src/arcademodesetup.cpp \
		src/paletteeditor.cpp \
		src/colorwidget.cpp \
		src/brusheditor.cpp \
		src/gradientstopactions.cpp \
		src/softwarestatefilter.cpp \
		src/additionalartworksetup.cpp \
		src/imageformatsetup.cpp \
		src/settings.cpp \
		src/sevenzipfile.cpp \
		src/networkaccessmanager.cpp \
		src/ftpreply.cpp \
		src/fileiconprovider.cpp \
		src/rankitemwidget.cpp \
		src/rankitemdelegate.cpp \
		src/aspectratiolabel.cpp \
		src/checksumscannerlog.cpp \
		src/collectionrebuilder.cpp \
		src/swldbmgr.cpp \
		src/datinfodbmgr.cpp \
		src/missingdumpsviewer.cpp \
		src/movierecordersetup.cpp \
		src/cryptedbytearray.cpp \
		src/bigbytearray.cpp \
		src/romstatefilter.cpp \
		src/customartwork.cpp \
		src/customsoftwareartwork.cpp \
		src/individualfallbacksettings.cpp \
		src/catverinioptimizer.cpp \
		src/filterconfigurationdialog.cpp \
		src/visiblecolumnsetup.cpp \
		src/manualscanner.cpp \
		src/rompathcleaner.cpp \
		src/setupwizard.cpp \
		src/clickablelabel.cpp \
		src/htmleditor/htmleditor.cpp \
		src/htmleditor/highlighter.cpp \
		src/lzma/7zAlloc.c \
		src/lzma/7zBuf2.c \
		src/lzma/7zBuf.c \
		src/lzma/7zCrc.c \
		src/lzma/7zCrcOpt.c \
		src/lzma/7zDec.c \
		src/lzma/7zFile.c \
		src/lzma/7zArcIn.c \
		src/lzma/7zStream.c \
		src/lzma/Alloc.c \
		src/lzma/Bcj2.c \
		src/lzma/Bra86.c \
		src/lzma/Bra.c \
		src/lzma/BraIA64.c \
		src/lzma/CpuArch.c \
		src/lzma/Delta.c \
		src/lzma/LzFind.c \
		src/lzma/Lzma2Dec.c \
		src/lzma/Lzma2Enc.c \
		src/lzma/Lzma86Dec.c \
		src/lzma/Lzma86Enc.c \
		src/lzma/LzmaDec.c \
		src/lzma/LzmaEnc.c \
		src/lzma/LzmaLib.c \
		src/lzma/Ppmd7.c \
		src/lzma/Ppmd7Dec.c \
		src/lzma/Ppmd7Enc.c \
		src/lzma/Sha256.c
	HEADERS += src/qmc2main.h \
		src/options.h \
		src/docbrowser.h \
		src/about.h \
		src/welcome.h \
		src/imagechecker.h \
		src/keyseqscan.h \
		src/toolexec.h \
		src/itemselect.h \
		src/romalyzer.h \
		src/machinelist.h \
		src/machinelistdbmgr.h \
		src/machinelistmodel.h \
		src/machinelistviewer.h \
		src/iconcachedbmgr.h \
		src/iconcachesetupdialog.h \
		src/processmanager.h \
		src/imagewidget.h \
		src/preview.h \
		src/flyer.h \
		src/cabinet.h \
		src/controller.h \
		src/marquee.h \
		src/title.h \
		src/pcb.h \
		src/emuopt.h \
		src/joystick.h \
		src/joyfuncscan.h \
		src/romstatusexport.h \
		src/swlistexport.h \
		src/deviceconfigurator.h \
		src/softwarelist.h \
		src/softwareimagewidget.h \
		src/softwaresnapshot.h \
		src/direditwidget.h \
		src/fileeditwidget.h \
		src/filesystemmodel.h \
		src/floateditwidget.h \
		src/comboeditwidget.h \
		src/comboboxwidget.h \
		src/componentsetup.h \
		src/miniwebbrowser.h \
		src/youtubevideoplayer.h \
		src/videoitemwidget.h \
		src/downloaditem.h \
		src/embedder.h \
		src/embedderopt.h \
		src/demomode.h \
		src/audioeffects.h \
		src/checksumdbmgr.h \
		src/xmldbmgr.h \
		src/userdatadbmgr.h \
		src/customidsetup.h \
		src/toolbarcustomizer.h \
		src/iconlineedit.h \
		src/cookiejar.h \
		src/cookiemanager.h \
		src/emuoptactions.h \
		src/arcademodesetup.h \
		src/paletteeditor.h \
		src/colorwidget.h \
		src/brusheditor.h \
		src/gradientstopactions.h \
		src/softwarestatefilter.h \
		src/additionalartworksetup.h \
		src/imageformatsetup.h \
		src/macros.h \
		src/settings.h \
		src/sevenzipfile.h \
		src/networkaccessmanager.h \
		src/ftpreply.h \
		src/fileiconprovider.h \
		src/rankitemwidget.h \
		src/rankitemdelegate.h \
		src/aspectratiolabel.h \
		src/checksumscannerlog.h \
		src/collectionrebuilder.h \
		src/swldbmgr.h \
		src/datinfodbmgr.h \
		src/missingdumpsviewer.h \
		src/movierecordersetup.h \
		src/cryptedbytearray.h \
		src/bigbytearray.h \
		src/romstatefilter.h \
		src/customartwork.h \
		src/customsoftwareartwork.h \
		src/individualfallbacksettings.h \
		src/catverinioptimizer.h \
		src/filterconfigurationdialog.h \
		src/visiblecolumnsetup.h \
		src/manualscanner.h \
		src/dbcolumninfo.h \
		src/rompathcleaner.h \
		src/setupwizard.h \
		src/clickablelabel.h \
		src/htmleditor/htmleditor.h \
		src/htmleditor/highlighter.h \
		src/arcade/keysequences.h
	TRANSLATIONS += data/lng/qmc2_de.ts \
		data/lng/qmc2_el.ts \
		data/lng/qmc2_es.ts \
		data/lng/qmc2_fr.ts \
		data/lng/qmc2_it.ts \
		data/lng/qmc2_pl.ts \
		data/lng/qmc2_pt.ts \
		data/lng/qmc2_ro.ts \
		data/lng/qmc2_sv.ts \
		data/lng/qmc2_us.ts
	RESOURCES += qmc2.qrc
	QMAKE_MAKEFILE = Makefile.qmake
	DEFINES += _7ZIP_PPMD_SUPPORT _7ZIP_ST

	contains(DEFINES, QMC2_LIBARCHIVE_ENABLED) {
		SOURCES += src/archivefile.cpp
		HEADERS += src/archivefile.h
		LIBS += -larchive
	}

	contains(DEFINES, QMC2_BUNDLED_MINIZIP) {
		INCLUDEPATH += src/minizip
		SOURCES += src/minizip/mz_compat.c \
			src/minizip/mz_crypt.c \
			src/minizip/mz_os.c \
			src/minizip/mz_strm.c \
			src/minizip/mz_strm_mem.c \
			src/minizip/mz_strm_zlib.c \
			src/minizip/mz_zip.c
		!win32 {
			SOURCES += src/minizip/mz_os_posix.c \
				src/minizip/mz_strm_os_posix.c
		} else {
			SOURCES += src/minizip/mz_os_win32.c \
				src/minizip/mz_strm_os_win32.c
		}
		DEFINES += HAVE_ZLIB ZLIB_COMPAT
	} else {
		CONFIG += link_pkgconfig
		PKGCONFIG += minizip
	}

	contains(DEFINES, QMC2_BUNDLED_ZLIB) {
		INCLUDEPATH += src/zlib
		SOURCES += src/zlib/adler32.c \
			src/zlib/compress.c \
			src/zlib/crc32.c \
			src/zlib/deflate.c \
			src/zlib/gzwrite.c \
			src/zlib/gzclose.c \
			src/zlib/gzread.c \
			src/zlib/gzlib.c \
			src/zlib/infback.c \
			src/zlib/inflate.c \
			src/zlib/inffast.c \
			src/zlib/inftrees.c \
			src/zlib/trees.c \
			src/zlib/uncompr.c \
			src/zlib/zutil.c
	} else {
		CONFIG += link_pkgconfig
		PKGCONFIG += zlib
	}

	# platform specific stuff
	macx {
		greaterThan(SDL, 1) {
			LIBS += -framework SDL2 -framework Cocoa -F/Library/Frameworks
			INCLUDEPATH += /Library/Frameworks/SDL2.framework/Headers
		} else {
			OBJECTIVE_SOURCES += src/SDLMain_tmpl.m
			HEADERS += src/SDLMain_tmpl.h
			LIBS += -framework SDL -framework Cocoa -F/Library/Frameworks
			INCLUDEPATH += /Library/Frameworks/SDL.framework/Headers
		}
		ICON = data/img/classic/mame.icns
		# hmmm?
		greaterThan(QMC2_MAC_UNIVERSAL, 0) {
			CONFIG += x86 ppc
		}
		QMAKE_INFO_PLIST = arch/Darwin/Info.plist
	} else {
		!win32 {
			LIBS += -lX11
		}
	}
	win32 {
		SOURCES += src/windows_tools.cpp
		DEFINES += PSAPI_VERSION=1
		# use VC++ (default) / MinGW
		greaterThan(QMC2_MINGW, 0) {
			CONFIG += windows
			DEFINES += QMC2_MINGW
			QMAKE_LIBS_QT_ENTRY =
			QMAKE_LFLAGS_CONSOLE =
			greaterThan(SDL, 1) {
				LIBS += -lSDL2.dll -lSDL2 -lole32 -lpsapi $$QMC2_LIBS
			} else {
				LIBS += -lSDLmain -lSDL.dll -lSDL -lole32 -lpsapi $$QMC2_LIBS
			}
			INCLUDEPATH += $$QMC2_INCLUDEPATH
		} else {
			CONFIG += embed_manifest_exe windows
			LIBS += psapi.lib ole32.lib
		}
		RC_FILE = qmc2-mame.rc
	}
	# Reroute generated files to build sub-dir
	OBJECTS_DIR = build
	MOC_DIR = build
	RCC_DIR = build
	UI_DIR = build
} else {
	error(Qt $$QT_VERSION is insufficient -- Qt 5.4+ required)
}
