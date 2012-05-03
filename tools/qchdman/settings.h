#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QSize>
#include <QMdiArea>
#include "macros.h"

class Settings : public QSettings
{
    Q_OBJECT

public:
    Settings() : QSettings(QSettings::IniFormat, QSettings::UserScope, QCHDMAN_APP_NAME) { }
    ~Settings() { sync(); }

public slots:
    // Preferences
    void setPreferencesChdmanBinary(QString path) { setValue("Preferences/ChdmanBinary", path); }
    QString preferencesChdmanBinary() { return value("Preferences/ChdmanBinary", QString()).toString(); }

    // MainWindow
    void setMainWindowState(QByteArray state) { setValue("MainWindow/State", state); }
    QByteArray mainWindowState() { return value("MainWindow/State", QByteArray()).toByteArray(); }
    void setMainWindowGeometry(QByteArray geom) { setValue("MainWindow/Geometry", geom); };
    QByteArray mainWindowGeometry() { return value("MainWindow/Geometry", QByteArray()).toByteArray(); }
    void setMainWindowViewMode(int mode) { setValue("MainWindow/ViewMode", mode); };
    int mainWindowViewMode() { return value("MainWindow/ViewMode", QCHDMAN_VIEWMODE_WINDOWED).toInt(); }

    // ProjectWidget
    void setProjectWidgetSplitterSizes(QSize sizes) { setValue("ProjectWidget/SplitterSizes", sizes); }
    QSize projectWidgetSplitterSizes() { return value("ProjectWidget/SplitterSizes", QSize(70, 30)).toSize(); }
};

#endif // SETTINGS_H
