#include <QTimer>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QDir>
#include <QMap>

#include "welcome.h"
#include "macros.h"
#include "cryptedbytearray.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

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
		comboBoxLanguage->blockSignals(true);
		comboBoxLanguage->addItems(availableLanguages);
		originalLanguage = startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Language", QString()).toString();
		int langIndex = comboBoxLanguage->findText(originalLanguage);
		if ( langIndex >= 0 )
			comboBoxLanguage->setCurrentIndex(langIndex);
		comboBoxLanguage->blockSignals(false);
		lineEditExecutableFile->setText(startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", QString()).toString());
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
		QFileInfo fileInfo(lineEditExecutableFile->text());
		if ( fileInfo.isExecutable() && fileInfo.isReadable() && fileInfo.isFile() ) {
			startupConfig->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", lineEditExecutableFile->text());
			if ( !lineEditWorkingDirectory->text().isEmpty() ) {
				QString s = lineEditWorkingDirectory->text();
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
			startupConfig->sync();
			emit accept();
		} else
			QMessageBox::critical(this, tr("Error"), tr("The specified file isn't executable!"));
	} else
		emit accept();
}

void Welcome::on_toolButtonBrowseExecutableFile_clicked()
{
	QString s;
	if ( lineEditExecutableFile->text().isEmpty() )
		s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), QString(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	else {
		QFileInfo fileInfo(lineEditExecutableFile->text());
		s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), fileInfo.absolutePath(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	}
	if ( !s.isEmpty() )
		lineEditExecutableFile->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseWorkingDirectory_clicked()
{
	QString workingDirectory = lineEditWorkingDirectory->text();
	QString executableFile = lineEditExecutableFile->text();
	QString suggestion = workingDirectory;
	if ( workingDirectory.isEmpty() && !executableFile.isEmpty() ) {
		QFileInfo fi(executableFile);
		suggestion = fi.dir().absolutePath();
	}
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), suggestion, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditWorkingDirectory->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseROMPath_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose ROM path"), lineEditROMPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditROMPath->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseSamplePath_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose sample path"), lineEditSamplePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditSamplePath->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseHashPath_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose hash path"), lineEditHashPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
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
	QString lang = startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Language", QString()).toString();
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

	QString directory = startupConfig->value("FilesAndDirectories/DataDirectory", "data/").toString() + "lng/";
	if ( qmc2QtTranslator ) {
		qApp->removeTranslator(qmc2QtTranslator);
		delete qmc2QtTranslator;
	}
	qmc2QtTranslator = new QTranslator(0);
	qmc2QtTranslator->load(directory + QString("qt_") + lang + ".qm");
	qApp->installTranslator(qmc2QtTranslator);
	if ( qmc2Translator ) {
		qApp->removeTranslator(qmc2Translator);
		delete qmc2Translator;
	}
	qmc2Translator = new QTranslator(0);
	qmc2Translator->load(directory + QString("qmc2_") + lang + ".qm");
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
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QMC2_DYNAMIC_DOT_PATH);

	startupConfig = new Settings(QSettings::IniFormat, QSettings::UserScope, "qmc2");

#if defined(QMC2_SDLMAME)
	variant = "qmc2-sdlmame";
#elif defined(QMC2_MAME)
	variant = "qmc2-mame";
