#include <QTimer>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QStyleFactory>
#include <QTabWidget>
#include <QApplication>

#include "welcome.h"
#include "macros.h"
#include "options.h"
#include "romalyzer.h"
#include "cryptedbytearray.h"
#include "setupwizard.h"

// external global variables
extern QTranslator *qmc2Translator;
extern QTranslator *qmc2QtTranslator;

Welcome::Welcome(QWidget *parent)
	: QDialog(parent)
{
	availableLanguages << "de" << "es" << "el" << "fr" << "it" << "pl" << "pt" << "ro" << "sv" << "us";
	checkOkay = false;
	hide();
	if ( !checkConfig() ) {
		setupUi(this);
#if !defined(QMC2_WIP_ENABLED)
		pushButtonRunSetupWizard->hide();
#endif
		comboBoxLanguage->blockSignals(true);
		comboBoxLanguage->addItems(availableLanguages);
		originalLanguage = startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Language", QString()).toString();
		int index = comboBoxLanguage->findText(originalLanguage);
		if ( index >= 0 )
			comboBoxLanguage->setCurrentIndex(index);
		comboBoxLanguage->blockSignals(false);
		comboBoxStyle->blockSignals(true);
		comboBoxStyle->addItem(QObject::tr("Default"));
		comboBoxStyle->addItems(QStyleFactory::keys());
		QString myStyle(QObject::tr((const char *)startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Style", "Default").toString().toUtf8()));
		int styleIndex = comboBoxStyle->findText(myStyle, Qt::MatchFixedString);
		if ( styleIndex < 0 )
			styleIndex = 0;
		comboBoxStyle->setCurrentIndex(styleIndex);
		comboBoxStyle->blockSignals(false);
		QStringList emuHistory(startupConfig->value(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory", QStringList()).toStringList());
		emuHistory.sort();
		for (int i = 0; i < emuHistory.count(); i++) {
			QString emuPath(emuHistory.at(i));
			QFileInfo fi(emuPath);
			if ( fi.exists() && fi.isReadable() && fi.isExecutable() && fi.isFile() )
				comboBoxExecutableFile->insertItem(i, emuPath);
		}
		comboBoxExecutableFile->lineEdit()->setText(startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", QString()).toString());
		index = comboBoxExecutableFile->findText(comboBoxExecutableFile->lineEdit()->text());
		if ( index >= 0 )
			comboBoxExecutableFile->setCurrentIndex(index);
		lineEditWorkingDirectory->setText(startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString());
		lineEditROMPath->setText(startupConfig->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath", QString()).toString());
		lineEditSamplePath->setText(startupConfig->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath", QString()).toString());
		lineEditHashPath->setText(startupConfig->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath", QString()).toString());
		adjustSize();
		show();
	} else {
		checkOkay = true;
		QTimer::singleShot(0, this, SLOT(on_pushButtonOkay_clicked()));
	}
}

Welcome::~Welcome()
{
	delete startupConfig;
}

void Welcome::on_pushButtonOkay_clicked()
{
	if ( !checkOkay ) {
		QFileInfo fileInfo(comboBoxExecutableFile->lineEdit()->text());
		if ( fileInfo.isExecutable() && fileInfo.isReadable() && fileInfo.isFile() ) {
			startupConfig->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", comboBoxExecutableFile->lineEdit()->text());
			QStringList emuHistory;
			for (int i = 0; i < comboBoxExecutableFile->count(); i++)
				emuHistory << comboBoxExecutableFile->itemText(i);
			if ( !emuHistory.contains(comboBoxExecutableFile->lineEdit()->text()) )
				emuHistory << comboBoxExecutableFile->lineEdit()->text();
			if ( emuHistory.isEmpty() )
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory");
			else {
				emuHistory.sort();
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory", emuHistory);
			}
			if ( !lineEditWorkingDirectory->text().isEmpty() ) {
				QString s(lineEditWorkingDirectory->text());
				if ( !s.endsWith("/") )
					s += "/";
				startupConfig->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", s);
			}
			if ( !lineEditROMPath->text().isEmpty() )
				startupConfig->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath", lineEditROMPath->text());
			if ( !lineEditSamplePath->text().isEmpty() )
				startupConfig->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath", lineEditSamplePath->text());
			if ( !lineEditHashPath->text().isEmpty() )
				startupConfig->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath", lineEditHashPath->text());
			QString styleName(comboBoxStyle->currentText());
			if ( styleName == tr("Default") )
				styleName = "Default";
			startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/Style", styleName);
			startupConfig->sync();
			emit accept();
		} else
			QMessageBox::critical(this, tr("Error"), tr("The specified file isn't executable!"));
	} else
		emit accept();
}

void Welcome::on_pushButtonRunSetupWizard_clicked()
{
	SetupWizard wizard(startupConfig, this);
	hide();
	switch ( wizard.exec() ) {
		case QDialog::Accepted:
			emit accept();
			break;
		case QDialog::Rejected:
		default:
			emit reject();
			break;
	}
}

void Welcome::on_toolButtonBrowseExecutableFile_clicked()
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
	raise();
}

void Welcome::on_toolButtonBrowseWorkingDirectory_clicked()
{
	QString workingDirectory(lineEditWorkingDirectory->text());
	QString executableFile(comboBoxExecutableFile->lineEdit()->text());
	QString suggestion(workingDirectory);
	if ( workingDirectory.isEmpty() && !executableFile.isEmpty() ) {
		QFileInfo fi(executableFile);
		suggestion = fi.dir().absolutePath();
	}
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose working directory"), suggestion, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditWorkingDirectory->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseROMPath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose ROM path"), lineEditROMPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditROMPath->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseSamplePath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose sample path"), lineEditSamplePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditSamplePath->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseHashPath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose hash path"), lineEditHashPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditHashPath->setText(s);
	raise();
}

void Welcome::on_comboBoxLanguage_currentIndexChanged(int index)
{
	startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", availableLanguages[index]);
	setupLanguage();
	retranslateUi(this);
	adjustSize();
}

void Welcome::reject()
{
	if ( !originalLanguage.isEmpty() )
		startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", originalLanguage);
	QDialog::reject();
}

void Welcome::setupLanguage()
{
	QString lang(startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Language", QString()).toString());
	if ( lang.isEmpty() || !availableLanguages.contains(lang) ) {
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
		startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", lang);
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
}

bool Welcome::checkConfig()
{
	bool configOkay = true;

	QCoreApplication::setOrganizationName(QMC2_ORGANIZATION_NAME);
	QCoreApplication::setOrganizationDomain(QMC2_ORGANIZATION_DOMAIN);
	QCoreApplication::setApplicationName(QMC2_VARIANT_NAME);

#if !defined(QMC2_OS_WIN)
	QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, QMC2_SYSCONF_PATH);
#endif
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, Options::configPath());

	startupConfig = new Settings(QSettings::IniFormat, QSettings::UserScope, "qmc2");

#if defined(QMC2_SDLMAME)
	variant = "qmc2-sdlmame";
#elif defined(QMC2_MAME)
	variant = "qmc2-mame";
#endif

	setupLanguage();

	startupConfig->beginGroup(QMC2_FRONTEND_PREFIX);
	if ( startupConfig->value("GUI/CheckSingleInstance", true).toBool() ) {
		if ( startupConfig->value(QString("InstanceRunning")).toBool() ) {
			switch ( QMessageBox::question(0, tr("Single-instance check"),
						       tr("It appears that another instance of %1 is already running.\nHowever, this can also be the leftover of a previous crash.\n\nExit now, accept once or ignore completely?").arg(variant),
						       tr("&Exit"), tr("&Once"), tr("&Ignore"), 0, 0) ) {
				case 0:
					startupConfig->setValue("GUI/CheckSingleInstance", true);
					qApp->quit();
					return false;
					break;
				case 1:
					startupConfig->setValue("GUI/CheckSingleInstance", true);
					break;
				case 2: 
				default:
					startupConfig->setValue("GUI/CheckSingleInstance", false);
					break;
			}
		}
	}
	startupConfig->endGroup();

	QStringList verList(startupConfig->value("Version").toString().split('.', QString::SkipEmptyParts));
	if ( verList.count() > 1 ) {
		int omv = verList.at(1).toInt();
		int osr = startupConfig->value("SVN_Revision").toInt();
		if ( QMC2_TEST_VERSION(omv, 70, osr, 7815) ) {
			// more "Game" => "Machine" transitions
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicator", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator", true).toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicatorOnlyWhenRequired", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired", false).toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/ShowGameName") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowMachineName", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/ShowGameName", true).toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/ShowGameName");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/ShowGameNameOnlyWhenRequired") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowMachineNameOnlyWhenRequired", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/ShowGameNameOnlyWhenRequired", false).toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/ShowGameNameOnlyWhenRequired");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineDetail/TabPosition", startupConfig->value(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::North).toInt());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/SaveMachineSelection", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection", true).toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "GUI/RestoreMachineSelection", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection", true).toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection");
			}
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX + "SelectedGame") ) {
				startupConfig->setValue(QMC2_EMULATOR_PREFIX + "SelectedMachine", startupConfig->value(QMC2_EMULATOR_PREFIX + "SelectedGame", QString()).toString());
				startupConfig->remove(QMC2_EMULATOR_PREFIX + "SelectedGame");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SelectGame") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SelectMachine", startupConfig->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SelectGame", true).toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SelectGame");
			}
			startupConfig->remove(QMC2_FRONTEND_PREFIX + "SoftwareROMAlyzer/SelectGame");
		}
		if ( QMC2_TEST_VERSION(omv, 70, osr, 7829) ) {
			// we need to reset the emulator control panel's header state to ensure all columns are shown on all OSs from now on
			startupConfig->remove(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/EmulatorControlHeaderState");
		}
		if ( QMC2_TEST_VERSION(omv, 183, osr, 7910) ) {
			// we got rid of using a temporary file *completely* in this revision
			startupConfig->remove(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile");
		}
		if ( QMC2_TEST_VERSION(omv, 187, osr, 8076) ) {
			// QMC2 Arcade / ToxicWaste theme changes
			if ( startupConfig->contains("Arcade/ToxicWaste/gameCardPage") ) {
				startupConfig->setValue("Arcade/ToxicWaste/machineCardPage", startupConfig->value("Arcade/ToxicWaste/gameCardPage", 0).toInt());
				startupConfig->remove("Arcade/ToxicWaste/gameCardPage");
			}
			if ( startupConfig->contains("Arcade/ToxicWaste/gameListOpacity") ) {
				startupConfig->setValue("Arcade/ToxicWaste/machineListOpacity", startupConfig->value("Arcade/ToxicWaste/gameListOpacity", 1.0).toDouble());
				startupConfig->remove("Arcade/ToxicWaste/gameListOpacity");
			}
		}
	}

	configOkay &= !startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", QString()).toString().isEmpty();
	configOkay &= !QMC2_CLI_OPT_RECONFIGURE;

	return configOkay;
}
