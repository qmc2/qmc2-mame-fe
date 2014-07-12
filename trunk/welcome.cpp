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
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), suggestion, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditWorkingDirectory->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseROMPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseROMPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose ROM path"), lineEditROMPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditROMPath->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseSamplePath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseSamplePath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose sample path"), lineEditSamplePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSamplePath->setText(s);
	raise();
}

void Welcome::on_toolButtonBrowseHashPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseHashPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose hash path"), lineEditHashPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
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
		if ( QMC2_TEST_VERSION(omv, 37, osr, 3983) ) {
			// add F7 standard-shortcut for all MESS targets
			if ( !startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Shortcuts/F7") )
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MESS + "Shortcuts/F7", "F7");
		}
		if ( QMC2_TEST_VERSION(omv, 37, osr, 4160) ) {
			// rename "HtmlEditor/*" keys
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "HtmlEditor/Zoom") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MAME + "HtmlEditor/SoftwareNotes/Zoom", startupConfig->value(QMC2_FRONTEND_PREFIX_MAME + "HtmlEditor/Zoom", 100).toInt());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "HtmlEditor/Zoom");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "HtmlEditor/WidgetState") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MAME + "HtmlEditor/SoftwareNotes/WidgetState", startupConfig->value(QMC2_FRONTEND_PREFIX_MAME + "HtmlEditor/WidgetState", QByteArray()).toByteArray());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "HtmlEditor/WidgetState");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "HtmlEditor/Zoom") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MESS + "HtmlEditor/SoftwareNotes/Zoom", startupConfig->value(QMC2_FRONTEND_PREFIX_MESS + "HtmlEditor/Zoom", 100).toInt());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "HtmlEditor/Zoom");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "HtmlEditor/WidgetState") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MESS + "HtmlEditor/SoftwareNotes/WidgetState", startupConfig->value(QMC2_FRONTEND_PREFIX_MESS + "HtmlEditor/WidgetState", QByteArray()).toByteArray());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "HtmlEditor/WidgetState");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "HtmlEditor/Zoom") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_UME + "HtmlEditor/SoftwareNotes/Zoom", startupConfig->value(QMC2_FRONTEND_PREFIX_UME + "HtmlEditor/Zoom", 100).toInt());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "HtmlEditor/Zoom");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "HtmlEditor/WidgetState") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_UME + "HtmlEditor/SoftwareNotes/WidgetState", startupConfig->value(QMC2_FRONTEND_PREFIX_UME + "HtmlEditor/WidgetState", QByteArray()).toByteArray());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "HtmlEditor/WidgetState");
			}
		}
		if ( QMC2_TEST_VERSION(omv, 38, osr, 4304) ) {
			// remove deprecated keys: "SampleChecker/SelectGame"
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "SampleChecker/SelectGame") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "SampleChecker/SelectGame");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "SampleChecker/SelectGame") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "SampleChecker/SelectGame");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "SampleChecker/SelectGame") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "SampleChecker/SelectGame");
		}
		if ( QMC2_TEST_VERSION(omv, 38, osr, 4327) ) {
			// remove deprecated keys: "Tools/FileRemovalTool*"
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "Tools/FileRemovalTool") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Tools/FileRemovalTool");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "Tools/FileRemovalToolArguments") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Tools/FileRemovalToolArguments");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Tools/FileRemovalTool") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Tools/FileRemovalTool");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Tools/FileRemovalToolArguments") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Tools/FileRemovalToolArguments");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "Tools/FileRemovalTool") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Tools/FileRemovalTool");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "Tools/FileRemovalToolArguments") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Tools/FileRemovalToolArguments");
		}
		if ( QMC2_TEST_VERSION(omv, 38, osr, 4420) ) {
			// remove deprecated keys: "Configuration/Global/OptionColumnWidth" keys
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX_MAME + "Configuration/Global/OptionColumnWidth") )
				startupConfig->remove(QMC2_EMULATOR_PREFIX_MAME + "Configuration/Global/OptionColumnWidth");
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX_MESS + "Configuration/Global/OptionColumnWidth") )
				startupConfig->remove(QMC2_EMULATOR_PREFIX_MESS + "Configuration/Global/OptionColumnWidth");
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX_UME + "Configuration/Global/OptionColumnWidth") )
				startupConfig->remove(QMC2_EMULATOR_PREFIX_UME + "Configuration/Global/OptionColumnWidth");
		}
		if ( QMC2_TEST_VERSION(omv, 38, osr, 4433) ) {
			// remove old "Arcade" keys (will be replaced by new ones)
			if ( startupConfig->childGroups().contains("Arcade") ) {
				startupConfig->beginGroup("Arcade");
				startupConfig->remove("");
				startupConfig->endGroup();
			}
		}
		if ( QMC2_TEST_VERSION(omv, 39, osr, 4813) ) {
			// remove deprecated keys: "Gamelist/SortOnline" & "Gamelist/HideWhileLoading"
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "Gamelist/SortOnline") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Gamelist/SortOnline");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Gamelist/SortOnline") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Gamelist/SortOnline");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "Gamelist/SortOnline") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Gamelist/SortOnline");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "Gamelist/HideWhileLoading") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Gamelist/HideWhileLoading");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Gamelist/HideWhileLoading") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Gamelist/HideWhileLoading");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "Gamelist/HideWhileLoading") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Gamelist/HideWhileLoading");
		}
