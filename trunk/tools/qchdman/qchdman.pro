VERSION = 0.1

QT += core gui
TARGET = qchdman
TEMPLATE = app

greaterThan(DEBUG, 0) | contains(DEFINES, "QCHDMAN_DEBUG") {
    !contains(DEFINES, "QCHDMAN_DEBUG"): DEFINES += QCHDMAN_DEBUG
    !contains(CONFIG, "warn_on debug"): CONFIG += warn_on debug
} else {
    !contains(DEFINES, "QCHDMAN_RELEASE"): DEFINES += QCHDMAN_RELEASE
    !contains(CONFIG, "warn_off release"): CONFIG += warn_off release
}

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

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
