greaterThan(QT_MAJOR_VERSION, 3) {
	greaterThan(QT_MINOR_VERSION, 6) {
		# general project settings
		isEmpty(TARGET):TARGET = qmc2
		CONFIG += qtestlib
		QT += xml webkit network
		TEMPLATE = app
		INCLUDEPATH += minizip/
		FORMS += qmc2main.ui \
			options.ui \
			docbrowser.ui \
			about.ui \
			welcome.ui \
			imgcheck.ui \
			sampcheck.ui \
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
			arcade/arcadesetupdialog.ui \
			htmleditor/htmleditor.ui \
			htmleditor/inserthtmldialog.ui \
			htmleditor/tablepropertydialog.ui
		SOURCES += qmc2main.cpp \
			options.cpp \
			docbrowser.cpp \
			about.cpp \
			welcome.cpp \
			imgcheck.cpp \
			sampcheck.cpp \
			keyseqscan.cpp \
			toolexec.cpp \
			itemselect.cpp \
			romalyzer.cpp \
			gamelist.cpp \
			procmgr.cpp \
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
			customidsetup.cpp \
			minizip/ioapi.c \
			minizip/unzip.c \
			minizip/zip.c \
			arcade/arcadeview.cpp \
			arcade/arcadeitem.cpp \
			arcade/arcademenuitem.cpp \
			arcade/arcadescene.cpp \
			arcade/arcademenuscene.cpp \
			arcade/arcadesettings.cpp \
			arcade/arcadescreenshotsaverthread.cpp \
			arcade/arcadesetupdialog.cpp \
			htmleditor/htmleditor.cpp \
			htmleditor/highlighter.cpp
		HEADERS += qmc2main.h \
			options.h \
			docbrowser.h \
			about.h \
			welcome.h \
			imgcheck.h \
			sampcheck.h \
			keyseqscan.h \
			toolexec.h \
			itemselect.h \
			romalyzer.h \
			gamelist.h \
			procmgr.h \
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
			customidsetup.h \
			macros.h \
			minizip/ioapi.h \
			minizip/unzip.h \
			minizip/zip.h \
			arcade/arcadeview.h \
			arcade/arcadeitem.h \
			arcade/arcademenuitem.h \
			arcade/arcadescene.h \
			arcade/arcademenuscene.h \
			arcade/arcadesettings.h \
			arcade/arcadescreenshotsaverthread.h \
			arcade/arcadesetupdialog.h \
			htmleditor/htmleditor.h \
			htmleditor/highlighter.h
		PRECOMPILED_HEADER = qmc2_prefix.h
		TRANSLATIONS += data/lng/qmc2_us.ts \
			data/lng/qmc2_de.ts \
			data/lng/qmc2_pl.ts \
			data/lng/qmc2_fr.ts \
			data/lng/qmc2_pt.ts \
			data/lng/qmc2_it.ts \
			data/lng/qmc2_ro.ts
		RESOURCES += qmc2.qrc
		QMAKE_MAKEFILE = Makefile.qmake

		# QtWidgets is a separate module in Qt 5
		greaterThan(QT_MAJOR_VERSION, 4) {
			QT += widgets
		}

		# platform specific stuff
		macx {
			QMAKESPEC = macx-xcode
			OBJECTIVE_SOURCES += SDLMain_tmpl.m
			HEADERS += SDLMain_tmpl.h
			LIBS += -framework SDL -framework Cocoa -lz
			contains(TARGET, qmc2-sdlmame): ICON = data/img/classic/mame.icns
			contains(TARGET, qmc2-sdlmess): ICON = data/img/classic/mess.icns
			greaterThan(QMC2_MAC_UNIVERSAL, 0) {
				CONFIG += x86 ppc
			}
			QMAKE_INFO_PLIST = arch/Darwin/Info.plist
		} else {
			!win32 {
				LIBS += -lSDL -lz -lX11
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
				LIBS += -lSDL -lSDLmain -lSDL.dll -lz -lpsapi $$quote($$QMC2_LIBS)
				INCLUDEPATH += $$quote($$QMC2_INCLUDEPATH)
				contains(TARGET, qmc2-mame):RC_FILE = qmc2-mame.rc
				contains(TARGET, qmc2-mess):RC_FILE = qmc2-mess.rc
			} else {
				CONFIG += embed_manifest_exe windows
				LIBS += psapi.lib
				contains(TARGET, qmc2-mame):RC_FILE = qmc2-mame.rc
				contains(TARGET, qmc2-mess):RC_FILE = qmc2-mess.rc
			}
		}
	} else {
		error(Qt $$QT_VERSION is insufficient -- Qt 4.7.0+ required)
	}
} else {
	error(Qt $$QT_VERSION is insufficient -- Qt 4.7.0+ required)
}
