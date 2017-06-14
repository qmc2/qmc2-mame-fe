#include <QCryptographicHash>
#include <QApplication>
#include <QStringList>
#include <QFileDialog>
#include <QLineEdit>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include <QDir>

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
	m_totalMachines = -1;
	m_modificationTime = -1;
	m_listfullSha1.clear();
	m_emuConfigName.clear();
	QFileInfo fi(comboBoxExecutableFile->currentText());
	if ( fi.isReadable() && fi.isExecutable() ) {
		labelFileIsExecutableResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("Yes") + "</b></font>");
		QStringList emulatorIdentifiers, emulatorConfigNames;
		emulatorIdentifiers << "MAME" << "M.A.M.E." << "HBMAME" << "HB.M.A.M.E." << "MESS" << "M.E.S.S.";
		emulatorConfigNames << "mame" << "mame" << "hbmame" << "hbmame" << "mess" << "mess";
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
						labelIdentifiedAsMameResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("Yes") + " (" + versionWords.first() + ")</b></font>");
						m_emuConfigName = emulatorConfigNames.at(emulatorIdentifiers.indexOf(versionWords.first()));
						QStringList emulatorVersionInfo(versionWords[1].remove('v').split('.'));
						if ( emulatorVersionInfo.count() > 1 ) {
							int verMinor = emulatorVersionInfo.at(0).toInt();
							int verMajor = emulatorVersionInfo.at(1).toInt();
							if ( verMinor >= m_minRequiredMameVersionMinor && verMajor >= m_minRequiredMameVersionMajor )
								labelVersionSupportedResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("Yes") + " (" + versionWords.at(1) + ")</b></font>");
							else
								labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No") + " (" + versionWords.at(1) + ", " + tr("%1.%2+ required").arg(m_minRequiredMameVersionMinor).arg(m_minRequiredMameVersionMajor) + ")</b></font>");
						} else
							labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("can't parse version info") + ")</b></font>");
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
								labelTotalMachinesResult->setText("<font color=\"green\" size=\"4\"><b>" + QString::number(m_totalMachines) + "</b></font>");
								labelBinaryIdentHashResult->setText("<font color=\"green\" size=\"4\"><b>" + m_listfullSha1 + "</b></font>");
							} else {
								labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
								labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
							}
						} else {
							labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
							labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
						}
						m_modificationTime = fi.lastModified().toTime_t();
						labelFileModificationDateResult->setText("<font color=\"green\" size=\"4\"><b>" + fi.lastModified().toString(Qt::SystemLocaleShortDate) + "</b></font>");
						if ( findIniFiles() ) {
							if ( m_emulatorIniPath.isEmpty() )
								labelEmulatorIniFileLocationResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("No ini-file found") + "</b></font>");
							else
								labelEmulatorIniFileLocationResult->setText("<font color=\"green\" size=\"4\"><b>" + m_emulatorIniPath + "</b></font>");
							if ( m_frontendIniPath.isEmpty() )
								labelFrontendIniFileLocationResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("No ini-file found") + "</b></font>");
							else
								labelFrontendIniFileLocationResult->setText("<font color=\"green\" size=\"4\"><b>" + m_frontendIniPath + "</b></font>");
							button(QWizard::NextButton)->setEnabled(true);
						} else {
							labelEmulatorIniFileLocation->setText("<font color=\"red\" size=\"4\"><b>" + tr("Couldn't determine ini-path") + "</b></font>");
							labelFrontendIniFileLocation->setText("<font color=\"red\" size=\"4\"><b>" + tr("Couldn't determine ini-path") + "</b></font>");
						}
					} else {
						labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + " (" + tr("incompatible binary") + ")</b></font>");
						labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					}
				} else {
					labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + " (" + tr("incompatible binary") + ")</b></font>");
					labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				}
			} else {
				labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + " (" + tr("emulator didn't start") + ")</b></font>");
				labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			}
		} else {
			labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + "(" + tr("emulator didn't start") + ")</b></font>");
			labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		}
	} else {
		labelFileIsExecutableResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + "</b></font>");
		labelIdentifiedAsMameResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
	}
}

