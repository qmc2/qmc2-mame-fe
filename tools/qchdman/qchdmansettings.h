#ifndef QCHDMANSETTINGS_H
#define QCHDMANSETTINGS_H

#include <QApplication>
#include <QSettings>
#include <QMdiArea>
#include <QStyle>
#include <QSize>
#include <QFont>
#include <QLocale>

#include "macros.h"
#include "../../settings.h"

class QtChdmanGuiSettings : public Settings
{
	Q_OBJECT

public:
	QMap<QString, QLocale::Language> languageMap;

	QtChdmanGuiSettings() : Settings(QSettings::IniFormat, QSettings::UserScope, QCHDMAN_APP_NAME)
	{
		languageMap["de"] = QLocale::German;
		languageMap["es"] = QLocale::Spanish;
		languageMap["fr"] = QLocale::French;
		languageMap["el"] = QLocale::Greek;
		languageMap["it"] = QLocale::Italian;
		languageMap["pl"] = QLocale::Polish;
		languageMap["pt"] = QLocale::Portuguese;
		languageMap["ro"] = QLocale::Romanian;
		languageMap["sv"] = QLocale::Swedish;
		languageMap["us"] = QLocale::English;
	}

	~QtChdmanGuiSettings()
	{
		sync();
	}

	QString languageToString(QLocale::Language lang)
	{
		QString langStr = languageMap.key(lang);
		if ( !langStr.isEmpty() )
			return langStr;
		else
			return "us";
	}

	QLocale::Language languageFromString(QString lang)
	{
		if ( languageMap.contains(lang) )
			return languageMap[lang];
		else
			return QLocale::English;
	}

public slots:
	// General
	void setApplicationVersion(QString version) { setValue("Version", version); }
	QString applicationVersion() { return value("Version", QCHDMAN_APP_VERSION).toString(); }

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
	void setPreferencesEditorFont(QString font) { setValue("Preferences/EditorFont", font); }
	QString preferencesEditorFont() { return value("Preferences/EditorFont", qApp->font().toString()).toString(); }
	void setPreferencesEditorFontSize(int size) { setValue("Preferences/EditorFontSize", size); }
	int preferencesEditorFontSize() { return value("Preferences/EditorFontSize", qApp->font().pointSize()).toInt(); }
	void setPreferencesChdmanBinary(QString path) { setValue("Preferences/ChdmanBinary", path); }
	QString preferencesChdmanBinary() { return value("Preferences/ChdmanBinary", QString()).toString(); }
	void setPreferencesPreferredCHDInputPath(QString path) { setValue("Preferences/PreferredCHDInputPath", path); }
	QString preferencesPreferredCHDInputPath() { return value("Preferences/PreferredCHDInputPath", QString()).toString(); }
	void setPreferencesPreferredCHDOutputPath(QString path) { setValue("Preferences/PreferredCHDOutputPath", path); }
	QString preferencesPreferredCHDOutputPath() { return value("Preferences/PreferredCHDOutputPath", QString()).toString(); }
	void setPreferencesPreferredInputPath(QString path) { setValue("Preferences/PreferredInputPath", path); }
	QString preferencesPreferredInputPath() { return value("Preferences/PreferredInputPath", QString()).toString(); }
	void setPreferencesPreferredOutputPath(QString path) { setValue("Preferences/PreferredOutputPath", path); }
	QString preferencesPreferredOutputPath() { return value("Preferences/PreferredOutputPath", QString()).toString(); }
	void setPreferencesShowHelpTexts(bool enable) { setValue("Preferences/ShowHelpTexts", enable); }
	bool preferencesShowHelpTexts() { return value("Preferences/ShowHelpTexts", false).toBool(); }
	void setPreferencesMaximizeWindows(bool enable) { setValue("Preferences/MaximizeWindows", enable); }
	bool preferencesMaximizeWindows() { return value("Preferences/MaximizeWindows", false).toBool(); }
	void setPreferencesLanguage(QString lang) { setValue("Preferences/Language", lang); }
	QString preferencesLanguage() { return value("Preferences/Language", languageToString(QLocale::system().language())).toString(); }
	void setPreferencesNativeFileDialogs(bool enable) { setValue("Preferences/NativeFileDialogs", enable); }
#if defined(QCHDMAN_OS_MAC)
	bool preferencesNativeFileDialogs() { return value("Preferences/NativeFileDialogs", true).toBool(); }
#else
	bool preferencesNativeFileDialogs() { return value("Preferences/NativeFileDialogs", false).toBool(); }
#endif
	void setPreferencesLogChannelNames(bool enable) { setValue("Preferences/LogChannelNames", enable); }
	bool preferencesLogChannelNames() { return value("Preferences/LogChannelNames", true).toBool(); }

	// MainWindow
	void setMainWindowState(QByteArray state) { setValue("MainWindow/State", state); }
	QByteArray mainWindowState() { return value("MainWindow/State", QByteArray()).toByteArray(); }
	void setMainWindowGeometry(QByteArray geom) { setValue("MainWindow/Geometry", geom); }
	QByteArray mainWindowGeometry() { return value("MainWindow/Geometry", QByteArray()).toByteArray(); }
	void setMainWindowViewMode(int mode) { setValue("MainWindow/ViewMode", mode); }
	int mainWindowViewMode() { return value("MainWindow/ViewMode", QCHDMAN_VIEWMODE_WINDOWED).toInt(); }
	void setMainWindowRecentFiles(QStringList recentFiles) { setValue("MainWindow/RecentFiles", recentFiles); }
	QStringList mainWindowRecentFiles() { return value("MainWindow/RecentFiles", QStringList()).toStringList(); }
	void setMainWindowRecentScripts(QStringList recentScripts) { setValue("MainWindow/RecentScripts", recentScripts); }
	QStringList mainWindowRecentScripts() { return value("MainWindow/RecentScripts", QStringList()).toStringList(); }

	// ScriptWidget
	void setScriptWidgetProjectMonitorHeaderState(QByteArray state) { setValue("ScriptWidget/ProjectMonitorHeaderState", state); }
	QByteArray scriptWidgetProjectMonitorHeaderState() { return value("ScriptWidget/ProjectMonitorHeaderState", QByteArray()).toByteArray(); }
	void setScriptWidgetSplitterState(QByteArray state) { setValue("ScriptWidget/SplitterState", state); }
	QByteArray scriptWidgetSplitterState() { return value("ScriptWidget/SplitterState", QByteArray()).toByteArray(); }
	void setScriptWidgetTabIndex(int index) { setValue("ScriptWidget/TabIndex", index); }
	int scriptWidgetTabIndex() { return value("ScriptWidget/TabIndex", 0).toInt(); }
	void setScriptWidgetLogLimit(int limit) { setValue("ScriptWidget/LogLimit", limit); }
	int scriptWidgetLogLimit() { return value("ScriptWidget/LogLimit", 1000).toInt(); }
};

#endif // SETTINGS_H
