greaterThan(QT_MAJOR_VERSION, 3) {
	greaterThan(QT_MINOR_VERSION, 7)|greaterThan(QT_MAJOR_VERSION, 4) {
		# general project settings
		isEmpty(TARGET):TARGET = qmc2
		QT += core gui xml xmlpatterns webkit network sql svg
		greaterThan(QT_MAJOR_VERSION, 4) {
			QT += testlib widgets webkitwidgets
			contains(DEFINES, "QMC2_MULTIMEDIA=1"): QT += multimedia multimediawidgets
			HEADERS += qftp/qftp.h \
				qftp/qurlinfo.h
			SOURCES += qftp/qftp.cpp \
				qftp/qurlinfo.cpp
			INCLUDEPATH += qftp
		} else {
			CONFIG += qtestlib
		}
		TEMPLATE = app
		INCLUDEPATH += lzma
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
			deviceconfigurator.ui \
			softwarelist.ui \
			direditwidget.ui \
			fileeditwidget.ui \
			floateditwidget.ui \
			comboeditwidget.ui \
			comboboxwidget.ui \
			componentsetup.ui \
			machinelistviewer.ui \
			iconcachesetupdialog.ui \
			miniwebbrowser.ui \
			youtubevideoplayer.ui \
			videoitemwidget.ui \
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
			rankitemwidget.ui \
			checksumscannerlog.ui \
			collectionrebuilder.ui \
			missingdumpsviewer.ui \
			movierecordersetup.ui \
			romstatefilter.ui \
			individualfallbacksettings.ui \
			catverinioptimizer.ui \
			filterconfigurationdialog.ui \
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
			machinelist.cpp \
			machinelistdbmgr.cpp \
			machinelistmodel.cpp \
			machinelistviewer.cpp \
			iconcachedbmgr.cpp \
			iconcachesetupdialog.cpp \
			processmanager.cpp \
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
			deviceconfigurator.cpp \
			softwarelist.cpp \
			softwareimagewidget.cpp \
			softwaresnapshot.cpp \
			direditwidget.cpp \
			fileeditwidget.cpp \
			floateditwidget.cpp \
			comboeditwidget.cpp \
			comboboxwidget.cpp \
			componentsetup.cpp \
			miniwebbrowser.cpp \
			youtubevideoplayer.cpp \
			videoitemwidget.cpp \
			downloaditem.cpp \
			embedder.cpp \
			embedderopt.cpp \
			demomode.cpp \
			audioeffects.cpp \
			checksumdbmgr.cpp \
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
			rankitemwidget.cpp \
			aspectratiolabel.cpp \
			checksumscannerlog.cpp \
			collectionrebuilder.cpp \
			swldbmgr.cpp \
			datinfodbmgr.cpp \
			missingdumpsviewer.cpp \
			movierecordersetup.cpp \
			cryptedbytearray.cpp \
			romstatefilter.cpp \
			customartwork.cpp \
			customsoftwareartwork.cpp \
			individualfallbacksettings.cpp \
			catverinioptimizer.cpp \
			filterconfigurationdialog.cpp \
			rankitemdelegate.cpp \
			htmleditor/htmleditor.cpp \
			htmleditor/highlighter.cpp \
			lzma/7zAlloc.c \
			lzma/7zBuf2.c \
			lzma/7zBuf.c \
			lzma/7zCrc.c \
			lzma/7zCrcOpt.c \
			lzma/7zDec.c \
			lzma/7zFile.c \
			lzma/7zArcIn.c \
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
			machinelist.h \
			machinelistdbmgr.h \
			machinelistmodel.h \
			machinelistviewer.h \
			iconcachedbmgr.h \
			iconcachesetupdialog.h \
			processmanager.h \
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
			deviceconfigurator.h \
			softwarelist.h \
			softwareimagewidget.h \
			softwaresnapshot.h \
			direditwidget.h \
			fileeditwidget.h \
			filesystemmodel.h \
			floateditwidget.h \
			comboeditwidget.h \
			comboboxwidget.h \
			componentsetup.h \
			miniwebbrowser.h \
			youtubevideoplayer.h \
			videoitemwidget.h \
			downloaditem.h \
			embedder.h \
			embedderopt.h \
			demomode.h \
			audioeffects.h \
			checksumdbmgr.h \
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
			rankitemwidget.h \
			aspectratiolabel.h \
			checksumscannerlog.h \
			collectionrebuilder.h \
			swldbmgr.h \
			datinfodbmgr.h \
			missingdumpsviewer.h \
			movierecordersetup.h \
			cryptedbytearray.h \
			romstatefilter.h \
			customartwork.h \
			customsoftwareartwork.h \
			individualfallbacksettings.h \
			catverinioptimizer.h \
			filterconfigurationdialog.h \
			rankitemdelegate.h \
			htmleditor/htmleditor.h \
			htmleditor/highlighter.h \
			arcade/keysequences.h
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
			SOURCES += archivefile.cpp
			HEADERS += archivefile.h
			LIBS += -larchive
		}

		contains(DEFINES, QMC2_BUNDLED_MINIZIP) {
			INCLUDEPATH += minizip
			SOURCES += minizip/ioapi.c \
				minizip/unzip.c \
				minizip/zip.c
		} else {
			CONFIG += link_pkgconfig
			PKGCONFIG += minizip
		}

		contains(DEFINES, QMC2_BUNDLED_ZLIB) {
			INCLUDEPATH += zlib
			SOURCES += zlib/adler32.c \
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
				zlib/zutil.c
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
				OBJECTIVE_SOURCES += SDLMain_tmpl.m
				HEADERS += SDLMain_tmpl.h
				LIBS += -framework SDL -framework Cocoa -F/Library/Frameworks
				INCLUDEPATH += /Library/Frameworks/SDL.framework/Headers
			}
			contains(TARGET, qmc2-sdlmame): ICON = data/img/classic/mame.icns
			contains(TARGET, qmc2-sdlmess): ICON = data/img/classic/mess.icns
			contains(TARGET, qmc2-sdlume): ICON = data/img/classic/ume.icns
			greaterThan(QMC2_MAC_UNIVERSAL, 0) {
				CONFIG += x86 ppc
			}
			QMAKE_INFO_PLIST = arch/Darwin/Info.plist
		} else {
			!win32 {
				LIBS += -lX11
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
				greaterThan(SDL, 1) {
					LIBS += -lSDL2.dll -lSDL2 -lole32 -lpsapi $$QMC2_LIBS
				} else {
					LIBS += -lSDLmain -lSDL.dll -lSDL -lole32 -lpsapi $$QMC2_LIBS
				}
				INCLUDEPATH += $$QMC2_INCLUDEPATH
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
