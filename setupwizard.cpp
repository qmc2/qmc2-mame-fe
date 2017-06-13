#include "setupwizard.h"
#include "macros.h"

SetupWizard::SetupWizard(QSettings *cfg, QWidget *parent) :
	QWizard(parent),
	m_startupConfig(cfg)
{
	setupUi(this);
}
