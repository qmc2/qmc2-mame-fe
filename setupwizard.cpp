#include <QCryptographicHash>
#include <QStyleFactory>
#include <QApplication>
#include <QStringList>
#include <QFileDialog>
#include <QTranslator>
#include <QLineEdit>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include <QFont>
#include <QDir>

#include "setupwizard.h"

// external global variables
extern bool qmc2TemplateCheck;
extern QTranslator *qmc2Translator;
extern QTranslator *qmc2QtTranslator;

CustomSettings::CustomSettings(QSettings *cfg, QObject *parent) :
	QObject(parent)
{
	loadFrom(cfg);
}

void CustomSettings::loadFrom(QSettings *cfg)
{
	clear();
	if ( cfg )
		foreach (QString key, cfg->allKeys())
			m_settingsHash.insert(key, cfg->value(key));
}

void CustomSettings::saveTo(QSettings *cfg)
{
	if ( cfg ) {
		cfg->clear();
		foreach (QString key, m_settingsHash.uniqueKeys())
			cfg->setValue(key, m_settingsHash.value(key));
	}
}

void CustomSettings::setValue(const QString &key, const QVariant &value)
{
	m_settingsHash.insert(key, value);
}

QVariant CustomSettings::value(const QString &key, const QVariant &defaultValue)
{
	if ( m_settingsHash.contains(key) )
		return m_settingsHash.value(key);
	else
		return defaultValue;
}

void CustomSettings::remove(const QString &key)
{
	if ( m_settingsHash.contains(key) )
		m_settingsHash.remove(key);
}

