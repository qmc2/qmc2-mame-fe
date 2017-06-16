#ifndef _SETUPWIZARD_H
#define _SETUPWIZARD_H

#include <QSettings>
#include <QString>

#include "ui_setupwizard.h"
#include "clickablelabel.h"
#include "macros.h"

#define QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE		1
#define QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE		2
#define QMC2_SETUPWIZARD_PAGE_ID_IMPORT_INI_FILES		3
#define QMC2_SETUPWIZARD_PAGE_ID_IMPORTING_INI_FILES		4
#define QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS		5
#define QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE			6

class SetupWizard : public QWizard, public Ui::SetupWizard
{
	Q_OBJECT

       	public:
		SetupWizard(QSettings *cfg, QWidget *parent = 0);

#if defined(QMC2_OS_MAC)
		bool useNativeFileDialogs() { return m_startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", true).toBool(); }
#else
		bool useNativeFileDialogs() { return m_startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", false).toBool(); }
#endif
		bool findIniFiles();

	public slots:
		void init();
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

	protected:
		int nextId() const;
		void initializePage(int id);
		bool validateCurrentPage();

	private:
		QSettings *m_startupConfig;
		QString m_emulatorIniPath;
		QString m_frontendIniPath;
		QString m_listfullSha1;
		QString m_emuConfigName;
		int m_minRequiredMameVersionMinor;
		int m_minRequiredMameVersionMajor;
		int m_totalMachines;
		int m_modificationTime;
		ClickableLabel *m_labelImportMameIni;
		ClickableLabel *m_labelImportUiIni;
		ClickableLabel *m_labelImportBothInis;
		ClickableLabel *m_labelImportNothing;
};

#endif
