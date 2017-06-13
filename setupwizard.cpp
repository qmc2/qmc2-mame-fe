#include <QFileDialog>
#include <QStringList>
#include <QLineEdit>
#include <QFileInfo>
#include <QTimer>

#include "setupwizard.h"

SetupWizard::SetupWizard(QSettings *cfg, QWidget *parent) :
	QWizard(parent),
	m_startupConfig(cfg)
{
	setupUi(this);
	adjustSize();
	connect(comboBoxExecutableFile->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(comboBoxExecutableFile_textChanged(const QString &)));
	QTimer::singleShot(0, this, SLOT(init()));
}

void SetupWizard::init()
{
	button(QWizard::NextButton)->setEnabled(false);
	QStringList emuHistory(m_startupConfig->value(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory", QStringList()).toStringList());
	emuHistory.sort();
	for (int i = 0; i < emuHistory.count(); i++) {
		QString emuPath(emuHistory.at(i));
		QFileInfo fi(emuPath);
		if ( fi.exists() && fi.isReadable() && fi.isExecutable() && fi.isFile() )
			comboBoxExecutableFile->insertItem(i, emuPath);
	}
	comboBoxExecutableFile->lineEdit()->setText(m_startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", QString()).toString());
	int index = comboBoxExecutableFile->findText(comboBoxExecutableFile->lineEdit()->text());
	if ( index >= 0 )
		comboBoxExecutableFile->setCurrentIndex(index);
}

void SetupWizard::probeExecutable()
{
	button(QWizard::NextButton)->setEnabled(false);
	QFileInfo fi(comboBoxExecutableFile->currentText());
	if ( fi.isReadable() && fi.isExecutable() )
		labelFileIsExecutableResult->setText("<font color=\"green\" size=\"+1\"><b>" + tr("Yes!") + "</b></font>");
	else
		labelFileIsExecutableResult->setText("<font color=\"red\" size=\"+1\"><b>" + tr("No!") + "</b></font>");
	// FIXME
	button(QWizard::NextButton)->setEnabled(true);
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

void SetupWizard::initializePage(int id)
{
	switch ( id ) {
		case QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE:
			labelFileIsExecutableResult->setText("<font size=\"+1\">" + tr("Check result pending...") + "</font>");
			labelIdentifiedAsMameResult->setText("<font size=\"+1\">" + tr("Check result pending...") + "</font>");
			labelVersionSupportedResult->setText("<font size=\"+1\">" + tr("Check result pending...") + "</font>");
			labelTotalMachinesResult->setText("<font size=\"+1\">" + tr("Check result pending...") + "</font>");
			labelBinaryIdentHashResult->setText("<font size=\"+1\">" + tr("Check result pending...") + "</font>");
			labelFileModificationDateResult->setText("<font size=\"+1\">" + tr("Check result pending...") + "</font>");
			QTimer::singleShot(0, this, SLOT(probeExecutable()));
			break;
	}
}

bool SetupWizard::validateCurrentPage()
{
	switch ( currentId() ) {
		case QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE:
			findMameIni();
			break;
	}
	return true;
}

QString &SetupWizard::findMameIni()
{
	m_mameIniPath.clear();
	// FIXME
	return m_mameIniPath;
}

void SetupWizard::comboBoxExecutableFile_textChanged(const QString &text)
{
	QFileInfo fi(text);
	button(QWizard::NextButton)->setEnabled(fi.exists() && fi.isFile());
}

void SetupWizard::on_toolButtonBrowseExecutableFile_clicked()
{
	QString s;
	if ( comboBoxExecutableFile->lineEdit()->text().isEmpty() )
		s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), QString(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	else {
		QFileInfo fileInfo(comboBoxExecutableFile->lineEdit()->text());
		s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), fileInfo.absoluteFilePath(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	}
	if ( !s.isNull() )
		comboBoxExecutableFile->lineEdit()->setText(s);
}
