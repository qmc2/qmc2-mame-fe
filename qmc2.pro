greaterThan(QT_MAJOR_VERSION, 3) {
	greaterThan(QT_MINOR_VERSION, 4) {
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
			messdevcfg.ui \
			messswlist.ui \
			direditwidget.ui \
			fileeditwidget.ui \
			detailsetup.ui \
			miniwebbrowser.ui \
			mawsqdlsetup.ui \
			embedderopt.ui \
			demomode.ui \
			audioeffects.ui \
			arcade/arcadesetupdialog.ui
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
			messdevcfg.cpp \
			messswlist.cpp \
			direditwidget.cpp \
			fileeditwidget.cpp \
			detailsetup.cpp \
			miniwebbrowser.cpp \
			downloaditem.cpp \
			mawsqdlsetup.cpp \
			embedder.cpp \
			embedderopt.cpp \
			demomode.cpp \
			audioeffects.cpp \
			romdbmgr.cpp \
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
			arcade/arcadesetupdialog.cpp
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
			messdevcfg.h \
			messswlist.h \
			direditwidget.h \
			fileeditwidget.h \
			detailsetup.h \
			miniwebbrowser.h \
			downloaditem.h \
			mawsqdlsetup.h \
			embedder.h \
			embedderopt.h \
			demomode.h \
			audioeffects.h \
			romdbmgr.h \
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
			arcade/arcadesetupdialog.h
		PRECOMPILED_HEADER = qmc2_prefix.h
		TRANSLATIONS += data/lng/qmc2_us.ts \
			data/lng/qmc2_de.ts \
			data/lng/qmc2_pl.ts \
			data/lng/qmc2_fr.ts \
			data/lng/qmc2_gr.ts \
			data/lng/qmc2_pt.ts
		RESOURCES += qmc2.qrc
		QMAKE_MAKEFILE = Makefile.qmake

		# produce pretty (silent) compile output
		greaterThan(QMC2_PRETTY_COMPILE, 0) { 
			!isEmpty(QMAKE_CXX):QMAKE_CXX = @echo [C++ ] $< && $$QMAKE_CXX
			!isEmpty(QMAKE_CC):QMAKE_CC = @echo [CC\\ \\ ] $< && $$QMAKE_CC
			!isEmpty(QMAKE_LINK):QMAKE_LINK = @echo [LINK] $@ && $$QMAKE_LINK
			!isEmpty(QMAKE_MOC):QMAKE_MOC = @echo [MOC ] `echo $@ | sed -e \'s/moc_//g\' | sed -e \'s/.cpp/.h/g\'` && $$QMAKE_MOC
			!isEmpty(QMAKE_UIC):QMAKE_UIC = @echo [UIC ] $< && $$QMAKE_UIC
			!isEmpty(QMAKE_RCC):QMAKE_RCC = @echo [RCC ] $< && $$QMAKE_RCC
		}

		# platform specific stuff
		macx {
			OBJECTIVE_SOURCES += SDLMain_tmpl.m
			HEADERS += SDLMain_tmpl.h
			LIBS += -framework SDL -framework Cocoa -lz
			greaterThan(QMC2_MAC_UNIVERSAL, 0) {
				CONFIG += x86 ppc
			}
			QMAKE_INFO_PLIST = macx/Info.plist
		} else {
			!win32 {
				LIBS += -lSDL -lz
			}
		}
		win32 {
			CONFIG += embed_manifest_exe windows
			contains(TARGET, qmc2-mame):RC_FILE = qmc2-mame.rc
			contains(TARGET, qmc2-mess):RC_FILE = qmc2-mess.rc
		}
	} else {
		error(Qt $$QT_VERSION is insufficient -- Qt 4.5.0+ required)
	}
} else {
	error(Qt $$QT_VERSION is insufficient -- Qt 4.5.0+ required)
}
