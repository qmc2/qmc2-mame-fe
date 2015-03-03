#include <QTimer>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QDir>

#include "welcome.h"
#include "macros.h"

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Welcome::Welcome(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	checkOkay = false;
	hide();
	if ( !checkConfig() ) {
		setupUi(this);

#if defined(QMC2_EMUTYPE_MESS)
		labelLogoPixmap->setPixmap(QString::fromUtf8(":/data/img/qmc2_mess_logo_big.png"));
#elif defined(QMC2_EMUTYPE_UME)
		labelLogoPixmap->setPixmap(QString::fromUtf8(":/data/img/qmc2_ume_logo_big.png"));
#endif

#if defined(QMC2_SDLMAME)
		QString emulatorName = tr("SDLMAME");
#elif defined(QMC2_SDLUME)
		QString emulatorName = tr("SDLUME");
#elif defined(QMC2_SDLMESS)
		QString emulatorName = tr("SDLMESS");
#elif defined(QMC2_MAME)
		QString emulatorName = tr("MAME");
#elif defined(QMC2_UME)
		QString emulatorName = tr("UME");
#elif defined(QMC2_MESS)
		QString emulatorName = tr("MESS");
#else
		QString emulatorName = tr("Unsupported emulator");
#endif
		labelExecutableFile->setText(tr("%1 executable file").arg(emulatorName));
		adjustSize();
		show();
	} else {
		checkOkay = true;
		QTimer::singleShot(0, this, SLOT(on_pushButtonOkay_clicked()));
	}
}

Welcome::~Welcome()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::~Welcome()");
#endif

	delete startupConfig;
}

void Welcome::on_pushButtonOkay_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_pushButtonOkay_clicked()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseExecutableFile_clicked()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseWorkingDirectory_clicked()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseROMPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose ROM path"), lineEditROMPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditROMPath->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseSamplePath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseSamplePath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose sample path"), lineEditSamplePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditSamplePath->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseHashPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseHashPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose hash path"), lineEditHashPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditHashPath->setText(s);
	raise();
}

void Welcome::setupLanguage()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::setupLanguage()");
#endif

	QString lang = startupConfig->value("GUI/Language").toString();
	QStringList availableLanguages;
	availableLanguages << "de" << "es" << "el" << "fr" << "it" << "pl" << "pt" << "ro" << "sv" << "us";

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
		startupConfig->setValue("GUI/Language", lang);
	}

	QString directory = startupConfig->value("FilesAndDirectories/DataDirectory", "data/").toString() + "lng/";
	qmc2QtTranslator = new QTranslator(0);
	qmc2QtTranslator->load(directory + QString("qt_") + lang + ".qm");
	qApp->installTranslator(qmc2QtTranslator);
	qmc2Translator = new QTranslator(0);
	qmc2Translator->load(directory + QString("qmc2_") + lang + ".qm");
	qApp->installTranslator(qmc2Translator);
}

bool Welcome::checkConfig()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::checkConfig()");
#endif

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
#elif defined(QMC2_SDLMESS)
	variant = "qmc2-sdlmess";
#elif defined(QMC2_SDLUME)
	variant = "qmc2-sdlume";
#elif defined(QMC2_MAME)
	variant = "qmc2-mame";
#elif defined(QMC2_MESS)
	variant = "qmc2-mess";
#elif defined(QMC2_UME)
	variant = "qmc2-ume";
#else
	variant = "qmc2-???";
#endif

	startupConfig->beginGroup(QString("Frontend/%1").arg(variant));

	setupLanguage();

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
#if defined(QMC2_OS_MAC)
		if ( QMC2_TEST_VERSION(omv, 47, osr, 6350) ) {
			// changed meaning of Command+Q / Command+X no longer used
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Shortcuts/Ctrl+Q");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Shortcuts/Ctrl+X");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Shortcuts/Ctrl+Q");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Shortcuts/Ctrl+X");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Shortcuts/Ctrl+Q");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Shortcuts/Ctrl+X");
		}
#endif
		if ( QMC2_TEST_VERSION(omv, 47, osr, 6367) ) {
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/SampleChecker/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/OptionsWidget/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/ROMAlyzer/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/SoftwareROMAlyzer/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/SampleChecker/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/OptionsWidget/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/ROMAlyzer/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/SoftwareROMAlyzer/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/SampleChecker/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/OptionsWidget/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/ROMAlyzer/Visible");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/SoftwareROMAlyzer/Visible");
		}
		if ( QMC2_TEST_VERSION(omv, 49, osr, 6467) ) {
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "ROMAlyzer/SetRewriterIgnoreErrors");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "SoftwareROMAlyzer/SetRewriterIgnoreErrors");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "ROMAlyzer/SetRewriterIgnoreErrors");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "SoftwareROMAlyzer/SetRewriterIgnoreErrors");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "ROMAlyzer/SetRewriterIgnoreErrors");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "SoftwareROMAlyzer/SetRewriterIgnoreErrors");
		}
		if ( QMC2_TEST_VERSION(omv, 49, osr, 6479) ) {
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/CollectionRebuilder/Geometry");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/SoftwareCollectionRebuilder/Geometry");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "GUI/SaveLayout");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "GUI/RestoreLayout");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/CollectionRebuilder/Geometry");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/SoftwareCollectionRebuilder/Geometry");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "GUI/SaveLayout");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "GUI/RestoreLayout");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/CollectionRebuilder/Geometry");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/SoftwareCollectionRebuilder/Geometry");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "GUI/SaveLayout");
			startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "GUI/RestoreLayout");
		}
	}

	configOkay &= !startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString().isEmpty();
	return configOkay;
}