#else
	variant = "qmc2-???";
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

	QStringList verList = startupConfig->value("Version").toString().split(".");
	if ( verList.count() > 1 ) {
		int omv = verList[1].toInt();
		int osr = startupConfig->value("SVN_Revision").toInt();
		if ( QMC2_TEST_VERSION(omv, 50, osr, 6566) ) {
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/ActiveDetails") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MAME + "Layout/Component2/ActiveFeatures", startupConfig->value(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/ActiveDetails").toStringList());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/ActiveDetails");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/ActiveDetails") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MESS + "Layout/Component2/ActiveFeatures", startupConfig->value(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/ActiveDetails").toStringList());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/ActiveDetails");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/ActiveDetails") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_UME + "Layout/Component2/ActiveFeatures", startupConfig->value(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/ActiveDetails").toStringList());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/ActiveDetails");
			}
		}
		if ( QMC2_TEST_VERSION(omv, 50, osr, 6579) ) {
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/hSplitterFlipped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/hSplitterSwapped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/vSplitterFlipped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/vSplitterSwapped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/hSplitterFlipped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/hSplitterSwapped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/vSplitterFlipped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/vSplitterSwapped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/hSplitterFlipped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/hSplitterSwapped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/vSplitterFlipped");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/vSplitterSwapped");
		}
		if ( QMC2_TEST_VERSION(omv, 52, osr, 6646) ) {
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS);
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME);
			startupConfig->remove(QMC2_EMULATOR_PREFIX_MESS);
			startupConfig->remove(QMC2_EMULATOR_PREFIX_UME);
		}
		if ( QMC2_TEST_VERSION(omv, 52, osr, 6650) ) {
			QMap<QString, QVariant> valueMap;
			startupConfig->beginGroup(QMC2_FRONTEND_PREFIX_MAME);
			foreach (QString key, startupConfig->allKeys()) {
				valueMap[key] = startupConfig->value(key);
				startupConfig->remove(key);
			}
			startupConfig->endGroup();
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME);
			foreach (QString key, valueMap.uniqueKeys())
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + key, valueMap[key]);
		}
		if ( QMC2_TEST_VERSION(omv, 52, osr, 6660) ) {
			startupConfig->remove("MAME/FilesAndDirectories/MAWSCacheDirectory");
			startupConfig->remove("Frontend/MAWS/Zoom");
			startupConfig->remove("Frontend/GUI/ExitOnVariantLaunch");
			startupConfig->remove("Frontend/GUI/MinimizeOnVariantLaunch");
		}
		if ( QMC2_TEST_VERSION(omv, 53, osr, 6702) ) {
			QString oldKeySubString("Gamelist");
			QString newKeySubString("MachineList");
			foreach (QString key, startupConfig->allKeys()) {
				if ( key.contains(oldKeySubString) ) {
					QVariant value = startupConfig->value(key);
					startupConfig->remove(key);
					QString newKey = key.replace(oldKeySubString, newKeySubString);
					startupConfig->setValue(newKey, value);
				}
			}
		}
		if ( QMC2_TEST_VERSION(omv, 53, osr, 6711) ) {
			QString oldKeySubString("MESSDeviceConfigurator");
			QString newKeySubString("DeviceConfigurator");
			foreach (QString key, startupConfig->allKeys()) {
				if ( key.contains(oldKeySubString) ) {
					QVariant value = startupConfig->value(key);
					startupConfig->remove(key);
					QString newKey = key.replace(oldKeySubString, newKeySubString);
					startupConfig->setValue(newKey, value);
				}
			}
		}
		if ( QMC2_TEST_VERSION(omv, 53, osr, 6749) ) {
			QString oldKeySubString("MESS/Configuration/Devices");
			QString newKeySubString("MAME/Configuration/Devices");
			foreach (QString key, startupConfig->allKeys()) {
				if ( key.startsWith(oldKeySubString) ) {
					QVariant value = startupConfig->value(key);
					startupConfig->remove(key);
					QString newKey = key.replace(oldKeySubString, newKeySubString);
					startupConfig->setValue(newKey, value);
				}
			}
		}
		if ( QMC2_TEST_VERSION(omv, 53, osr, 6752) ) {
			QString key("/Network/HTTPProxy/Password");
			if ( startupConfig->contains(key) ) {
				QString oldPwd(QMC2_UNCOMPRESS(startupConfig->value("Network/HTTPProxy/Password", QString()).toByteArray()));
				CryptedByteArray cpw(oldPwd.toLatin1());
				startupConfig->setValue("Network/HTTPProxy/Password", cpw.encryptedData());
			}
		}
	}

	configOkay &= !startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString().isEmpty();
	configOkay &= !QMC2_CLI_OPT_RECONFIGURE;

	return configOkay;
}
