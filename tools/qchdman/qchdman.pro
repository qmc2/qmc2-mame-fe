QT += core gui
CONFIG += warn_on release

TARGET = qchdman
TEMPLATE = app

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