int SetupWizard::nextId() const
{
	switch ( currentId() ) {
		case QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE:
			return QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE;
		case QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE:
			if ( !m_emulatorIniPath.isEmpty() || !m_frontendIniPath.isEmpty() )
				return QMC2_SETUPWIZARD_PAGE_ID_IMPORT_INI_FILES;
			else
				return QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORT_INI_FILES:
			if ( radioButtonImportNothing->isChecked() )
				return QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS;
			else
				return QMC2_SETUPWIZARD_PAGE_ID_IMPORTING_INI_FILES;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORTING_INI_FILES:
			return QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS;
		case QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS:
			return QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE;
		case QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE:
			return -1;
	}
}

void SetupWizard::initializePage(int id)
{
	switch ( id ) {
		case QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE:
			labelFileIsExecutableResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelIdentifiedAsMameResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelVersionSupportedResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelTotalMachinesResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelBinaryIdentHashResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelFileModificationDateResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelEmulatorIniFileLocationResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelFrontendIniFileLocationResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			QTimer::singleShot(0, this, SLOT(probeExecutable()));
			break;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORT_INI_FILES:
			radioButtonImportMameIni->setVisible(!m_emulatorIniPath.isEmpty());
			labelImportMameIni->setVisible(!m_emulatorIniPath.isEmpty());
			labelImportMameIni->setText("<font size=\"4\">" + tr("Import emulator settings from %1").arg(m_emulatorIniPath) + "</font>");
			radioButtonImportUiIni->setVisible(!m_frontendIniPath.isEmpty());
			labelImportUiIni->setVisible(!m_frontendIniPath.isEmpty());
			labelImportUiIni->setText("<font size=\"4\">" + tr("Import front-end settings from %1").arg(m_frontendIniPath) + "</font>");
			radioButtonImportBothInis->setVisible(!m_emulatorIniPath.isEmpty() && !m_frontendIniPath.isEmpty());
			labelImportBothInis->setVisible(!m_emulatorIniPath.isEmpty() && !m_frontendIniPath.isEmpty());
			labelImportBothInis->setText("<font size=\"4\">" + tr("Import both emulator and front-end settings") + "</font>");
			labelImportNothing->setText("<font size=\"4\">" + tr("Import nothing") + "</font>");
			radioButtonImportNothing->setChecked(true);
			break;
	}
}

bool SetupWizard::validateCurrentPage()
{
	// FIXME
	return true;
}

bool SetupWizard::findIniFiles()
{
	m_emulatorIniPath.clear();
	m_frontendIniPath.clear();
	if ( m_emuConfigName.isEmpty() )
		return false;
	QProcess commandProc;
	bool started = false, commandProcStarted = false;
	int retries = 0;
	commandProc.start(comboBoxExecutableFile->currentText(), QStringList() << "-showconfig");
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
			bool result = true;
			QString iniPath;
			foreach (QString configLine, output.split('\n')) {
				configLine = configLine.trimmed();
				if ( configLine.startsWith("inipath") ) {
					iniPath = configLine.mid(8).trimmed();
					if ( iniPath.isEmpty() )
						result = false;
					break;
				}
			}
			if ( result ) {
				QStringList iniPaths(iniPath.split(';', QString::SkipEmptyParts));
				QFileInfo fiExec(comboBoxExecutableFile->currentText());
				foreach (QString path, iniPaths) {
					if ( path == "." )
						path = fiExec.absolutePath();
					else
						path = path.replace("$HOME", QDir::homePath());
					QFileInfo fi(path + "/" + m_emuConfigName + ".ini");
					if ( fi.exists() && fi.isReadable() ) {
						m_emulatorIniPath = fi.absoluteFilePath();
						break;
					}
				}
				foreach (QString path, iniPaths) {
					if ( path == "." )
						path = fiExec.absolutePath();
					else
						path = path.replace("$HOME", QDir::homePath());
					QFileInfo fi(path + "/ui.ini");
					if ( fi.exists() && fi.isReadable() ) {
						m_frontendIniPath = fi.absoluteFilePath();
						break;
					}
				}
			}
			return result;
		} else
			return false;
	} else
		return false;
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
