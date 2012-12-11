VERSION = 0.1

# Add more folders to ship with the application, here
folder_01.source = qml/ToxicWaste
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
# CONFIG += qdeclarative-boostable

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    tweakedqmlappviewer.cpp \
    imageprovider.cpp \
    arcadesettings.cpp \
    gameobject.cpp \
    consolewindow.cpp \
    processmanager.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qml/ToxicWaste/ToxicWaste.qml \
    qml/ToxicWaste/ToxicWaste.js \
    qml/ToxicWaste/BackgroundAnimation.qml

HEADERS += \
    tweakedqmlappviewer.h \
    imageprovider.h \
    arcadesettings.h \
    macros.h \
    gameobject.h \
    consolewindow.h \
    processmanager.h \
    emulatoroption.h

DEFINES += QMC2_ARCADE_VERSION=$$VERSION

RESOURCES += \
    qmc2-arcade.qrc

evil_hack_to_fool_lupdate {
    SOURCES += qml/ToxicWaste/ToxicWaste.qml \
               qml/ToxicWaste/ToxicWaste.js
}

TRANSLATIONS += translations/qmc2-arcade_de.ts \
    translations/qmc2-arcade_es.ts \
    translations/qmc2-arcade_el.ts \
    translations/qmc2-arcade_it.ts \
    translations/qmc2-arcade_fr.ts \
    translations/qmc2-arcade_pl.ts \
    translations/qmc2-arcade_pt.ts \
    translations/qmc2-arcade_ro.ts \
    translations/qmc2-arcade_sv.ts \
    translations/qmc2-arcade_us.ts

win32 {
    RC_FILE = qmc2-arcade.rc
}
