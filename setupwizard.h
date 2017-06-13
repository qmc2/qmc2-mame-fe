#ifndef _SETUPWIZARD_H
#define _SETUPWIZARD_H

#include <QSettings>

#include "ui_setupwizard.h"

class SetupWizard : public QWizard, public Ui::SetupWizard
{
	Q_OBJECT

       	public:
		SetupWizard(QSettings *cfg, QWidget *parent = 0);

	public slots:

	private:
		QSettings *m_startupConfig;
};

#endif
