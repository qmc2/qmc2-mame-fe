#ifndef _SETUPWIZARD_H
#define _SETUPWIZARD_H

#include <QSettings>
#include <QString>

#include "ui_setupwizard.h"

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

	public slots:

	protected:
		int nextId() const;
		bool validateCurrentPage();

	private:
		QSettings *m_startupConfig;
		QString m_mameIniPath;
};

#endif
