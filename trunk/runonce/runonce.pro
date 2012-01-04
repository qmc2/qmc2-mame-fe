TEMPLATE = app
SOURCES += runonce.c
CONFIG += qt warn_off release
LIBS += -lXmu -lX11
macx {
	LIBS += -L/usr/X11R6/lib
	CONFIG -= app_bundle
	greaterThan(QMC2_MAC_UNIVERSAL, 0) {
		CONFIG += x86 ppc
	}
}
QMAKE_MAKEFILE = Makefile.qmake
