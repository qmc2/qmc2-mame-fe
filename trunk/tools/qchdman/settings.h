#ifndef SETTINGS_H
#define SETTINGS_H

#include <QApplication>
#include <QSettings>
#include <QMdiArea>
#include <QStyle>
#include <QSize>
#include <QFont>
#include "macros.h"

class Settings : public QSettings
{
    Q_OBJECT

public:
    Settings() : QSettings(QSettings::IniFormat, QSettings::UserScope, QCHDMAN_APP_NAME) { }
    ~Settings() { sync(); }

public slots:
    // Preferences
    void setPreferencesGuiStyle(QString style) { setValue("Preferences/GuiStyle", style); }
    QString preferencesGuiStyle() { return value("Preferences/GuiStyle", qApp->style()->objectName()).toString(); }
    void setPreferencesAppFont(QString font) { setValue("Preferences/AppFont", font); }
    QString preferencesAppFont() { return value("Preferences/AppFont", qApp->font().toString()).toString(); }
    void setPreferencesAppFontSize(int size) { setValue("Preferences/AppFontSize", size); }
    int preferencesAppFontSize() { return value("Preferences/AppFontSize", qApp->font().pointSize()).toInt(); }
    void setPreferencesLogFont(QString font) { setValue("Preferences/LogFont", font); }
    QString preferencesLogFont() { return value("Preferences/LogFont", qApp->font().toString()).toString(); }
    void setPreferencesLogFontSize(int size) { setValue("Preferences/LogFontSize", size); }
    int preferencesLogFontSize() { return value("Preferences/LogFontSize", qApp->font().pointSize()).toInt(); }
    void setPreferencesChdmanBinary(QString path) { setValue("Preferences/ChdmanBinary", path); }
    QString preferencesChdmanBinary() { return value("Preferences/ChdmanBinary", QString()).toString(); }
    void setPreferencesShowHelpTexts(bool enable) { setValue("Preferences/ShowHelpTexts", enable); }
    bool preferencesShowHelpTexts() { return value("Preferences/ShowHelpTexts", false).toBool(); }
    void setPreferencesMaximizeWindows(bool enable) { setValue("Preferences/MaximizeWindows", enable); }
    bool preferencesMaximizeWindows() { return value("Preferences/MaximizeWindows", false).toBool(); }

    // MainWindow
    void setMainWindowState(QByteArray state) { setValue("MainWindow/State", state); }
    QByteArray mainWindowState() { return value("MainWindow/State", QByteArray()).toByteArray(); }
    void setMainWindowGeometry(QByteArray geom) { setValue("MainWindow/Geometry", geom); };
    QByteArray mainWindowGeometry() { return value("MainWindow/Geometry", QByteArray()).toByteArray(); }
    void setMainWindowViewMode(int mode) { setValue("MainWindow/ViewMode", mode); };
    int mainWindowViewMode() { return value("MainWindow/ViewMode", QCHDMAN_VIEWMODE_WINDOWED).toInt(); }
    void setMainWindowRecentFiles(QStringList recentFiles) { setValue("MainWindow/RecentFiles", recentFiles); }
    QStringList mainWindowRecentFiles() { return value("MainWindow/RecentFiles", QStringList()).toStringList(); }
};

#endif // SETTINGS_H
