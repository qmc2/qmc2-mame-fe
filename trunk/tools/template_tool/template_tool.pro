TARGET       = qmc2-template-tool
VERSION      = 0.1
TEMPLATE     = app
SOURCES     += main.cpp template_tool.cpp
HEADERS     += template_tool.h
FORMS       += template_tool.ui
CONFIG      += qt
QT          += xml
DEFINES     += "VERSION=$${VERSION}"
greaterThan(DEBUG, 0) {
DEFINES     += "DEBUG"
	CONFIG      += warn_on debug
} else {
	CONFIG      += warn_off release
}
isEmpty($$PREFIX) {
	PREFIX       = /usr/local
}
target.path  = $$PREFIX/bin
INSTALLS    += target
