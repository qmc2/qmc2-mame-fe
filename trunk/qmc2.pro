greaterThan(QT_MAJOR_VERSION, 3) {
	greaterThan(QT_MINOR_VERSION, 7)|greaterThan(QT_MAJOR_VERSION, 4) {
		# general project settings
		isEmpty(TARGET):TARGET = qmc2
		greaterThan(QT_MAJOR_VERSION, 4) {
			QT += testlib widgets webkitwidgets
			contains(DEFINES, "QMC2_YOUTUBE_ENABLED"): QT += multimedia multimediawidgets
			HEADERS += qftp/qftp.h \
				qftp/qurlinfo.h
			SOURCES += qftp/qftp.cpp \
				qftp/qurlinfo.cpp
			INCLUDEPATH += qftp
		} else {
			CONFIG += qtestlib
		}
		QT += xml xmlpatterns webkit network sql svg
		TEMPLATE = app
		INCLUDEPATH += minizip zlib lzma
		FORMS += qmc2main.ui \
			options.ui \
			docbrowser.ui \
			about.ui \
			welcome.ui \
			imagechecker.ui \
			samplechecker.ui \
			keyseqscan.ui \
			joyfuncscan.ui \
			toolexec.ui \
			itemselect.ui \
			romalyzer.ui \
			romstatusexport.ui \
			swlistexport.ui \
			messdevcfg.ui \
			softwarelist.ui \
			direditwidget.ui \
			fileeditwidget.ui \
			floateditwidget.ui \
			comboeditwidget.ui \
			detailsetup.ui \
			miniwebbrowser.ui \
			youtubevideoplayer.ui \
			videoitemwidget.ui \
			mawsqdlsetup.ui \
			embedderopt.ui \
			demomode.ui \
			audioeffects.ui \
			customidsetup.ui \
			toolbarcustomizer.ui \
			cookiemanager.ui \
			emuoptactions.ui \
			arcademodesetup.ui \
			paletteeditor.ui \
			colorwidget.ui \
			brusheditor.ui \
			gradientstopactions.ui \
			softwarestatefilter.ui \
			additionalartworksetup.ui \
			imageformatsetup.ui \
			htmleditor/htmleditor.ui \
			htmleditor/inserthtmldialog.ui \
			htmleditor/tablepropertydialog.ui
		SOURCES += qmc2main.cpp \
			options.cpp \
			docbrowser.cpp \
			about.cpp \
			welcome.cpp \
			imagechecker.cpp \
			samplechecker.cpp \
			keyseqscan.cpp \
			toolexec.cpp \
			itemselect.cpp \
			romalyzer.cpp \
			gamelist.cpp \
			procmgr.cpp \
			imagewidget.cpp \
			preview.cpp \
			flyer.cpp \
			cabinet.cpp \
			controller.cpp \
			marquee.cpp \
			title.cpp \
			pcb.cpp \
			emuopt.cpp \
			joystick.cpp \
			joyfuncscan.cpp \
			romstatusexport.cpp \
			swlistexport.cpp \
			messdevcfg.cpp \
			softwarelist.cpp \
			direditwidget.cpp \
			fileeditwidget.cpp \
			floateditwidget.cpp \
			comboeditwidget.cpp \
			detailsetup.cpp \
			miniwebbrowser.cpp \
			youtubevideoplayer.cpp \
			videoitemwidget.cpp \
			downloaditem.cpp \
			mawsqdlsetup.cpp \
			embedder.cpp \
			embedderopt.cpp \
			demomode.cpp \
			audioeffects.cpp \
			romdbmgr.cpp \
			xmldbmgr.cpp \
			userdatadbmgr.cpp \
			customidsetup.cpp \
			toolbarcustomizer.cpp \
			iconlineedit.cpp \
			cookiejar.cpp \
			cookiemanager.cpp \
			emuoptactions.cpp \
			arcademodesetup.cpp \
			paletteeditor.cpp \
			colorwidget.cpp \
			brusheditor.cpp \
			gradientstopactions.cpp \
			softwarestatefilter.cpp \
			additionalartworksetup.cpp \
			imageformatsetup.cpp \
			settings.cpp \
			sevenzipfile.cpp \
			networkaccessmanager.cpp \
			ftpreply.cpp \
			fileiconprovider.cpp \
			minizip/ioapi.c \
			minizip/unzip.c \
			minizip/zip.c \
			htmleditor/htmleditor.cpp \
			htmleditor/highlighter.cpp \
			zlib/adler32.c \
			zlib/compress.c \
			zlib/crc32.c \
			zlib/deflate.c \
			zlib/gzwrite.c \
			zlib/gzclose.c \
			zlib/gzread.c \
			zlib/gzlib.c \
			zlib/infback.c \
			zlib/inflate.c \
			zlib/inffast.c \
			zlib/inftrees.c \
			zlib/trees.c \
			zlib/uncompr.c \
			zlib/zutil.c \
			lzma/7zAlloc.c \
			lzma/7zBuf2.c \
			lzma/7zBuf.c \
			lzma/7zCrc.c \
			lzma/7zCrcOpt.c \
			lzma/7zDec.c \
			lzma/7zFile.c \
			lzma/7zIn.c \
			lzma/7zStream.c \
			lzma/Alloc.c \
			lzma/Bcj2.c \
			lzma/Bra86.c \
			lzma/Bra.c \
			lzma/BraIA64.c \
			lzma/CpuArch.c \
			lzma/Delta.c \
			lzma/LzFind.c \
			lzma/Lzma2Dec.c \
			lzma/Lzma2Enc.c \
			lzma/Lzma86Dec.c \
			lzma/Lzma86Enc.c \
			lzma/LzmaDec.c \
			lzma/LzmaEnc.c \
			lzma/LzmaLib.c \
			lzma/Ppmd7.c \
			lzma/Ppmd7Dec.c \
			lzma/Ppmd7Enc.c \
			lzma/Sha256.c
		HEADERS += qmc2main.h \
			options.h \
			docbrowser.h \
			about.h \
			welcome.h \
			imagechecker.h \
			samplechecker.h \
			keyseqscan.h \
			toolexec.h \
			itemselect.h \
			romalyzer.h \
			gamelist.h \
			procmgr.h \
			imagewidget.h \
			preview.h \
			flyer.h \
			cabinet.h \
			controller.h \
			marquee.h \
			title.h \
			pcb.h \
			emuopt.h \
			joystick.h \
			joyfuncscan.h \
			romstatusexport.h \
			swlistexport.h \
			messdevcfg.h \
			softwarelist.h \
			direditwidget.h \
			fileeditwidget.h \
			filesystemmodel.h \
			floateditwidget.h \
			comboeditwidget.h \
			detailsetup.h \
			miniwebbrowser.h \
			youtubevideoplayer.h \
			videoitemwidget.h \
			downloaditem.h \
			mawsqdlsetup.h \
			embedder.h \
			embedderopt.h \
			demomode.h \
			audioeffects.h \
			romdbmgr.h \
			xmldbmgr.h \
			userdatadbmgr.h \
			customidsetup.h \
			toolbarcustomizer.h \
			iconlineedit.h \
			cookiejar.h \
			cookiemanager.h \
			emuoptactions.h \
			arcademodesetup.h \
			paletteeditor.h \
			colorwidget.h \
			brusheditor.h \
			gradientstopactions.h \
			softwarestatefilter.h \
			additionalartworksetup.h \
			imageformatsetup.h \
			macros.h \
			settings.h \
			sevenzipfile.h \
			networkaccessmanager.h \
			ftpreply.h \
			fileiconprovider.h \
			htmleditor/htmleditor.h \
			htmleditor/highlighter.h \
			arcade/keysequences.h
		PRECOMPILED_HEADER = qmc2_prefix.h
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

		# platform specific stuff
		macx {
			QMAKESPEC = macx-xcode
			OBJECTIVE_SOURCES += SDLMain_tmpl.m
			HEADERS += SDLMain_tmpl.h
			LIBS += -framework SDL -framework Cocoa
			contains(TARGET, qmc2-sdlmame): ICON = data/img/classic/mame.icns
			contains(TARGET, qmc2-sdlmess): ICON = data/img/classic/mess.icns
			contains(TARGET, qmc2-sdlume): ICON = data/img/classic/ume.icns
			greaterThan(QMC2_MAC_UNIVERSAL, 0) {
				CONFIG += x86 ppc
			}
			QMAKE_INFO_PLIST = arch/Darwin/Info.plist
		} else {
			!win32 {
				LIBS += -lSDL -lX11
				lessThan(QT_MAJOR_VERSION, 5) {
					SOURCES += x11_tools.cpp
				}
			}
		}
		win32 {
			SOURCES += windows_tools.cpp
			DEFINES += PSAPI_VERSION=1
			# use VC++ (default) / MinGW
			greaterThan(QMC2_MINGW, 0) {
				CONFIG += windows
				DEFINES += QMC2_MINGW
				QMAKE_LIBS_QT_ENTRY =
				QMAKE_LFLAGS_CONSOLE =
				LIBS += -lSDLmain -lSDL.dll -lSDL -lole32 -lpsapi $$quote($$QMC2_LIBS)
				INCLUDEPATH += $$quote($$QMC2_INCLUDEPATH)
				contains(TARGET, qmc2-mame):RC_FILE = qmc2-mame.rc
				contains(TARGET, qmc2-mess):RC_FILE = qmc2-mess.rc
				contains(TARGET, qmc2-ume):RC_FILE = qmc2-ume.rc
			} else {
				CONFIG += embed_manifest_exe windows
				LIBS += psapi.lib ole32.lib
				contains(TARGET, qmc2-mame):RC_FILE = qmc2-mame.rc
				contains(TARGET, qmc2-mess):RC_FILE = qmc2-mess.rc
				contains(TARGET, qmc2-ume):RC_FILE = qmc2-ume.rc
			}
		}
	} else {
		error(Qt $$QT_VERSION is insufficient -- Qt 4.8.0+ required)
	}
} else {
	error(Qt $$QT_VERSION is insufficient -- Qt 4.8.0+ required)
}
