#include "setupwizard.h"
#include "macros.h"

SetupWizard::SetupWizard(QSettings *cfg, QWidget *parent) :
	QWizard(parent),
	m_startupConfig(cfg)
{
	setupUi(this);
	adjustSize();
}

int SetupWizard::nextId() const
{
	switch ( currentId() ) {
		case QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE:
			return QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE;
		case QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE:
			if ( !m_mameIniPath.isEmpty() )
				return QMC2_SETUPWIZARD_PAGE_ID_IMPORT_MAME_INI;
			else
				return QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SEARCH_PATHS;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORT_MAME_INI:
			return QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SEARCH_PATHS;
		case QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SEARCH_PATHS:
			return QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE;
		case QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE:
			return -1;
	}
}

bool SetupWizard::validateCurrentPage()
{
	switch ( currentId() ) {
		case QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE:
			return !comboBoxExecutableFile->currentText().isEmpty();
		case QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE:
			findMameIni();
			return true;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORT_MAME_INI:
			return true;
		case QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SEARCH_PATHS:
			return true;
		default:
			return true;
	}
}

QString &SetupWizard::findMameIni()
{
	m_mameIniPath.clear();
	// FIXME
	return m_mameIniPath;
}
