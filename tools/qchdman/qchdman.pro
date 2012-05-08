VERSION = 0.1

QT += core gui
TARGET = qchdman
TEMPLATE = app
DEFINES += QCHDMAN_VERSION=$$VERSION

SOURCES += main.cpp\
    mainwindow.cpp \
    projectwindow.cpp \
    projectwidget.cpp \
    preferencesdialog.cpp

HEADERS  += mainwindow.h \
    macros.h \
    projectwindow.h \
    projectwidget.h \
    settings.h \
    preferencesdialog.h

FORMS += mainwindow.ui \
    projectwidget.ui \
    preferencesdialog.ui

RESOURCES += qchdman.qrc
