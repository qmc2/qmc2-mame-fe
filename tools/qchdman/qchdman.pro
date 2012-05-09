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

win32 {
    RC_FILE = qchdman.rc
}

DEFINES += QCHDMAN_VERSION=$$VERSION

SOURCES += main.cpp\
    mainwindow.cpp \
    projectwindow.cpp \
    projectwidget.cpp \
    preferencesdialog.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    macros.h \
    projectwindow.h \
    projectwidget.h \
    settings.h \
    preferencesdialog.h \
    aboutdialog.h

FORMS += mainwindow.ui \
    projectwidget.ui \
    preferencesdialog.ui \
    aboutdialog.ui

RESOURCES += qchdman.qrc

TRANSLATIONS += translations/qchdman_de.ts \
    translations/qchdman_es.ts \
    translations/qchdman_it.ts \
    translations/qchdman_fr.ts \
    translations/qchdman_pl.ts \
    translations/qchdman_pt.ts \
    translations/qchdman_ro.ts \
    translations/qchdman_us.ts
