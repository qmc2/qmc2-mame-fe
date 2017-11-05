#ifndef _SETUPWIZARD_H
#define _SETUPWIZARD_H

#include <QSettings>
#include <QString>
#include <QVariant>

#include "ui_setupwizard.h"
#include "clickablelabel.h"
#include "macros.h"

#define QMC2_SETUPWIZARD_PAGE_ID_WELCOME			0
#define QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE		1
#define QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE		2
#define QMC2_SETUPWIZARD_PAGE_ID_IMPORT_INI_FILES		3
#define QMC2_SETUPWIZARD_PAGE_ID_IMPORTING_INI_FILES		4
#define QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS		5
#define QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE			6

class CustomSettings : public QObject
{
	public:
		explicit CustomSettings(QSettings *cfg, QObject *parent = 0);

	public:
		void loadFrom(QSettings *cfg);
		void saveTo(QSettings *cfg);
		void setValue(const QString &key, const QVariant &value);
		QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
		void remove(const QString &key);
		void clear() { m_settingsHash.clear(); }
		bool contains(const QString &key) { return m_settingsHash.contains(key); }

	private:
		QHash<QString, QVariant> m_settingsHash;
};

class SetupWizard : public QWizard, public Ui::SetupWizard
{
	Q_OBJECT

       	public:
		explicit SetupWizard(QSettings *cfg, QWidget *parent = 0);
		~SetupWizard();

#if defined(QMC2_OS_MAC)
		bool useNativeFileDialogs() { return m_startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", true).toBool(); }
#else
		bool useNativeFileDialogs() { return m_startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", false).toBool(); }
#endif
		bool findIniFiles();

	public slots:
		void accept();
		void init();
		void setupLanguage();
		void setupStyle(const QString &);
		void log(const QString &);
		void probeExecutable();
		void comboBoxExecutableFile_textChanged(const QString &text);
		void labelImportMameIni_clicked() { radioButtonImportMameIni->click(); }
		void labelImportUiIni_clicked() { radioButtonImportUiIni->click(); }
		void labelImportBothInis_clicked() { radioButtonImportBothInis->click(); }
		void labelImportNothing_clicked() { radioButtonImportNothing->click(); }
		void importMameIni();
		void importUiIni();
		void on_toolButtonBrowseExecutableFile_clicked();
		void on_toolButtonBrowseWorkingDirectory_clicked();
		void on_toolButtonBrowseROMPath_clicked();
		void on_toolButtonBrowseSamplePath_clicked();
		void on_toolButtonBrowseHashPath_clicked();
		void on_toolButtonBrowseMameIni_clicked();
		void on_toolButtonBrowseUiIni_clicked();
		void on_comboBoxLanguage_currentIndexChanged(int index);

	protected:
		int nextId() const;
		void initializePage(int id);

	private:
		QSettings *m_startupConfig;
		QString m_emulatorIniPath;
		QString m_frontendIniPath;
		QString m_listfullSha1;
		QString m_emuConfigName;
		QString m_defaultStyle;
		QStringList m_availableLanguages;
		int m_minRequiredMameVersionMinor;
		int m_minRequiredMameVersionMajor;
		int m_totalMachines;
		int m_modificationTime;
		ClickableLabel *m_labelImportMameIni;
		ClickableLabel *m_labelImportUiIni;
		ClickableLabel *m_labelImportBothInis;
		ClickableLabel *m_labelImportNothing;
		CustomSettings *m_customSettings;
};

#endif