SetupWizard::SetupWizard(QSettings *cfg, QWidget *parent) :
	QWizard(parent),
	m_startupConfig(cfg),
	m_minRequiredMameVersionMinor(0),
	m_minRequiredMameVersionMajor(183),
	m_totalMachines(-1),
	m_modificationTime(-1)
{
	// remember the default style
	m_defaultStyle = QApplication::style()->objectName();

	m_availableLanguages << "de" << "es" << "el" << "fr" << "it" << "pl" << "pt" << "ro" << "sv" << "us";
	m_customSettings = new CustomSettings(m_startupConfig, this);

	setupUi(this);

	// set customized UI font when applicable
	if ( m_customSettings->contains(QMC2_FRONTEND_PREFIX + "GUI/Font") ) {
		QFont f;
		f.fromString(m_customSettings->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
		qApp->setFont(f);
	}

	// setting explicit button texts makes it possible to translate them dynamically (however, some styles ignore this)
	setButtonText(QWizard::NextButton, tr("&Next >"));
	setButtonText(QWizard::BackButton, tr("< &Back"));
	setButtonText(QWizard::CancelButton, tr("&Cancel"));
	setButtonText(QWizard::FinishButton, tr("&Finish"));

#if QMC2_SVN_REV > 0
	labelVersion->setText(QString("QMC2 v%1 (SVN r%2)").arg(XSTR(QMC2_VERSION)).arg(QMC2_SVN_REV));
#else
	labelVersion->setText(QString("QMC2 v%1").arg(XSTR(QMC2_VERSION)));
#endif

	gridLayout3->removeWidget(labelImportMameIni);
	delete labelImportMameIni;
	m_labelImportMameIni = new ClickableLabel(this);
	gridLayout3->addWidget(m_labelImportMameIni, 3, 1);
	connect(m_labelImportMameIni, SIGNAL(clicked()), this, SLOT(labelImportMameIni_clicked()));

	gridLayout3->removeWidget(labelImportUiIni);
	delete labelImportUiIni;
	m_labelImportUiIni = new ClickableLabel(this);
	gridLayout3->addWidget(m_labelImportUiIni, 4, 1);
	connect(m_labelImportUiIni, SIGNAL(clicked()), this, SLOT(labelImportUiIni_clicked()));

	gridLayout3->removeWidget(labelImportBothInis);
	delete labelImportBothInis;
	m_labelImportBothInis = new ClickableLabel(this);
	gridLayout3->addWidget(m_labelImportBothInis, 5, 1);
	connect(m_labelImportBothInis, SIGNAL(clicked()), this, SLOT(labelImportBothInis_clicked()));

	gridLayout3->removeWidget(labelImportNothing);
	delete labelImportNothing;
	m_labelImportNothing = new ClickableLabel(this);
	gridLayout3->addWidget(m_labelImportNothing, 6, 1);
	connect(m_labelImportNothing, SIGNAL(clicked()), this, SLOT(labelImportNothing_clicked()));

	comboBoxLanguage->blockSignals(true);
	comboBoxLanguage->addItems(m_availableLanguages);
	int index = comboBoxLanguage->findText(m_customSettings->value(QMC2_FRONTEND_PREFIX + "GUI/Language", QString()).toString());
	if ( index >= 0 )
		comboBoxLanguage->setCurrentIndex(index);
	comboBoxLanguage->blockSignals(false);

	comboBoxStyle->blockSignals(true);
	comboBoxStyle->addItem(QObject::tr("Default"));
	comboBoxStyle->addItems(QStyleFactory::keys());
	QString myStyle(QObject::tr((const char *)m_customSettings->value(QMC2_FRONTEND_PREFIX + "GUI/Style", "Default").toString().toUtf8()));
	int styleIndex = comboBoxStyle->findText(myStyle, Qt::MatchFixedString);
	if ( styleIndex < 0 )
		styleIndex = 0;
	comboBoxStyle->setCurrentIndex(styleIndex);
	setupStyle(comboBoxStyle->currentText());
	comboBoxStyle->blockSignals(false);

	connect(comboBoxStyle, SIGNAL(activated(const QString &)), this, SLOT(setupStyle(const QString &)));
	connect(comboBoxExecutableFile->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(comboBoxExecutableFile_textChanged(const QString &)));
	QTimer::singleShot(0, this, SLOT(init()));
}

SetupWizard::~SetupWizard()
{
	delete m_customSettings;
}

void SetupWizard::accept()
{
	m_customSettings->saveTo(m_startupConfig);
	m_startupConfig->sync();
	QWizard::accept();
}

void SetupWizard::init()
{
	setupLanguage();
	button(QWizard::NextButton)->setEnabled(false);
	QStringList emuHistory(m_customSettings->value(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory", QStringList()).toStringList());
	emuHistory.sort();
	for (int i = 0; i < emuHistory.count(); i++) {
		QString emuPath(emuHistory.at(i));
		QFileInfo fi(emuPath);
		if ( fi.exists() && fi.isReadable() && fi.isExecutable() && fi.isFile() )
			comboBoxExecutableFile->insertItem(i, emuPath);
	}
	comboBoxExecutableFile->lineEdit()->setText(m_customSettings->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", QString()).toString());
	int index = comboBoxExecutableFile->findText(comboBoxExecutableFile->lineEdit()->text());
	if ( index >= 0 )
		comboBoxExecutableFile->setCurrentIndex(index);
}

void SetupWizard::log(const QString &message)
{
	plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
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
							if ( verMinor >= m_minRequiredMameVersionMinor && verMajor >= m_minRequiredMameVersionMajor ) {
								labelVersionSupportedResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("Yes") + " (" + versionWords.at(1) + ")</b></font>");
							} else
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

void SetupWizard::importMameIni()
{
	button(QWizard::NextButton)->setEnabled(false);
	log(tr("importing emulator settings from %1").arg(m_emulatorIniPath));
	// FIXME
	if ( radioButtonImportBothInis->isChecked() )
		QTimer::singleShot(0, this, SLOT(importUiIni()));
	else
		button(QWizard::NextButton)->setEnabled(true);
}

void SetupWizard::importUiIni()
{
	button(QWizard::NextButton)->setEnabled(false);
	log(tr("importing front-end settings from %1").arg(m_frontendIniPath));
	// FIXME
	button(QWizard::NextButton)->setEnabled(true);
}

int SetupWizard::nextId() const
{
	switch ( currentId() ) {
		case QMC2_SETUPWIZARD_PAGE_ID_WELCOME:
			return QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE;
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
			m_labelImportMameIni->setVisible(!m_emulatorIniPath.isEmpty());
			m_labelImportMameIni->setText("<font size=\"4\">" + tr("Import emulator settings from %1").arg(m_emulatorIniPath) + "</font>");
			radioButtonImportUiIni->setVisible(!m_frontendIniPath.isEmpty());
			m_labelImportUiIni->setVisible(!m_frontendIniPath.isEmpty());
			m_labelImportUiIni->setText("<font size=\"4\">" + tr("Import front-end settings from %1").arg(m_frontendIniPath) + "</font>");
			radioButtonImportBothInis->setVisible(!m_emulatorIniPath.isEmpty() && !m_frontendIniPath.isEmpty());
			m_labelImportBothInis->setVisible(!m_emulatorIniPath.isEmpty() && !m_frontendIniPath.isEmpty());
			m_labelImportBothInis->setText("<font size=\"4\">" + tr("Import both emulator and front-end settings") + "</font>");
			m_labelImportNothing->setText("<font size=\"4\">" + tr("Import nothing") + "</font>");
			radioButtonImportNothing->setChecked(true);
			break;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORTING_INI_FILES:
			plainTextEdit->clear();
			if ( radioButtonImportBothInis->isChecked() || radioButtonImportMameIni->isChecked() )
				QTimer::singleShot(0, this, SLOT(importMameIni()));
			else if ( radioButtonImportUiIni->isChecked() )
				QTimer::singleShot(0, this, SLOT(importUiIni()));
			break;
		case QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS: {
				QString wd(m_customSettings->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString());
				if ( wd.isEmpty() ) // default to emulator executable's path
					wd = QFileInfo(comboBoxExecutableFile->currentText()).absolutePath();
				lineEditWorkingDirectory->setText(wd);
				lineEditROMPath->setText(m_customSettings->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath", QString()).toString());
				lineEditSamplePath->setText(m_customSettings->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath", QString()).toString());
				lineEditHashPath->setText(m_customSettings->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath", QString()).toString());
			}
			break;
		case QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE: {
				if ( comboBoxStyle->currentText() == tr("Default") )
					m_customSettings->remove(QMC2_FRONTEND_PREFIX + "GUI/Style");
				else
					m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "GUI/Style", comboBoxStyle->currentText());
				m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", comboBoxLanguage->currentText());
				m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", comboBoxExecutableFile->currentText());
				QStringList emuHistory;
				for (int i = 0; i < comboBoxExecutableFile->count(); i++)
					emuHistory << comboBoxExecutableFile->itemText(i);
				if ( !emuHistory.contains(comboBoxExecutableFile->currentText()) )
					emuHistory << comboBoxExecutableFile->currentText();
				if ( emuHistory.isEmpty() )
					m_customSettings->remove(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory");
				else {
					emuHistory.sort();
					m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory", emuHistory);
				}
				if ( !lineEditWorkingDirectory->text().isEmpty() )
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", lineEditWorkingDirectory->text());
				else
					m_customSettings->remove(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory");
				if ( !lineEditROMPath->text().isEmpty() )
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath", lineEditROMPath->text());
				else
					m_customSettings->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath");
				if ( !lineEditSamplePath->text().isEmpty() )
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath", lineEditSamplePath->text());
				else
					m_customSettings->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath");
				if ( !lineEditHashPath->text().isEmpty() )
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath", lineEditHashPath->text());
				else
					m_customSettings->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath");
			}
			break;
	}
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

void SetupWizard::on_toolButtonBrowseWorkingDirectory_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose working directory"), lineEditWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditWorkingDirectory->setText(s);
}

void SetupWizard::on_toolButtonBrowseROMPath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose ROM path"), lineEditROMPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditROMPath->setText(s);
}

void SetupWizard::on_toolButtonBrowseSamplePath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose sample path"), lineEditSamplePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditSamplePath->setText(s);
}

void SetupWizard::on_toolButtonBrowseHashPath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose hash path"), lineEditHashPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditHashPath->setText(s);
}

void SetupWizard::on_comboBoxLanguage_currentIndexChanged(int index)
{
	m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", m_availableLanguages.at(index));
	setupLanguage();
	retranslateUi(this);
	setButtonText(QWizard::NextButton, tr("&Next >"));
	setButtonText(QWizard::BackButton, tr("< &Back"));
	setButtonText(QWizard::CancelButton, tr("&Cancel"));
	setButtonText(QWizard::FinishButton, tr("&Finish"));
}

void SetupWizard::setupLanguage()
{
	QString lang(m_customSettings->value(QMC2_FRONTEND_PREFIX + "GUI/Language", QString()).toString());
	if ( lang.isEmpty() || !m_availableLanguages.contains(lang) ) {
		// try to use default system locale - use "us" if a translation is not available for the system locale
		switch ( QLocale::system().language() ) {
			case QLocale::German:
				lang = "de";
				break;
			case QLocale::Spanish:
				lang = "es";
				break;
			case QLocale::French:
				lang = "fr";
				break;
			case QLocale::Greek:
				lang = "el";
				break;
			case QLocale::Italian:
				lang = "it";
				break;
			case QLocale::Polish:
				lang = "pl";
				break;
			case QLocale::Portuguese:
				lang = "pt";
				break;
			case QLocale::Romanian:
				lang = "ro";
				break;
			case QLocale::Swedish:
				lang = "sv";
				break;
			default:
				lang = "us";
				break;
		}
		m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", lang);
	}
	if ( qmc2QtTranslator ) {
		qApp->removeTranslator(qmc2QtTranslator);
		delete qmc2QtTranslator;
	}
	qmc2QtTranslator = new QTranslator(0);
	qmc2QtTranslator->load(QString(":/data/lng/qt_%1.qm").arg(lang));
	qApp->installTranslator(qmc2QtTranslator);
	if ( qmc2Translator ) {
		qApp->removeTranslator(qmc2Translator);
		delete qmc2Translator;
	}
	qmc2Translator = new QTranslator(0);
	qmc2Translator->load(QString(":/data/lng/qmc2_%1.qm").arg(lang));
	qApp->installTranslator(qmc2Translator);

	// we need to "retranslate" the style list due to "Default"
	comboBoxStyle->blockSignals(true);
	int styleIndex = comboBoxStyle->currentIndex();
	comboBoxStyle->clear();
	comboBoxStyle->addItem(QObject::tr("Default"));
	comboBoxStyle->addItems(QStyleFactory::keys());
	comboBoxStyle->setCurrentIndex(styleIndex);
	comboBoxStyle->blockSignals(false);
}

void SetupWizard::setupStyle(const QString &styleName)
{
	if ( styleName == QObject::tr("Default") )
		qApp->setStyle(QStyleFactory::create(m_defaultStyle));
	else
		qApp->setStyle(QStyleFactory::create(styleName));
}