#if defined(QMC2_OS_UNIX) || defined(QMC2_OS_WIN)
		if ( QMC2_TEST_VERSION(omv, 39, osr, 4818) ) {
			// remove deprecated keys: "Embedder/NativeSnapshotResolution"
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "Embedder/NativeSnapshotResolution") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Embedder/NativeSnapshotResolution");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Embedder/NativeSnapshotResolution") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Embedder/NativeSnapshotResolution");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "Embedder/NativeSnapshotResolution") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Embedder/NativeSnapshotResolution");
		}
#endif
		if ( QMC2_TEST_VERSION(omv, 40, osr, 5171) ) {
			// reset "Layout/MainWidget/EmulatorControlHeaderState"
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/EmulatorControlHeaderState") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/EmulatorControlHeaderState");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/EmulatorControlHeaderState") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/EmulatorControlHeaderState");
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/EmulatorControlHeaderState") )
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/EmulatorControlHeaderState");
		}
		if ( QMC2_TEST_VERSION(omv, 43, osr, 5640) ) {
			// remove the old list-xml-cache file and the deprecated "FilesAndDirectories/ListXMLCache" settings key
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ListXMLCache") ) {
				QFile f(startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ListXMLCache").toString());
				if ( f.exists() )
					f.remove();
				startupConfig->remove(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ListXMLCache");
			}
		}
		if ( QMC2_TEST_VERSION(omv, 43, osr, 5684) ) {
			// separate and rename MAME/MESS emulator info DB keys (in order to be able to merge them for UME)
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB");

			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameInfoDat", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB");

			}
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/EmuInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat", startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/EmuInfoDB").toString());
				startupConfig->remove(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/EmuInfoDB");
			}
#elif defined(QMC2_EMUTYPE_MESS)
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB");

			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessInfoDat", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB");

			}
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/EmuInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat", startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/EmuInfoDB").toString());
				startupConfig->remove(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/EmuInfoDB");
			}
#endif
		}
		if ( QMC2_TEST_VERSION(omv, 43, osr, 5688) ) {
			// separate and rename MAME/MESS game/machine info DB keys (in order to be able to merge them for UME)
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB");

			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameHistoryDat", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB");

			}
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GameInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat", startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GameInfoDB").toString());
				startupConfig->remove(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GameInfoDB");
			}
#elif defined(QMC2_EMUTYPE_MESS)
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB");

			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessSysinfoDat", startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB");

			}
			if ( startupConfig->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GameInfoDB") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat", startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GameInfoDB").toString());
				startupConfig->remove(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GameInfoDB");
			}
#endif
		}
		if ( QMC2_TEST_VERSION(omv, 44, osr, 5837) ) {
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistHeaderState") ||
			     startupConfig->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyHeaderState") ||
			     startupConfig->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/CategoryViewHeaderState") ||
			     startupConfig->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/VersionViewHeaderState") ) {
#if defined(QMC2_EMUTYPE_MESS)
				QMessageBox::information(this,
						tr("Important upgrade information"),
						tr("<p>A new data column ('%1') has been added to all machine lists!</p>"
						   "<p>Please check/adapt column visibility <a href=\"%2\">as shown in our wiki</a>!</p>"
						   "<p>Note that you'll have to do this for the other QMC2 variants as well (this message will not be shown again).</p>").arg("Rank").arg("http://wiki.batcom-it.net/index.php?title=The_'ultimate'_guide_to_QMC2#Customizing_columns"));
#else
				QMessageBox::information(this,
						tr("Important upgrade information"),
						tr("<p>A new data column ('%1') has been added to all game lists!</p>"
						   "<p>Please check/adapt column visibility <a href=\"%2\">as shown in our wiki</a>!</p>"
						   "<p>Note that you'll have to do this for the other QMC2 variants as well (this message will not be shown again).</p>").arg("Rank").arg("http://wiki.batcom-it.net/index.php?title=The_'ultimate'_guide_to_QMC2#Customizing_columns"));
#endif
			}
		}
		if ( QMC2_TEST_VERSION(omv, 44, osr, 6024) ) {
			// rename "Layout/MainWindow/NegateSearch" to "Layout/MainWidget/NegateSearch" (as it should)
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWindow/NegateSearch") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWidget/NegateSearch", startupConfig->value(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWindow/NegateSearch").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MAME + "Layout/MainWindow/NegateSearch");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWindow/NegateSearch") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWidget/NegateSearch", startupConfig->value(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWindow/NegateSearch").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_MESS + "Layout/MainWindow/NegateSearch");
			}
			if ( startupConfig->contains(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWindow/NegateSearch") ) {
				startupConfig->setValue(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWidget/NegateSearch", startupConfig->value(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWindow/NegateSearch").toBool());
				startupConfig->remove(QMC2_FRONTEND_PREFIX_UME + "Layout/MainWindow/NegateSearch");
			}
		}
	}

	configOkay &= !startupConfig->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString().isEmpty();
	return configOkay;
}
