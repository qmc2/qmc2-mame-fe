#include <QCryptographicHash>
#include <QApplication>
#include <QStringList>
#include <QFileDialog>
#include <QLineEdit>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QTimer>

#include "setupwizard.h"

extern bool qmc2TemplateCheck;

SetupWizard::SetupWizard(QSettings *cfg, QWidget *parent) :
	QWizard(parent),
	m_startupConfig(cfg),
	m_minRequiredMameVersionMinor(0),
	m_minRequiredMameVersionMajor(183),
	m_totalMachines(-1),
	m_modificationTime(-1)
{
	setupUi(this);
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
	m_totalMachines = -1;
	m_modificationTime = -1;
	m_listfullSha1.clear();
	button(QWizard::NextButton)->setEnabled(false);
	QFileInfo fi(comboBoxExecutableFile->currentText());
	if ( fi.isReadable() && fi.isExecutable() ) {
		labelFileIsExecutableResult->setText("<font color=\"green\" size=\"+1\"><b>" + tr("Yes") + "</b></font>");
		QStringList emulatorIdentifiers;
		emulatorIdentifiers << "MAME" << "M.A.M.E." << "HBMAME" << "HB.M.A.M.E." << "MESS" << "M.E.S.S.";
		QProcess commandProc;
		bool started = false, commandProcStarted = false;
		int retries = 0;
		commandProc.start(comboBoxExecutableFile->currentText(), QStringList() << "-help");
		started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
		while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
			qApp->processEvents();
			started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
		}
		if ( started ) {
			commandProcStarted = true;
			bool commandProcRunning = (commandProc.state() == QProcess::Running);
			while ( commandProcRunning && !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) ) {
				qApp->processEvents();
				commandProcRunning = (commandProc.state() == QProcess::Running);
			}
			if ( commandProcStarted ) {
				QString output(commandProc.readAllStandardOutput());
#if defined(QMC2_OS_WIN)
				output.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
				QStringList versionLines(output.split('\n'));
				QStringList versionWords(versionLines.first().split(' '));
				if ( versionWords.count() > 1 ) {
					if ( emulatorIdentifiers.contains(versionWords.first()) ) {
						labelIdentifiedAsMameResult->setText("<font color=\"green\" size=\"+1\"><b>" + tr("Yes") + " (" + versionWords.first() + ")</b></font>");
						QStringList emulatorVersionInfo(versionWords[1].remove('v').split('.'));
						if ( emulatorVersionInfo.count() > 1 ) {
							int verMinor = emulatorVersionInfo.at(0).toInt();
							int verMajor = emulatorVersionInfo.at(1).toInt();
							if ( verMinor >= m_minRequiredMameVersionMinor && verMajor >= m_minRequiredMameVersionMajor )
								labelVersionSupportedResult->setText("<font color=\"green\" size=\"+1\"><b>" + tr("Yes") + " (" + versionWords.at(1) + ")</b></font>");
							else
								labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No") + " (" + versionWords.at(1) + ", " + tr("%1.%2+ required").arg(m_minRequiredMameVersionMinor).arg(m_minRequiredMameVersionMajor) + ")</b></font>");
						} else
							labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("Unknown") + " (" + tr("can't parse version info") + ")</b></font>");
						commandProcStarted = false;
						retries = 0;
						commandProc.start(comboBoxExecutableFile->currentText(), QStringList() << "-listfull");
						started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
						while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
							qApp->processEvents();
							started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
						}
						if ( started ) {
							commandProcStarted = true;
							commandProcRunning = (commandProc.state() == QProcess::Running);
							while ( commandProcRunning && !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) ) {
								qApp->processEvents();
								commandProcRunning = (commandProc.state() == QProcess::Running);
							}
							if ( commandProcStarted ) {
								QCryptographicHash sha1(QCryptographicHash::Sha1);
								QString lfOutput(commandProc.readAllStandardOutput());
								m_totalMachines = lfOutput.count('\n') - 1;
								sha1.addData(lfOutput.toUtf8().constData());
								m_listfullSha1 = sha1.result().toHex();
								labelTotalMachinesResult->setText("<font color=\"green\" size=\"+1\"><b>" + QString::number(m_totalMachines) + "</b></font>");
								labelBinaryIdentHashResult->setText("<font color=\"green\" size=\"+1\"><b>" + m_listfullSha1 + "</b></font>");
							} else {
								labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
								labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
							}
						} else {
							labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
							labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
						}
						m_modificationTime = fi.lastModified().toTime_t();
						labelFileModificationDateResult->setText("<font color=\"green\" size=\"+1\"><b>" + fi.lastModified().toString(Qt::SystemLocaleShortDate) + "</b></font>");
						button(QWizard::NextButton)->setEnabled(true);
					} else {
						labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"+1\"><b>" + tr("No") + " (" + tr("incompatible binary") + ")</b></font>");
						labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
						labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
						labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
						labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
					}
				} else {
					labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"+1\"><b>" + tr("No") + " (" + tr("incompatible binary") + ")</b></font>");
					labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
					labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
					labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
					labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
				}
			} else {
				labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"+1\"><b>" + tr("No") + " (" + tr("emulator didn't start") + ")</b></font>");
				labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
				labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
				labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
				labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
			}
		} else {
			labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"+1\"><b>" + tr("No") + "(" + tr("emulator didn't start") + ")</b></font>");
			labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
			labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
			labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
			labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
		}
	} else {
		labelFileIsExecutableResult->setText("<font color=\"red\" size=\"+1\"><b>" + tr("No") + "</b></font>");
		labelIdentifiedAsMameResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
		labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
		labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
		labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
		labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"+1\"><b>" + tr("No result") + "</b></font>");
	}
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
