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
# produce pretty (silent) compile output
greaterThan(QMC2_PRETTY_COMPILE, 0) { 
	!isEmpty(QMAKE_CXX):QMAKE_CXX = @echo [C++ ] $< && $$QMAKE_CXX
	!isEmpty(QMAKE_CC):QMAKE_CC = @echo [CC\\ \\ ] $< && $$QMAKE_CC
	!isEmpty(QMAKE_LINK):QMAKE_LINK = @echo [LINK] $@ && $$QMAKE_LINK
	!isEmpty(QMAKE_MOC):QMAKE_MOC = @echo [MOC ] `echo $@ | sed -e \'s/moc_//g\' | sed -e \'s/.cpp/.h/g\'` && $$QMAKE_MOC
	!isEmpty(QMAKE_UIC):QMAKE_UIC = @echo [UIC ] $< && $$QMAKE_UIC
	!isEmpty(QMAKE_RCC):QMAKE_RCC = @echo [RCC ] $< && $$QMAKE_RCC
}
