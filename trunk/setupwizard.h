#ifndef _SETUPWIZARD_H
#define _SETUPWIZARD_H

#include <QSettings>
#include <QString>

#include "ui_setupwizard.h"
#include "macros.h"

#define QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE		1
#define QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE		2
#define QMC2_SETUPWIZARD_PAGE_ID_IMPORT_MAME_INI		3
#define QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SEARCH_PATHS		4
#define QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE			5

class SetupWizard : public QWizard, public Ui::SetupWizard
{
	Q_OBJECT

       	public:
		SetupWizard(QSettings *cfg, QWidget *parent = 0);

		QString &findMameIni();
#if defined(QMC2_OS_MAC)
		bool useNativeFileDialogs() { return m_startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", true).toBool(); }
#else
		bool useNativeFileDialogs() { return m_startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", false).toBool(); }
#endif

	public slots:
		void init();
		void probeExecutable();
		void comboBoxExecutableFile_textChanged(const QString &text);
		void on_toolButtonBrowseExecutableFile_clicked();

	protected:
		int nextId() const;
		void initializePage(int id);
		bool validateCurrentPage();

	private:
		QSettings *m_startupConfig;
		QString m_mameIniPath;
		QString m_listfullSha1;
		int m_minRequiredMameVersionMinor;
		int m_minRequiredMameVersionMajor;
		int m_totalMachines;
		int m_modificationTime;
};

#endif
