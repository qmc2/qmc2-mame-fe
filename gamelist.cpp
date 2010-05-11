#include <QTextStream>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QFile>
#include <QFontMetrics>
#include <QFont>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <QDir>
#include <QBitArray>
#include <QByteArray>
#include <QWebView>

#include "gamelist.h"
#include "emuopt.h"
#include "qmc2main.h"
#include "options.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "romstatusexport.h"
#include "miniwebbrowser.h"
#include "macros.h"
#include "unzip.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern QSettings *qmc2Config;
extern EmulatorOptions *qmc2EmulatorOptions;
extern ROMStatusExporter *qmc2ROMStatusExporter;
extern bool qmc2ReloadActive;
extern bool qmc2EarlyReloadActive;
extern bool qmc2StopParser;
extern bool qmc2StartingUp;
extern bool qmc2VerifyActive;
extern bool qmc2FilterActive;
extern bool qmc2UseIconFile;
extern bool qmc2IconsPreloaded;
extern bool qmc2WidgetsEnabled;
extern bool qmc2StatesTogglesEnabled;
extern int qmc2GamelistResponsiveness;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QTreeWidgetItem *qmc2LastDeviceConfigItem;
extern QTreeWidgetItem *qmc2LastGameInfoItem;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemByDescriptionMap;
extern QMap<QString, QString> qmc2GamelistNameMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;
extern QMap<QString, QString> qmc2GamelistStatusMap;
extern QMap<QString, QStringList> qmc2HierarchyMap;
extern QMap<QString, QString> qmc2ParentMap;
extern int qmc2SortCriteria;
extern Qt::SortOrder qmc2SortOrder;
extern QBitArray qmc2Filter;
extern unzFile qmc2IconFile;
extern QMap<QString, QIcon> qmc2IconMap;
extern QStringList qmc2BiosROMs;
#if defined(QMC2_EMUTYPE_MAME)
extern QTreeWidgetItem *qmc2LastMAWSItem;
extern MiniWebBrowser *qmc2MAWSLookup;
extern QTreeWidgetItem *qmc2LastEmuInfoItem;
extern QMap<QString, QByteArray *> qmc2EmuInfoDB;
extern QMap<QString, QString> qmc2CategoryMap;
extern QMap<QString, QString> qmc2VersionMap;
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif

// local global variables
QStringList Gamelist::phraseTranslatorList;
int numVerifyRoms = 0;
QString verifyLastLine;
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
QStringList verifiedList;
#endif

Gamelist::Gamelist(QObject *parent)
  : QObject(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::Gamelist()");
#endif

  numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numSearchGames = -1;
  loadProc = verifyProc = NULL;
  autoROMCheck = FALSE;
  emulatorVersion = tr("unknown");

  QString imgDir = qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", "data/").toString() + "img/";
  qmc2SmallGhostImageIcon.addFile(imgDir + "ghost_small.png");
  qmc2UnknownImageIcon.addFile(imgDir + "sphere_blue.png");
  qmc2UnknownBIOSImageIcon.addFile(imgDir + "sphere_blue_bios.png");
  qmc2CorrectImageIcon.addFile(imgDir + "sphere_green.png");
  qmc2CorrectBIOSImageIcon.addFile(imgDir + "sphere_green_bios.png");
  qmc2MostlyCorrectImageIcon.addFile(imgDir + "sphere_yellowgreen.png");
  qmc2MostlyCorrectBIOSImageIcon.addFile(imgDir + "sphere_yellowgreen_bios.png");
  qmc2IncorrectImageIcon.addFile(imgDir + "sphere_red.png");
  qmc2IncorrectBIOSImageIcon.addFile(imgDir + "sphere_red_bios.png");
  qmc2NotFoundImageIcon.addFile(imgDir + "sphere_grey.png");
  qmc2NotFoundBIOSImageIcon.addFile(imgDir + "sphere_grey_bios.png");

  if ( phraseTranslatorList.isEmpty() )
    phraseTranslatorList << tr("good") << tr("bad") << tr("preliminary") << tr("supported") << tr("unsupported")
                         << tr("imperfect") << tr("yes") << tr("no") << tr("baddump") << tr("nodump")
                         << tr("vertical") << tr("horizontal") << tr("raster") << tr("unknown") << tr("Unknown") 
                         << tr("On") << tr("Off") << tr("audio") << tr("unused") << tr("Unused") << tr("cpu")
                         << tr("vector") << tr("lcd") << tr("joy4way") << tr("joy8way") << tr("trackball")
                         << tr("joy2way") << tr("doublejoy8way") << tr("dial") << tr("paddle") << tr("pedal")
                         << tr("stick") << tr("vjoy2way") << tr("lightgun") << tr("doublejoy4way") << tr("vdoublejoy2way")
                         << tr("doublejoy2way") << tr("printer") << tr("cdrom") << tr("cartridge") << tr("cassette")
                         << tr("quickload") << tr("floppydisk") << tr("serial") << tr("snapshot");

  if ( qmc2UseIconFile ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2IconFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/IconFile").toString().toAscii());
    if ( qmc2IconFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(qmc2Config->value("MAME/FilesAndDirectories/IconFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2IconFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/IconFile").toString().toAscii());
    if ( qmc2IconFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(qmc2Config->value("MESS/FilesAndDirectories/IconFile").toString()));
#endif
  }
}

Gamelist::~Gamelist()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::~Gamelist()");
#endif

  if ( loadProc )
    loadProc->terminate();

  if ( verifyProc )
    verifyProc->terminate();
}

void Gamelist::enableWidgets(bool enable)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::enableWidgets(bool enable = " + QString(enable ? "TRUE" : "FALSE") + ")");
#endif

  // store widget enablement flag for later dialog setups
  qmc2WidgetsEnabled = enable;

  qmc2Options->toolButtonBrowseStyleSheet->setEnabled(enable);
  qmc2Options->toolButtonBrowseFont->setEnabled(enable);
  qmc2Options->toolButtonBrowseLogFont->setEnabled(enable);
  qmc2Options->toolButtonBrowseTemporaryFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseFrontendLogFile->setEnabled(enable);
  qmc2Options->toolButtonBrowsePreviewDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowsePreviewFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseDataDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseGameInfoDB->setEnabled(enable);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2Options->toolButtonBrowseEmuInfoDB->setEnabled(enable);
  qmc2Options->toolButtonBrowseMAWSCacheDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseCatverIniFile->setEnabled(enable);
  qmc2Options->checkBoxUseCatverIni->setEnabled(enable);
#endif
  qmc2Options->toolButtonBrowseExecutableFile->setEnabled(enable);
  qmc2Options->lineEditExecutableFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseEmulatorLogFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseOptionsTemplateFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseListXMLCache->setEnabled(enable);
  qmc2Options->toolButtonBrowseFavoritesFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseHistoryFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseGamelistCacheFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseROMStateCacheFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseFlyerDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseFlyerFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseIconDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseIconFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseCabinetDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseCabinetFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseControllerDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseControllerFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseMarqueeDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseMarqueeFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseTitleDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseTitleFile->setEnabled(enable);
  qmc2Options->toolButtonBrowsePCBDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowsePCBFile->setEnabled(enable);
  qmc2Options->toolButtonShowC->setEnabled(enable);
  qmc2Options->toolButtonShowM->setEnabled(enable);
  qmc2Options->toolButtonShowI->setEnabled(enable);
  qmc2Options->toolButtonShowN->setEnabled(enable);
  qmc2Options->toolButtonShowU->setEnabled(enable);
  qmc2Options->comboBoxSortCriteria->setEnabled(enable);
  qmc2Options->comboBoxSortOrder->setEnabled(enable);
  qmc2Options->treeWidgetShortcuts->clearSelection();
  qmc2Options->treeWidgetShortcuts->setEnabled(enable);
  qmc2Options->treeWidgetJoystickMappings->clearSelection();
  qmc2Options->treeWidgetJoystickMappings->setEnabled(enable);
  qmc2Options->toolButtonBrowseZipTool->setEnabled(enable);
  qmc2Options->toolButtonBrowseFileRemovalTool->setEnabled(enable);
  qmc2Options->toolButtonBrowseAdditionalEmulatorExecutable->setEnabled(enable);
#if QMC2_USE_PHONON_API
  qmc2MainWindow->toolButtonAudioAddTracks->setEnabled(enable);
#endif
  if ( qmc2ROMStatusExporter )
    qmc2ROMStatusExporter->pushButtonExport->setEnabled(enable);
  qmc2MainWindow->pushButtonSelectRomFilter->setEnabled(enable);
}

void Gamelist::load()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::load()");
#endif

  QString userScopePath = QMC2_DOT_PATH;

  qmc2ReloadActive = qmc2EarlyReloadActive = TRUE;
  qmc2StopParser = FALSE;
  qmc2GamelistItemMap.clear();
  qmc2GamelistNameMap.clear();
  qmc2GamelistItemByDescriptionMap.clear();
  qmc2GamelistDescriptionMap.clear();
  qmc2GamelistStatusMap.clear();
  qmc2BiosROMs.clear();
  qmc2HierarchyItemMap.clear();

  enableWidgets(FALSE);

  numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numSearchGames = -1;
  qmc2MainWindow->treeWidgetGamelist->clear();
  qmc2MainWindow->treeWidgetHierarchy->clear();
#if defined(QMC2_EMUTYPE_MAME)
  qmc2CategoryItemMap.clear();
  qmc2VersionItemMap.clear();
  qmc2MainWindow->treeWidgetCategoryView->clear();
  qmc2MainWindow->treeWidgetVersionView->clear();
#endif
  qmc2MainWindow->listWidgetSearch->clear();
  qmc2MainWindow->textBrowserGameInfo->clear();
  qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorBlue);
  qmc2CurrentItem = NULL;
  qmc2LastDeviceConfigItem = NULL;
  qmc2LastGameInfoItem = NULL;
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->textBrowserEmuInfo->clear();
  qmc2LastEmuInfoItem = NULL;
  if ( qmc2MAWSLookup ) {
    qmc2MAWSLookup->setVisible(FALSE);
    QLayout *vbl = qmc2MainWindow->tabMAWS->layout();
    if ( vbl )
      delete vbl;
    delete qmc2MAWSLookup;
    qmc2MAWSLookup = NULL;
  }
  qmc2LastMAWSItem = NULL;
#endif

  qmc2Preview->update();
  qmc2Flyer->update();
  qmc2Cabinet->update();
  qmc2Controller->update();
  qmc2Marquee->update();
  qmc2Title->update();
  qmc2PCB->update();

  qApp->processEvents();
  QTreeWidgetItem *dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetGamelist);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetHierarchy);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
#if defined(QMC2_EMUTYPE_MAME)
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetCategoryView);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetVersionView);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
#endif
  if ( qmc2EmulatorOptions ) {
    qmc2EmulatorOptions->save();
    QLayout *vbl = qmc2MainWindow->tabConfiguration->layout();
    if ( vbl )
      delete vbl;
    delete qmc2MainWindow->labelEmuSelector;
    delete qmc2MainWindow->comboBoxEmuSelector;
    delete qmc2EmulatorOptions;
    delete qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile;
    delete qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile;
    qmc2EmulatorOptions = NULL;
  }
  qmc2MainWindow->labelGamelistStatus->setText(status());

#if defined(QMC2_EMUTYPE_MAME)
  switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
    case QMC2_VIEW_CATEGORY_INDEX:
      QTimer::singleShot(0, qmc2MainWindow, SLOT(viewByCategory()));
      break;
    case QMC2_VIEW_VERSION_INDEX:
      QTimer::singleShot(0, qmc2MainWindow, SLOT(viewByVersion()));
      break;
  }
#endif

  // determine emulator version and supported games
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("determining emulator version and supported games"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("determining emulator version and supported machines"));
#endif

  QStringList args;
  QTime elapsedTime;
  parseTimer.start();
  QString command;

  // emulator version
  QProcess commandProc;
#if defined(QMC2_SDLMAME)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#else
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif

#if !defined(Q_WS_WIN)
  commandProc.setStandardErrorFile("/dev/null");
#endif
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
  args << "-help";
#endif
  qApp->processEvents();
#if defined(QMC2_EMUTYPE_MAME)
  commandProc.start(qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString(), args);
#elif defined(QMC2_EMUTYPE_MESS)
  commandProc.start(qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString(), args);
#endif
  bool commandProcStarted = FALSE;
  if ( commandProc.waitForStarted() ) {
    commandProcStarted = TRUE;
    bool commandProcRunning = (commandProc.state() == QProcess::Running);
    while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
      qApp->processEvents();
      commandProcRunning = (commandProc.state() == QProcess::Running);
    }
  }

#if defined(QMC2_SDLMAME)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#else
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif
  if ( commandProcStarted && qmc2TempVersion.open(QFile::ReadOnly) ) {
    QTextStream ts(&qmc2TempVersion);
    QString s = ts.readAll();
    qmc2TempVersion.close();
    qmc2TempVersion.remove();
    QStringList versionLines = s.split("\n");
#if defined(QMC2_EMUTYPE_MAME)
    QStringList versionWords = versionLines.first().split(" ");
    if ( versionWords.count() > 1 ) {
      if ( versionWords[0] == "M.A.M.E." ) {
        emulatorVersion = versionWords[1].remove("v");
        emulatorType = "MAME";
      } else {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: selected executable file is not MAME"));
        emulatorVersion = tr("unknown");
        emulatorType = versionWords[0];
      }
    } else {
      emulatorVersion = tr("unknown");
      emulatorType = tr("unknown");
    }
#elif defined(QMC2_EMUTYPE_MESS)
    QStringList versionWords = versionLines.first().split(" ");
    if ( versionWords.count() > 1 ) {
      if ( versionWords[0] == "MESS" ) {
        emulatorVersion = versionWords[1].remove("v");
        emulatorType = "MESS";
      } else {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: selected executable file is not MESS"));
        emulatorVersion = tr("unknown");
        emulatorType = versionWords[0];
      }
    } else {
      emulatorVersion = tr("unknown");
      emulatorType = tr("unknown");
    }
#endif
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create temporary file, please check emulator executable and permissions"));
    emulatorVersion = tr("unknown");
    emulatorType = tr("unknown");
  }

  // supported games/machines
  args.clear();
  args << "-listfull";
#if defined(QMC2_AUDIT_WILDCARD)
  args << "*";
#endif
  qApp->processEvents();
#if defined(QMC2_EMUTYPE_MAME)
  commandProc.start(qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString(), args);
#elif defined(QMC2_EMUTYPE_MESS)
  commandProc.start(qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString(), args);
#endif
  commandProcStarted = FALSE;
  if ( commandProc.waitForStarted() ) {
    commandProcStarted = TRUE;
    bool commandProcRunning = (commandProc.state() == QProcess::Running);
    while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
      qApp->processEvents();
      commandProcRunning = (commandProc.state() == QProcess::Running);
    }
  }

#if defined(QMC2_SDLMAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#else
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif
  if ( commandProcStarted && qmc2Temp.open(QFile::ReadOnly) ) {
    QTextStream ts(&qmc2Temp);
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
    QString s = ts.readAll();
#endif
    qmc2Temp.close();
    qmc2Temp.remove();
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
    numTotalGames = s.split("\n").count() - 2;
#endif
    elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (determining emulator version and supported games, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (determining emulator version and supported machines, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create temporary file, please check emulator executable and permissions"));
  }
  qmc2MainWindow->labelGamelistStatus->setText(status());

  if ( emulatorVersion != tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator info: type = %1, version = %2").arg(emulatorType).arg(emulatorVersion));
  else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator type and version"));
    qmc2ReloadActive = FALSE;
    enableWidgets(TRUE);
    return;
  }

  if ( numTotalGames > 0 )
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n supported game(s)", "", numTotalGames));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n supported machine(s)", "", numTotalGames));
#endif
  else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine supported games"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine supported machines"));
#endif
    qmc2ReloadActive = FALSE;
    enableWidgets(TRUE);
    return;
  }

#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool() )
    loadCatverIni();
#endif

  gamelistBuffer.clear();

  // try reading XML output from cache
  bool xmlCacheOkay = FALSE;
#if defined(QMC2_EMUTYPE_MAME)
  listXMLCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/ListXMLCache", userScopePath + "/mame.lxc").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  listXMLCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/ListXMLCache", userScopePath + "/mess.lxc").toString());
#endif
  listXMLCache.open(QIODevice::ReadOnly | QIODevice::Text);
  if ( listXMLCache.isOpen() ) {
    QTextStream ts(&listXMLCache);
    QString singleXMLLine = ts.readLine();
    singleXMLLine = ts.readLine();
#if defined(QMC2_EMUTYPE_MAME)
    if ( singleXMLLine.startsWith("MAME_VERSION") ) {
#elif defined(QMC2_EMUTYPE_MESS)
    if ( singleXMLLine.startsWith("MESS_VERSION") ) {
#endif
      QStringList words = singleXMLLine.split("\t");
      if ( words.count() > 1 ) { 
          if ( emulatorVersion == words[1] )
            xmlCacheOkay = TRUE;
      }
    }
    if ( xmlCacheOkay ) {
      QTime xmlElapsedTime;
      parseTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML game list data from cache"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML machine list data from cache"));
#endif
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("XML cache - %p%"));
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
      qmc2MainWindow->progressBarGamelist->reset();
      int i = 0;
      int gameCount = 0;
      QString readBuffer;
      while ( !ts.atEnd() || !readBuffer.isEmpty() ) {
        readBuffer += ts.read(QMC2_FILE_BUFFER_SIZE);
        bool endsWithNewLine = readBuffer.endsWith("\n");
        QStringList lines = readBuffer.split("\n");
        int l, lc = lines.count();
        if ( !endsWithNewLine )
          lc -= 1;
        for (l = 0; l < lc; l++) {
          if ( !lines[l].isEmpty() ) {
            singleXMLLine = lines[l];
            gamelistBuffer += singleXMLLine + "\n";
#if defined(QMC2_EMUTYPE_MAME)
            gameCount += singleXMLLine.count("<game name=");
#elif defined(QMC2_EMUTYPE_MESS)
            gameCount += singleXMLLine.count("<machine name=");
#endif
          }
        }

        if ( endsWithNewLine )
          readBuffer.clear();
        else
          readBuffer = lines.last();

        if ( i++ % QMC2_XMLCACHE_RESPONSIVENESS == 0 )
          qmc2MainWindow->progressBarGamelist->setValue(gameCount);
      }
      gamelistBuffer += "\n";
      xmlElapsedTime = xmlElapsedTime.addMSecs(parseTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML game list data from cache, elapsed time = %1)").arg(xmlElapsedTime.toString("mm:ss.zzz")));
      if ( singleXMLLine != "</mame>" ) {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML game list cache is incomplete, invalidating XML game list cache"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML machine list data from cache, elapsed time = %1)").arg(xmlElapsedTime.toString("mm:ss.zzz")));
      if ( singleXMLLine != "</mess>" ) {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML machine list cache is incomplete, invalidating XML machine list cache"));
#endif
        xmlCacheOkay = FALSE;
      } else
        qmc2EarlyReloadActive = FALSE;
    }
  }

  if ( listXMLCache.isOpen() )
    listXMLCache.close();

  if ( qmc2StopParser ) {
    qmc2MainWindow->progressBarGamelist->reset();
    qmc2ReloadActive = FALSE;
    enableWidgets(TRUE);
    return;
  }

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/HideWhileLoading", TRUE).toBool() ) {
    // hide game list
    qmc2MainWindow->treeWidgetGamelist->setVisible(FALSE);
    qmc2MainWindow->labelLoadingGamelist->setVisible(TRUE);
    qApp->processEvents();
  }

  if ( xmlCacheOkay ) {
    parse();
    loadFavorites();
    loadPlayHistory();

    // show game list
    qmc2MainWindow->labelLoadingGamelist->setVisible(FALSE);
    qmc2MainWindow->treeWidgetGamelist->setVisible(TRUE);

    if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_GAMELIST_INDEX ) {
      switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
	      case QMC2_VIEW_DETAIL_INDEX:
		      qmc2MainWindow->treeWidgetGamelist->setFocus();
		      break;
	      case QMC2_VIEW_TREE_INDEX:
		      qmc2MainWindow->treeWidgetHierarchy->setFocus();
		      break;
#if defined(QMC2_EMUTYPE_MAME)
	      case QMC2_VIEW_CATEGORY_INDEX:
		      qmc2MainWindow->treeWidgetCategoryView->setFocus();
		      break;
	      case QMC2_VIEW_VERSION_INDEX:
		      qmc2MainWindow->treeWidgetVersionView->setFocus();
		      break;
#endif
	      default:
		      qmc2MainWindow->treeWidgetGamelist->setFocus();
		      break;
      }
    }

    qApp->processEvents();
  } else {
    loadTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML game list data and (re)creating cache"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML machine list data and (re)creating cache"));
#endif
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("XML data - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
#if defined(QMC2_EMUTYPE_MAME)
    command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
#elif defined(QMC2_EMUTYPE_MESS)
    command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
#endif
    args.clear();
    args << "-listxml";
#if defined(QMC2_AUDIT_WILDCARD)
    args << "*";
#endif

    listXMLCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !listXMLCache.isOpen() ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open XML game list cache for writing, please check permissions"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open XML machine list cache for writing, please check permissions"));
#endif
    } else {
      tsListXMLCache.setDevice(&listXMLCache);
      tsListXMLCache.reset();
      tsListXMLCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
#if defined(QMC2_EMUTYPE_MAME)
      tsListXMLCache << "MAME_VERSION\t" + emulatorVersion + "\n";
#elif defined(QMC2_EMUTYPE_MESS)
      tsListXMLCache << "MESS_VERSION\t" + emulatorVersion + "\n";
#endif
    }
    loadProc = new QProcess(this);
    connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
    connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
    connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
    connect(loadProc, SIGNAL(readyReadStandardError()), this, SLOT(loadReadyReadStandardError()));
    connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
    connect(loadProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(loadStateChanged(QProcess::ProcessState)));
    loadProc->start(command, args);
  }
}

void Gamelist::verify(bool currentOnly)
{
  if ( currentOnly )
    if ( !qmc2CurrentItem )
      return;

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::verify(bool currentOnly = %1)").arg(currentOnly));
#endif

  verifyCurrentOnly = currentOnly;
  qmc2VerifyActive = TRUE;
  qmc2StopParser = FALSE;

  enableWidgets(FALSE);

  verifiedList.clear();
  verifyLastLine = "";
  verifyTimer.start();
  numVerifyRoms = 0;
  if ( verifyCurrentOnly ) {
    checkedItem = qmc2CurrentItem;
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for '%1'").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)));
    // decrease counter for current ROMs state
    switch ( checkedItem->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() ) {
      case QMC2_ROMSTATE_CHAR_C:
        numCorrectGames--;
        numUnknownGames++;
        break;

      case QMC2_ROMSTATE_CHAR_M:
        numMostlyCorrectGames--;
        numUnknownGames++;
        break;

      case QMC2_ROMSTATE_CHAR_I:
        numIncorrectGames--;
        numUnknownGames++;
        break;

      case QMC2_ROMSTATE_CHAR_N:
        numNotFoundGames--;
        numUnknownGames++;
        break;

      case QMC2_ROMSTATE_CHAR_U:
      default:
        break;
    }
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for all games"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for all machines"));
#endif
  }
  
  QStringList args;
#if defined(QMC2_EMUTYPE_MAME)
  QString command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
  if ( qmc2Config->contains("MAME/Configuration/Global/rompath") )
    args << "-rompath" << qmc2Config->value("MAME/Configuration/Global/rompath").toString().replace("~", "$HOME");
#elif defined(QMC2_EMUTYPE_MESS)
  QString command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
  if ( qmc2Config->contains("MESS/Configuration/Global/rompath") )
    args << "-rompath" << qmc2Config->value("MESS/Configuration/Global/rompath").toString().replace("~", "$HOME");
#endif
  args << "-verifyroms";
  if ( verifyCurrentOnly )
    args << checkedItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
#if defined(QMC2_AUDIT_WILDCARD)
  else
    args << "*";
#endif

  verifyProc = new QProcess(this);
  connect(verifyProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(verifyError(QProcess::ProcessError)));
  connect(verifyProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(verifyFinished(int, QProcess::ExitStatus)));
  connect(verifyProc, SIGNAL(readyReadStandardOutput()), this, SLOT(verifyReadyReadStandardOutput()));
  connect(verifyProc, SIGNAL(readyReadStandardError()), this, SLOT(verifyReadyReadStandardError()));
  connect(verifyProc, SIGNAL(started()), this, SLOT(verifyStarted()));
  connect(verifyProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(verifyStateChanged(QProcess::ProcessState)));
  verifyProc->start(command, args);
}

QString Gamelist::value(QString element, QString attribute, bool translate)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::value(QString element = " + element + ", QString attribute = \"" + attribute + "\", translate = " + QString(translate ? "TRUE" : "FALSE") + ")");
#endif

  if ( element.contains(attribute) ) {
    QString valueString = element.remove(0, element.indexOf(attribute) + attribute.length() + 2);
    valueString = valueString.remove(valueString.indexOf("\""), valueString.lastIndexOf(">")).replace("&amp;", "&");
    if ( translate )
      return tr(valueString.toAscii());
    else
      return valueString;
  }
  else {
    return QString::null;
  }
}

void Gamelist::insertAttributeItems(QTreeWidgetItem *parent, QString element, QStringList attributes, QStringList descriptions, bool translate)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::insertAttributeItems(QTreeWidgetItem *parent = 0x" + QString::number((ulong)parent, 16) + ", QString element = \"" + element + "\", QStringList attributes = ..., QStringList descriptions = ..., translate = " + QString(translate ? "TRUE" : "FALSE") + ")");
#endif

  int i;
  for (i = 0; i < attributes.count(); i++) {
    QString valueString = value(element, attributes.at(i), translate);
    if ( !valueString.isEmpty() ) {
      QTreeWidgetItem *attributeItem = new QTreeWidgetItem(parent);
      attributeItem->setText(QMC2_GAMELIST_COLUMN_GAME, descriptions.at(i));
      attributeItem->setText(QMC2_GAMELIST_COLUMN_ICON, tr(valueString.toAscii()));
    }
  }
}

void Gamelist::parseGameDetail(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::parseGameDetail(QTreeWidgetItem *item = 0x" + QString::number((ulong)item, 16) + "): item->text(QMC2_GAMELIST_COLUMN_GAME) = \"" + item->text(QMC2_GAMELIST_COLUMN_GAME) + "\"");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("retrieving game information for '%1'").arg(item->text(QMC2_GAMELIST_COLUMN_GAME)));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("retrieving machine information for '%1'").arg(item->text(QMC2_GAMELIST_COLUMN_GAME)));
#endif
  qApp->processEvents();

  int gamePos = 0;
  QString s = "<description>" + item->text(QMC2_GAMELIST_COLUMN_GAME).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;") + "</description>";
  while ( !xmlLines[gamePos].contains(s) ) gamePos++;

  QTreeWidgetItem *childItem = item->takeChild(0);
  delete childItem;

  QString element, content;
  QStringList attributes, descriptions;

  // game/machine element
  attributes << "name" << "sourcefile" << "isbios" << "runnable" << "cloneof" << "romof" << "sampleof";
  descriptions << tr("Name") << tr("Source file") << tr("Is BIOS?") << tr("Runnable") << tr("Clone of") << tr("ROM of") << tr("Sample of");
  element = xmlLines.at(gamePos - 1).simplified();
  insertAttributeItems(item, element, attributes, descriptions, TRUE);

#if defined(QMC2_EMUTYPE_MAME)
  while ( !xmlLines[gamePos].contains("</game>") ) {
#elif defined(QMC2_EMUTYPE_MESS)
  while ( !xmlLines[gamePos].contains("</machine>") ) {
#endif
    element = xmlLines[gamePos].simplified();
    if ( element.contains("<year>") ) {
      content = element.remove("<year>").remove("</year>");
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Year"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, content);
    }
    if ( element.contains("<manufacturer>") ) {
      content = element.remove("<manufacturer>").remove("</manufacturer>");
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Manufacturer"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, content);
    }
    if ( element.contains("<rom ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("ROM"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "bios" << "size" << "crc" << "sha1" << "merge" << "region" << "offset" << "status" << "dispose";
      descriptions.clear();
      descriptions << tr("BIOS") << tr("Size") << tr("CRC") << tr("SHA1") << tr("Merge") << tr("Region") << tr("Offset") << tr("Status") << tr("Dispose");
      insertAttributeItems(childItem, element, attributes, descriptions, TRUE);
    }
    if ( element.contains("<chip ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Chip"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "tag" << "type" << "clock";
      descriptions.clear();
      descriptions << tr("Tag") << tr("Type") << tr("Clock");
      insertAttributeItems(childItem, element, attributes, descriptions, TRUE);
    }
    if ( element.contains("<display ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Display"));
      attributes.clear();
      attributes << "type" << "rotate" << "flipx" << "width" << "height" << "refresh" << "pixclock" << "htotal" << "hbend" << "hbstart" << "vtotal" << "vbend" << "vbstart";
      descriptions.clear();
      descriptions << tr("Type") << tr("Rotate") << tr("Flip-X") << tr("Width") << tr("Height") << tr("Refresh") << tr("Pixel clock") << tr("H-Total") << tr("H-Bend") << tr("HB-Start") << tr("V-Total") << tr("V-Bend") << tr("VB-Start");
      insertAttributeItems(childItem, element, attributes, descriptions, TRUE);
    }
    if ( element.contains("<sound ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Sound"));
      attributes.clear();
      attributes << "channels";
      descriptions.clear();
      descriptions << tr("Channels");
      insertAttributeItems(childItem, element, attributes, descriptions, TRUE);
    }
    if ( element.contains("<input ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Input"));
      attributes.clear();
      attributes << "service" << "tilt" << "players" << "buttons" << "coins";
      descriptions.clear();
      descriptions << tr("Service") << tr("Tilt") << tr("Players") << tr("Buttons") << tr("Coins");
      insertAttributeItems(childItem, element, attributes, descriptions, TRUE);

      gamePos++;
      while ( xmlLines[gamePos].contains("<control ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *nextChildItem = new QTreeWidgetItem(childItem);
        nextChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Control"));
        nextChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "type", TRUE));
        attributes.clear();
        attributes << "minimum" << "maximum" << "sensitivity" << "keydelta" << "reverse";
        descriptions.clear();
        descriptions << tr("Minimum") << tr("Maximum") << tr("Sensitivity") << tr("Key Delta") << tr("Reverse");
        insertAttributeItems(nextChildItem, subElement, attributes, descriptions, TRUE);
        gamePos++;
      }
    }
    if ( element.contains("<dipswitch ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("DIP switch"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name", TRUE));

      gamePos++;
      while ( xmlLines[gamePos].contains("<dipvalue ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("DIP value"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", TRUE));
        attributes.clear();
        attributes << "default";
        descriptions.clear();
        descriptions << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, TRUE);
        gamePos++;
      }
    }
    if ( element.contains("<driver ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Driver"));
      attributes.clear();
      attributes << "status" << "emulation" << "color" << "sound" << "graphic" << "cocktail" << "protection" << "savestate" << "palettesize";
      descriptions.clear();
      descriptions << tr("Status") << tr("Emulation") << tr("Color") << tr("Sound") << tr("Graphic") << tr("Cocktail") << tr("Protection") << tr("Save state") << tr("Palette size");
      insertAttributeItems(childItem, element, attributes, descriptions, TRUE);
    }
    if ( element.contains("<biosset ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("BIOS set"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "description" << "default";
      descriptions.clear();
      descriptions << tr("Description") << tr("Default");
      insertAttributeItems(childItem, element, attributes, descriptions, TRUE);
    }
    if ( element.contains("<sample ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Sample"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
    }
    if ( element.contains("<disk ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Disk"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "md5" << "sha1" << "merge" << "region" << "index" << "status";
      descriptions.clear();
      descriptions << tr("MD5") << tr("SHA1") << tr("Merge") << tr("Region") << tr("Index") << tr("Status");
      insertAttributeItems(childItem, element, attributes, descriptions, TRUE);
    }
    if ( element.contains("<device ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Device"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "type", TRUE));
      attributes.clear();
      attributes << "tag" << "mandatory";
      descriptions.clear();
      descriptions << tr("Tag") << tr("Mandatory");
      insertAttributeItems(childItem, element, attributes, descriptions, FALSE);

      gamePos++;
      while ( xmlLines[gamePos].contains("<instance ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Instance"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", FALSE));
        attributes.clear();
        attributes << "briefname";
        descriptions.clear();
        descriptions << tr("Brief name");
        insertAttributeItems(secondChildItem, element, attributes, descriptions, FALSE);
        gamePos++;
      }
      while ( xmlLines[gamePos].contains("<extension ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Extension"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", FALSE));
        gamePos++;
      }
    }
    if ( element.contains("<ramoption") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("RAM options"));
      while ( xmlLines[gamePos].contains("<ramoption") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Option"));
        int fromIndex = subElement.indexOf('>') + 1;
        int toIndex = subElement.indexOf('<', fromIndex);
        QString ramOptionValue = subElement.mid(fromIndex, toIndex - fromIndex);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, ramOptionValue);
        attributes.clear();
        attributes << "default";
        descriptions.clear();
        descriptions << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, FALSE);
        gamePos++;
      }
      if ( xmlLines[gamePos].contains("</machine>") )
        gamePos--;
    }
    gamePos++;
  }
}

void Gamelist::parse()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::parse()");
#endif

  if ( qmc2StopParser ) {
    qmc2ReloadActive = FALSE;
    enableWidgets(TRUE);
    return;
  }

  QTime elapsedTime;
  autoROMCheck = FALSE;
  qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
#if defined(QMC2_EMUTYPE_MAME)
  romCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/ROMStateCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  romCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/ROMStateCacheFile").toString());
#endif
  romCache.open(QIODevice::ReadOnly | QIODevice::Text);
  if ( !romCache.isOpen() ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open ROM state cache, please check ROMs"));
  } else {
    parseTimer.start();
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading ROM state from cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("ROM states - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    qmc2MainWindow->progressBarGamelist->reset();
    qApp->processEvents();
    tsRomCache.setDevice(&romCache);
    tsRomCache.reset();
    int cachedGamesCounter = 0;
    while ( !tsRomCache.atEnd() ) {
      QString line = tsRomCache.readLine();
      if ( !line.isNull() && !line.startsWith("#") ) {
        QStringList words = line.split(" ");
        qmc2GamelistStatusMap[words[0]] = words[1];
        cachedGamesCounter++;
      }
      if ( cachedGamesCounter % QMC2_ROMCACHE_RESPONSIVENESS == 0 ) {
        qmc2MainWindow->progressBarGamelist->setValue(cachedGamesCounter);
        qApp->processEvents();
      }
    }
    numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numNotFoundGames = 0;
    elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading ROM state from cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n cached ROM state(s) loaded", "", cachedGamesCounter));
    if ( numTotalGames != cachedGamesCounter ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ROM state cache is incomplete or not up to date, please re-check ROMs"));
      autoROMCheck = TRUE;
    }
    romCache.close();
    qApp->processEvents();
  }

  QTime processGamelistElapsedTimer;
  parseTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("processing game list"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("processing machine list"));
#endif
  qmc2MainWindow->treeWidgetGamelist->clear();
  qmc2HierarchyMap.clear();
  qmc2ParentMap.clear();
  qmc2MainWindow->progressBarGamelist->reset();

#if defined(QMC2_EMUTYPE_MAME)
  gamelistCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  gamelistCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile").toString());
#endif
  gamelistCache.open(QIODevice::ReadOnly | QIODevice::Text);
  bool reparseGamelist = TRUE;

  if ( gamelistCache.isOpen() ) {
    QString line;
    tsGamelistCache.setDevice(&gamelistCache);
    tsGamelistCache.seek(0);
    
    if ( !tsGamelistCache.atEnd() ) {
      line = tsGamelistCache.readLine();
      while ( line.startsWith("#") && !tsGamelistCache.atEnd() )
        line = tsGamelistCache.readLine();
      QStringList words = line.split("\t");
      if ( words.count() >= 2 ) {
#if defined(QMC2_EMUTYPE_MAME)
        if ( words[0] == "MAME_VERSION" ) {
#elif defined(QMC2_EMUTYPE_MESS)
        if ( words[0] == "MESS_VERSION" ) {
#endif
          reparseGamelist = (words[1] != emulatorVersion);
        }
      } else {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator version of game list cache"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator version of machine list cache"));
#endif
      }
      if ( words.count() < 4 ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the game list cache will now be updated due to a new format"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the machine list cache will now be updated due to a new format"));
#endif
	reparseGamelist = TRUE;
      }
    }

#if defined(QMC2_EMUTYPE_MAME)
    bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
#endif

    if ( !reparseGamelist && !qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading game data from game list cache"));
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("Game data - %p%"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading machine data from machine list cache"));
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("Machine data - %p%"));
#endif
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      QTime gameDataCacheElapsedTime;
      miscTimer.start();
      numGames = numUnknownGames = 0;
      qmc2MainWindow->progressBarGamelist->reset();
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(FALSE);
      QString readBuffer;
      while ( (!tsGamelistCache.atEnd() || !readBuffer.isEmpty() ) && !qmc2StopParser ) {
        readBuffer += tsGamelistCache.read(QMC2_FILE_BUFFER_SIZE);
        bool endsWithNewLine = readBuffer.endsWith("\n");
        QStringList lines = readBuffer.split("\n");
        int l, lc = lines.count();
        if ( !endsWithNewLine )
          lc -= 1;
        for (l = 0; l < lc; l++) {
          line = lines[l];
          if ( !line.isEmpty() && !line.startsWith("#") ) {
            QStringList words = line.split("\t");
            QString gameName = words[0];
            QString gameDescription = words[1];
            QString gameManufacturer = words[2];
            QString gameYear = words[3];
            QString gameCloneOf = words[4];
            bool isBIOS = (words[5] == "1");
            bool hasROMs = (words[6] == "1");
            bool hasCHDs = (words[7] == "1");

#ifdef QMC2_DEBUG
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: gameName = %1, gameDescription = %2, gameManufacturer = %3, gameYear = %4, gameCloneOf = %5, isBIOS = %6, hasROMs = %7, hasCHDs = %8").
                            arg(gameName).arg(gameDescription).arg(gameManufacturer).arg(gameYear).arg(gameCloneOf).arg(isBIOS).arg(hasROMs).arg(hasCHDs));
#endif

            GamelistItem *gameDescriptionItem = new GamelistItem(qmc2MainWindow->treeWidgetGamelist);

            if ( !gameCloneOf.isEmpty() )
              qmc2HierarchyMap[gameCloneOf].append(gameName);
            else if ( !qmc2HierarchyMap.contains(gameName) )
              qmc2HierarchyMap.insert(gameName, QStringList());

            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_GAME, gameDescription);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_YEAR, gameYear);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_MANU, gameManufacturer);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_NAME, gameName);
	    if ( hasROMs && hasCHDs )
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("ROM, CHD"));
            else if ( hasROMs )
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("ROM"));
            else if ( hasCHDs )
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("CHD"));
#if defined(QMC2_EMUTYPE_MAME)
            if ( useCatverIni ) {
              QString categoryString = qmc2CategoryMap[gameName];
              QString versionString = qmc2VersionMap[gameName];
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, categoryString.isEmpty() ? tr("Unknown") : categoryString);
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_VERSION, versionString.isEmpty() ? tr("?") : versionString);
            }
#endif
            switch ( qmc2GamelistStatusMap[gameName][0].toAscii() ) {
              case 'C': 
                numCorrectGames++;
                if ( isBIOS ) {
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
                  qmc2BiosROMs << gameName;
                } else
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
                break;

              case 'M': 
                numMostlyCorrectGames++;
                if ( isBIOS ) {
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
                  qmc2BiosROMs << gameName;
                } else
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
                break;

              case 'I':
                numIncorrectGames++;
                if ( isBIOS ) {
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
                  qmc2BiosROMs << gameName;
                } else
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
                break;

              case 'N':
                numNotFoundGames++;
                if ( isBIOS ) {
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
                  qmc2BiosROMs << gameName;
                } else
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
                break;

              default:
                numUnknownGames++;
                qmc2GamelistStatusMap[gameName] = "U";
                if ( isBIOS ) {
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
                  qmc2BiosROMs << gameName;
                } else
                  gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
                break;
            }

            QTreeWidgetItem *nameItem = new QTreeWidgetItem(gameDescriptionItem);
            nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
            nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, gameName);
            qmc2GamelistItemMap[gameName] = gameDescriptionItem;
            qmc2GamelistItemByDescriptionMap[gameDescription] = gameDescriptionItem;
            qmc2GamelistDescriptionMap[gameName] = gameDescription;
            qmc2GamelistNameMap[gameDescription] = gameName;

            loadIcon(gameName, gameDescriptionItem);

            numGames++;
          }

          if ( numGames % qmc2GamelistResponsiveness == 0 ) {
            qmc2MainWindow->progressBarGamelist->setValue(numGames);
            qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);
            qmc2MainWindow->labelGamelistStatus->setText(status());
            if ( qmc2Options->config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortOnline").toBool() )
              qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
            qApp->processEvents();
            qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(FALSE);
          }
        }

        if ( endsWithNewLine )
          readBuffer.clear();
        else
          readBuffer = lines.last();
      }
      qmc2MainWindow->progressBarGamelist->setValue(numGames);
      qApp->processEvents();

      gameDataCacheElapsedTime = gameDataCacheElapsedTime.addMSecs(miscTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading game data from game list cache, elapsed time = %1)").arg(gameDataCacheElapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading machine data from machine list cache, elapsed time = %1)").arg(gameDataCacheElapsedTime.toString("mm:ss.zzz")));
#endif
    }
  } 
  if ( gamelistCache.isOpen() )
    gamelistCache.close();

  xmlLines.clear();
#if defined(QMC2_EMUTYPE_MAME)
  xmlLines = gamelistBuffer.remove(0, gamelistBuffer.indexOf("<mame build")).split("\n");
#elif defined(QMC2_EMUTYPE_MESS)
  xmlLines = gamelistBuffer.remove(0, gamelistBuffer.indexOf("<mess build")).split("\n");
#endif
  gamelistBuffer.clear();

  if ( reparseGamelist && !qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("parsing game data and (re)creating game list cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Game data - %p%"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("parsing machine data and (re)creating machine list cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Machine data - %p%"));
#endif
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    gamelistCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !gamelistCache.isOpen() ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open game list cache for writing, path = %1").arg(gamelistCache.fileName()));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open machine list cache for writing, path = %1").arg(gamelistCache.fileName()));
#endif
    } else {
      tsGamelistCache.setDevice(&gamelistCache);
      tsGamelistCache.reset();
      QString glcVersion("2");
      tsGamelistCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
#if defined(QMC2_EMUTYPE_MAME)
      tsGamelistCache << "MAME_VERSION\t" + emulatorVersion + "\tGLC_VERSION\t" + glcVersion + "\n";
#elif defined(QMC2_EMUTYPE_MESS)
      tsGamelistCache << "MESS_VERSION\t" + emulatorVersion + "\tGLC_VERSION\t" + glcVersion + "\n";
#endif
    }

    // parse XML gamelist data
    int lineCounter;
    numGames = numUnknownGames = 0;
    bool endParser = qmc2StopParser;
    qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(FALSE);

    for (lineCounter = 0; xmlLines.count() && !endParser; lineCounter++) {
      while ( !endParser && !xmlLines[lineCounter].contains("<description>") ) {
        lineCounter++;
#if defined(QMC2_EMUTYPE_MAME)
        endParser = xmlLines[lineCounter].contains("</mame>") || qmc2StopParser;
#elif defined(QMC2_EMUTYPE_MESS)
        endParser = xmlLines[lineCounter].contains("</mess>") || qmc2StopParser;
#endif
      }
      if ( !endParser ) {
        QString descriptionElement = xmlLines[lineCounter].simplified();
        QString gameElement = xmlLines[lineCounter - 1].simplified();
        QString runnableCheck = value(gameElement, "runnable");
        QString isbiosCheck = value(gameElement, "isbios");
        bool isBIOS = ( runnableCheck == "no" || isbiosCheck == "yes" );
        QString gameName = value(gameElement, "name");
        QString gameCloneOf = value(gameElement, "cloneof");
        QString gameDescription = descriptionElement.remove("<description>").remove("</description>").replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">");
        GamelistItem *gameDescriptionItem = new GamelistItem(qmc2MainWindow->treeWidgetGamelist);

#if defined(QMC2_EMUTYPE_MAME)
        bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
#endif

        // find year & manufacturer and determine ROM/CHD requirements
        bool endGame = FALSE;
        int i = lineCounter;
        QString gameYear = "?", gameManufacturer = "?";
        bool yearFound = FALSE, manufacturerFound = FALSE, hasROMs = FALSE, hasCHDs = FALSE;
        while ( !endGame ) {
          if ( xmlLines[i].contains("<year>") ) {
            gameYear = xmlLines[i].simplified().remove("<year>").remove("</year>");
            yearFound = TRUE;
          } else if ( xmlLines[i].contains("<manufacturer>") ) {
            gameManufacturer = xmlLines[i].simplified().remove("<manufacturer>").remove("</manufacturer>").replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">");
            manufacturerFound = TRUE;
          } else if ( xmlLines[i].contains("<rom name") ) {
            hasROMs = TRUE;
	  } else if ( xmlLines[i].contains("<disk name") ) {
            hasCHDs = TRUE;
	  }
#if defined(QMC2_EMUTYPE_MAME)
          endGame = xmlLines[i].contains("</game>") || ( yearFound && manufacturerFound && hasROMs && hasCHDs );
#elif defined(QMC2_EMUTYPE_MESS)
          endGame = xmlLines[i].contains("</machine>") || ( yearFound && manufacturerFound && hasROMs && hasCHDs );
#endif
          i++;
        }

        if ( !gameCloneOf.isEmpty() )
          qmc2HierarchyMap[gameCloneOf].append(gameName);
        else if ( !qmc2HierarchyMap.contains(gameName) )
          qmc2HierarchyMap.insert(gameName, QStringList());

        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_GAME, gameDescription);
        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_YEAR, gameYear);
        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_MANU, gameManufacturer);
        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_NAME, gameName);
	if ( hasROMs && hasCHDs )
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("ROM, CHD"));
        else if ( hasCHDs )
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("CHD"));
        else
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("ROM"));
#if defined(QMC2_EMUTYPE_MAME)
        if ( useCatverIni ) {
          QString categoryString = qmc2CategoryMap[gameName];
          QString versionString = qmc2VersionMap[gameName];
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, categoryString.isEmpty() ? tr("Unknown") : categoryString);
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_VERSION, versionString.isEmpty() ? tr("?") : versionString);
        }
#endif
        switch ( qmc2GamelistStatusMap[gameName][0].toAscii() ) {
          case 'C': 
            numCorrectGames++;
            if ( isBIOS ) {
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
              qmc2BiosROMs << gameName;
            } else
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
            break;

          case 'M': 
            numMostlyCorrectGames++;
            if ( isBIOS ) {
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
              qmc2BiosROMs << gameName;
            } else
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
            break;

          case 'I':
            numIncorrectGames++;
            if ( isBIOS ) {
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
              qmc2BiosROMs << gameName;
            } else
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
            break;

          case 'N':
            numNotFoundGames++;
            if ( isBIOS ) {
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
              qmc2BiosROMs << gameName;
            } else
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
            break;

          default:
            numUnknownGames++;
            qmc2GamelistStatusMap[gameName] = "U";
            if ( isBIOS ) {
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
              qmc2BiosROMs << gameName;
            } else
              gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
            break;
        }

        QTreeWidgetItem *nameItem = new QTreeWidgetItem(gameDescriptionItem);
        nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
        nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, gameName);
        qmc2GamelistItemMap[gameName] = gameDescriptionItem;
        qmc2GamelistItemByDescriptionMap[gameDescription] = gameDescriptionItem;
        qmc2GamelistDescriptionMap[gameName] = gameDescription;
        qmc2GamelistNameMap[gameDescription] = gameName;

        loadIcon(gameName, gameDescriptionItem);

        if ( gamelistCache.isOpen() )
          tsGamelistCache << gameName << "\t" << gameDescription << "\t" << gameManufacturer << "\t"
                          << gameYear << "\t" << gameCloneOf << "\t" << isBIOS << "\t"
			  << (hasROMs ? "1" : "0") << "\t" << (hasCHDs ? "1": "0") << "\n";

        numGames++;
      }

      qmc2MainWindow->progressBarGamelist->setValue(numGames);

      if ( numGames % qmc2GamelistResponsiveness == 0 ) {
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);
        qmc2MainWindow->labelGamelistStatus->setText(status());
        if ( qmc2Options->config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortOnline").toBool() )
          qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
        qApp->processEvents();
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(FALSE);
      }
    }
  }
  if ( gamelistCache.isOpen() )
    gamelistCache.close();

#if defined(QMC2_EMUTYPE_MAME)
  bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
#endif

  // create parent/clone hierarchy tree
  qmc2MainWindow->treeWidgetHierarchy->clear();
  QMapIterator<QString, QStringList> i(qmc2HierarchyMap);
  while ( i.hasNext() ) {
    i.next();
    QString iValue = i.key();
    QString iDescription = qmc2GamelistDescriptionMap[iValue];
    if ( iDescription.isEmpty() )
      continue;
    GamelistItem *hierarchyItem = new GamelistItem(qmc2MainWindow->treeWidgetHierarchy);
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_GAME, iDescription);
    QTreeWidgetItem *baseItem = qmc2GamelistItemMap[iValue];
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
#if defined(QMC2_EMUTYPE_MAME)
    if ( useCatverIni ) {
      hierarchyItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
      hierarchyItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
    }
#endif
    qmc2HierarchyItemMap[iValue] = hierarchyItem;
    switch ( qmc2GamelistStatusMap[iValue][0].toAscii() ) {
      case 'C': 
        if ( qmc2BiosROMs.contains(iValue) )
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
        else
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
        break;

      case 'M': 
        if ( qmc2BiosROMs.contains(iValue) )
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
        else
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
        break;

      case 'I':
        if ( qmc2BiosROMs.contains(iValue) )
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
        else
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
        break;

      case 'N':
        if ( qmc2BiosROMs.contains(iValue) )
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
        else
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
        break;

      default:
        if ( qmc2BiosROMs.contains(iValue) )
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
        else
          hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
        break;
    }

    loadIcon(iValue, hierarchyItem);

    int j;
    for (j = 0; j < i.value().count(); j++) {
      QString jValue = i.value().at(j);
      QString jDescription = qmc2GamelistDescriptionMap[jValue];
      if ( jDescription.isEmpty() )
        continue;
      GamelistItem *hierarchySubItem = new GamelistItem(hierarchyItem);
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_GAME, jDescription);
      baseItem = qmc2GamelistItemMap[jValue];
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
#if defined(QMC2_EMUTYPE_MAME)
      if ( useCatverIni ) {
        hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
        hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
      }
#endif
      qmc2HierarchyItemMap[jValue] = hierarchySubItem;
      qmc2ParentMap[jValue] = iValue;
#if defined(QMC2_EMUTYPE_MAME)
      // "fill up" emulator info data for clones
      if ( !qmc2EmuInfoDB.isEmpty() ) {
        QByteArray *p = qmc2EmuInfoDB[hierarchyItem->text(QMC2_GAMELIST_COLUMN_NAME)];
        if ( p )
          if ( !qmc2EmuInfoDB.contains(baseItem->text(QMC2_GAMELIST_COLUMN_NAME)) )
            qmc2EmuInfoDB[baseItem->text(QMC2_GAMELIST_COLUMN_NAME)] = p;
      }
#endif
      switch ( qmc2GamelistStatusMap[jValue][0].toAscii() ) {
        case 'C': 
          if ( qmc2BiosROMs.contains(jValue) )
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
          else
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
          break;

        case 'M': 
          if ( qmc2BiosROMs.contains(jValue) )
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
          else
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
          break;

        case 'I':
          if ( qmc2BiosROMs.contains(jValue) )
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
          else
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
          break;

        case 'N':
          if ( qmc2BiosROMs.contains(jValue) )
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
          else
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
          break;

        default:
          if ( qmc2BiosROMs.contains(jValue) )
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
          else
            hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
          break;
      }

      loadIcon(jValue, hierarchySubItem);
    }
  }

  QString sortCriteria = "?";
  switch ( qmc2SortCriteria ) {
    case QMC2_SORT_BY_DESCRIPTION:
#if defined(QMC2_EMUTYPE_MAME)
      sortCriteria = QObject::tr("game description");
#elif defined(QMC2_EMUTYPE_MESS)
      sortCriteria = QObject::tr("machine description");
#endif
      break;
    case QMC2_SORT_BY_ROM_STATE:
      sortCriteria = QObject::tr("ROM state");
      break;
    case QMC2_SORT_BY_YEAR:
      sortCriteria = QObject::tr("year");
      break;
    case QMC2_SORT_BY_MANUFACTURER:
      sortCriteria = QObject::tr("manufacturer");
      break;
    case QMC2_SORT_BY_NAME:
#if defined(QMC2_EMUTYPE_MAME)
      sortCriteria = QObject::tr("game name");
#elif defined(QMC2_EMUTYPE_MESS)
      sortCriteria = QObject::tr("machine name");
#endif
    case QMC2_SORT_BY_ROMTYPES:
      sortCriteria = QObject::tr("ROM types");
      break;
#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_SORT_BY_CATEGORY:
      sortCriteria = QObject::tr("category");
      break;
    case QMC2_SORT_BY_VERSION:
      sortCriteria = QObject::tr("version");
      break;
#endif
  }
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting game list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#endif
  qApp->processEvents();
  QList<QTreeWidgetItem *> itemList = qmc2MainWindow->treeWidgetGamelist->findItems("*", Qt::MatchContains | Qt::MatchWildcard);
  for (int i = 0; i < itemList.count(); i++) {
    if ( itemList[i]->childCount() > 1 ) {
      qmc2MainWindow->treeWidgetGamelist->collapseItem(itemList[i]);
      QList<QTreeWidgetItem *> childrenList = itemList[i]->takeChildren();
      int j;
      for (j = 0; j < childrenList.count(); j++)
        delete childrenList[j];
      QTreeWidgetItem *nameItem = new QTreeWidgetItem(itemList[i]);
      nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
      nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, qmc2GamelistNameMap[itemList[i]->text(QMC2_GAMELIST_COLUMN_GAME)]);
      qApp->processEvents();
    }
  }
  qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
  qmc2MainWindow->treeWidgetHierarchy->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
  qApp->processEvents();
  QTreeWidgetItem *ci = qmc2MainWindow->treeWidgetGamelist->currentItem();
  if ( ci ) {
    if ( ci->isSelected() ) {
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
    } else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection").toBool() ) {
#if defined(QMC2_EMUTYPE_MAME)
      QTreeWidgetItem *glItem = qmc2GamelistItemByDescriptionMap[qmc2Config->value("MAME/SelectedGame").toString()];
#elif defined(QMC2_EMUTYPE_MESS)
      QTreeWidgetItem *glItem = qmc2GamelistItemByDescriptionMap[qmc2Config->value("MESS/SelectedGame").toString()];
#endif
      if ( glItem ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring game selection"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring machine selection"));
#endif
        qmc2MainWindow->treeWidgetGamelist->setCurrentItem(glItem);
        QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
      }
    }
  } else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection").toBool() ) {
#if defined(QMC2_EMUTYPE_MAME)
    QTreeWidgetItem *glItem = qmc2GamelistItemByDescriptionMap[qmc2Config->value("MAME/SelectedGame").toString()];
#elif defined(QMC2_EMUTYPE_MESS)
    QTreeWidgetItem *glItem = qmc2GamelistItemByDescriptionMap[qmc2Config->value("MESS/SelectedGame").toString()];
#endif
    if ( glItem ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring game selection"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring machine selection"));
#endif
      qmc2MainWindow->treeWidgetGamelist->setCurrentItem(glItem);
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
    }
  }
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);
  qmc2MainWindow->labelGamelistStatus->setText(status());

  processGamelistElapsedTimer = processGamelistElapsedTimer.addMSecs(parseTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (processing game list, elapsed time = %1)").arg(processGamelistElapsedTimer.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n game(s) loaded", "", numGames));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (processing machine list, elapsed time = %1)").arg(processGamelistElapsedTimer.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n machine(s) loaded", "", numGames));
#endif
  if ( numGames != numTotalGames ) {
    if ( reparseGamelist && qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list not fully parsed, invalidating game list cache"));
      QFile f(qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list not fully parsed, invalidating machine list cache"));
      QFile f(qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile").toString());
#endif
      f.remove();
    } else if ( !qmc2StopParser) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list cache is out of date, invalidating game list cache"));
      QFile f(qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list cache is out of date, invalidating machine list cache"));
      QFile f(qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile").toString());
#endif
      f.remove();
    }
  }
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state info: C:%1 M:%2 I:%3 N:%4 U:%5").arg(numCorrectGames).arg(numMostlyCorrectGames).arg(numIncorrectGames).arg(numNotFoundGames).arg(numUnknownGames));
  qmc2MainWindow->progressBarGamelist->reset();

  if ( qmc2StopParser && loadProc )
    loadProc->terminate();
  qmc2ReloadActive = FALSE;
  qmc2StartingUp = FALSE;
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck").toBool() ) {
    if ( autoROMCheck )
      QTimer::singleShot(QMC2_AUTOROMCHECK_DELAY, qmc2MainWindow->actionCheckROMs, SLOT(trigger()));
    else
      filter();
  } else
    filter();
  enableWidgets(TRUE);
}

void Gamelist::filter()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::filter()");
#endif

  if ( qmc2FilterActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state filter already active"));
    return;
  }

  if ( qmc2VerifyActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
    return;
  }

  if ( qmc2ReloadActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return;
  }

  QTime elapsedTime;
  qmc2StopParser = FALSE;
  parseTimer.start();
  qmc2FilterActive = TRUE;
  enableWidgets(FALSE);
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("applying ROM state filter"));
  qmc2MainWindow->progressBarGamelist->reset();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(tr("State filter - %p%"));
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");

  qmc2MainWindow->progressBarGamelist->setRange(0, numGames - 1);

  bool showC = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC").toBool();
  bool showM = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM").toBool();
  bool showI = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI").toBool();
  bool showN = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN").toBool();
  bool showU = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU").toBool();

  QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
  int i = 0;
  int filterResponse = numGames / QMC2_STATEFILTER_UPDATES;
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(FALSE);
  while ( it.hasNext() && !qmc2StopParser ) {
    if ( i++ % filterResponse == 0 ) {
      qmc2MainWindow->progressBarGamelist->setValue(i);
      qApp->processEvents();
    }
    it.next();
    QTreeWidgetItem *item = it.value();
    if ( item ) {
      switch ( item->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() ) {
        case QMC2_ROMSTATE_CHAR_C:
          item->setHidden(!showC);
          break;

        case QMC2_ROMSTATE_CHAR_M:
          item->setHidden(!showM);
          break;

        case QMC2_ROMSTATE_CHAR_I:
          item->setHidden(!showI);
          break;

        case QMC2_ROMSTATE_CHAR_N:
          item->setHidden(!showN);
          break;

        case QMC2_ROMSTATE_CHAR_U:
        default:
          item->setHidden(!showU);
          break;
      }
    }
  }
  qmc2MainWindow->progressBarGamelist->setValue(numGames - 1);
  qApp->processEvents();
  qmc2MainWindow->scrollToCurrentItem();
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);
  qmc2FilterActive = FALSE;
  elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (applying ROM state filter, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  enableWidgets(TRUE);
  qmc2StatesTogglesEnabled = TRUE;
  QTimer::singleShot(0, qmc2MainWindow->progressBarGamelist, SLOT(reset()));
}

void Gamelist::save()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::save()");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving game list"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving game list)"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving machine list"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving machine list)"));
#endif
}

void Gamelist::loadFavorites()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadFavorites()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading favorites"));

  qmc2MainWindow->listWidgetFavorites->clear();
#if defined(QMC2_EMUTYPE_MAME)
  QFile f(qmc2Config->value("MAME/FilesAndDirectories/FavoritesFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  QFile f(qmc2Config->value("MESS/FilesAndDirectories/FavoritesFile").toString());
#endif
  if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    while ( !ts.atEnd() ) {
      QString gameName = ts.readLine();
      if ( !gameName.isEmpty() ) {
        QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
        if ( gameItem ) {
          QListWidgetItem *item = new QListWidgetItem(qmc2MainWindow->listWidgetFavorites);
          item->setText(gameItem->text(QMC2_GAMELIST_COLUMN_GAME));
        }
      }
    }
    f.close();
  }

  qmc2MainWindow->listWidgetFavorites->sortItems();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading favorites)"));
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_FAVORITES_INDEX )
    QTimer::singleShot(0, qmc2MainWindow, SLOT(checkCurrentFavoritesSelection()));
  else
    qmc2MainWindow->listWidgetFavorites->setCurrentIndex(QModelIndex());
}

void Gamelist::saveFavorites()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::saveFavorites()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving favorites"));

#if defined(QMC2_EMUTYPE_MAME)
  QFile f(qmc2Config->value("MAME/FilesAndDirectories/FavoritesFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  QFile f(qmc2Config->value("MESS/FilesAndDirectories/FavoritesFile").toString());
#endif
  if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    int i;
    for (i = 0; i < qmc2MainWindow->listWidgetFavorites->count(); i++) {
      ts << qmc2GamelistNameMap[qmc2MainWindow->listWidgetFavorites->item(i)->text()] << "\n";
    }
    f.close();
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open favorites file for writing, path = %1").arg(qmc2Config->value("MAME/FilesAndDirectories/FavoritesFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open favorites file for writing, path = %1").arg(qmc2Config->value("MESS/FilesAndDirectories/FavoritesFile").toString()));
#endif
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving favorites)"));
}

void Gamelist::loadPlayHistory()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadPlayHistory()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading play history"));

  qmc2MainWindow->listWidgetPlayed->clear();
#if defined(QMC2_EMUTYPE_MAME)
  QFile f(qmc2Config->value("MAME/FilesAndDirectories/HistoryFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  QFile f(qmc2Config->value("MESS/FilesAndDirectories/HistoryFile").toString());
#endif
  if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    while ( !ts.atEnd() ) {
      QString gameName = ts.readLine();
      if ( !gameName.isEmpty() ) {
        QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
        if ( gameItem ) {
          QListWidgetItem *item = new QListWidgetItem(qmc2MainWindow->listWidgetPlayed);
          item->setText(gameItem->text(QMC2_GAMELIST_COLUMN_GAME));
        }
      }
    }
    f.close();
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading play history)"));
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_PLAYED_INDEX )
    QTimer::singleShot(0, qmc2MainWindow, SLOT(checkCurrentPlayedSelection()));
  else
    qmc2MainWindow->listWidgetPlayed->setCurrentIndex(QModelIndex());
}

void Gamelist::savePlayHistory()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::savePlayHistory()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving play history"));

#if defined(QMC2_EMUTYPE_MAME)
  QFile f(qmc2Config->value("MAME/FilesAndDirectories/HistoryFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  QFile f(qmc2Config->value("MESS/FilesAndDirectories/HistoryFile").toString());
#endif
  if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    int i;
    for (i = 0; i < qmc2MainWindow->listWidgetPlayed->count(); i++) {
      ts << qmc2GamelistNameMap[qmc2MainWindow->listWidgetPlayed->item(i)->text()] << "\n";
    }
    f.close();
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open play history file for writing, path = %1").arg(qmc2Config->value("MAME/FilesAndDirectories/HistoryFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open play history file for writing, path = %1").arg(qmc2Config->value("MESS/FilesAndDirectories/HistoryFile").toString()));
#endif
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving play history)"));
}

QString Gamelist::status()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::status()");
#endif

  QString statusString = "<b>";
  statusString += "<font color=black>" + tr("L:") + QString(numGames > -1 ? QString::number(numGames) : "?") + "</font>\n";
  statusString += "<font color=#00cc00>" + tr("C:") + QString(numCorrectGames > -1 ? QString::number(numCorrectGames) : "?") + "</font>\n";
  statusString += "<font color=#a2c743>" + tr("M:") + QString(numMostlyCorrectGames > -1 ? QString::number(numMostlyCorrectGames) : "?") + "</font>\n";
  statusString += "<font color=#f90000>" + tr("I:") + QString(numIncorrectGames > -1 ? QString::number(numIncorrectGames) : "?") + "</font>\n";
  statusString += "<font color=#7f7f7f>" + tr("N:") + QString(numNotFoundGames > -1 ? QString::number(numNotFoundGames) : "?") + "</font>\n";
  statusString += "<font color=#0000f9>" + tr("U:") + QString(numUnknownGames > -1 ? QString::number(numUnknownGames) : "?") + "</font>\n";
  statusString += "<font color=chocolate>" + tr("S:") + QString(numSearchGames > -1 ? QString::number(numSearchGames) : "?") + "</font>";
  statusString += "</b>";

  return statusString;
}

void Gamelist::loadStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadStarted()");
#endif

  qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
  qmc2MainWindow->progressBarGamelist->reset();
}

void Gamelist::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadFinished(int exitCode = " + QString::number(exitCode) + ", QProcess::ExitStatus exitStatus = " + QString::number(exitStatus) + "): proc = 0x" + QString::number((ulong)proc, 16));
#endif

  if ( exitStatus != QProcess::NormalExit && !qmc2StopParser )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));

  QTime elapsedTime;
  elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML game list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_SDLMAME) || defined(QMC2_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML machine list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  qmc2MainWindow->progressBarGamelist->reset();
  qmc2EarlyReloadActive = FALSE;
  if ( loadProc )
    delete loadProc;
  loadProc = NULL;

  if ( romCache.isOpen() )
    romCache.close();

  if ( listXMLCache.isOpen() )
    listXMLCache.close();

  parse();
  loadFavorites();
  loadPlayHistory();

  // show game list
  qmc2MainWindow->labelLoadingGamelist->setVisible(FALSE);
  qmc2MainWindow->treeWidgetGamelist->setVisible(TRUE);

  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_GAMELIST_INDEX ) {
	  switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
		  case QMC2_VIEW_DETAIL_INDEX:
			  qmc2MainWindow->treeWidgetGamelist->setFocus();
		      	  break;
    		  case QMC2_VIEW_TREE_INDEX:
			  qmc2MainWindow->treeWidgetHierarchy->setFocus();
			  break;
#if defined(QMC2_EMUTYPE_MAME)
    		  case QMC2_VIEW_CATEGORY_INDEX:
			  qmc2MainWindow->treeWidgetCategoryView->setFocus();
    			  break;
    		  case QMC2_VIEW_VERSION_INDEX:
    			  qmc2MainWindow->treeWidgetVersionView->setFocus();
    			  break;
#endif
    		  default:
    			  qmc2MainWindow->treeWidgetGamelist->setFocus();
    			  break;
    	  }
  }

  qApp->processEvents();
}

void Gamelist::loadReadyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadReadyReadStandardOutput(): proc = 0x" + QString::number((ulong)proc, 16));
#endif

  QString s = proc->readAllStandardOutput();
  bool endsWithSpace = s.endsWith(" ");
  bool startWithSpace = s.startsWith(" ");

  s = s.simplified();
  if ( startWithSpace )
    s.prepend(" ");

  // ensure XML elements are on individual lines
  int i;
  for (i = 0; i < s.length(); i++) {
    if ( s[i] == '>' )
      if ( i + 1 < s.length() ) {
        if ( s[i + 1] == '<' )
          s.insert(i + 1, "\n");
        else if ( s[i + 1] == ' ' )
          if ( i + 2 < s.length() )
            if ( s[i + 2] == '<' )
              s.replace(i + 1, 1, "\n");
      }
  }

  QStringList sl = s.split("\n");
  int l, lc = sl.count();
  for (l = 0; l < lc; l++) {
    QString singleXMLLine = sl[l];
    if ( !singleXMLLine.startsWith("<!") && !singleXMLLine.startsWith("<?") && !singleXMLLine.startsWith("]>") ) {
      bool newLine = singleXMLLine.endsWith(">");
      if ( newLine ) {
        if ( singleXMLLine.endsWith("<description>") )
          newLine = FALSE;
        else if ( singleXMLLine.endsWith("<year>") )
          newLine = FALSE;
        else if ( singleXMLLine.endsWith("<manufacturer>") )
          newLine = FALSE;
        if ( newLine ) {
          bool found = FALSE;
          for (i = singleXMLLine.length() - 2; i > 0 && !found; i--)
            found = ( singleXMLLine[i] == '<' );
          if ( found && i == 0 )
            newLine = FALSE;
        }
      }
      bool needsSpace = singleXMLLine.endsWith("\"");
      if ( needsSpace ) {
        bool found = FALSE;
        bool stop = FALSE;
        for (i = singleXMLLine.length() - 2; i > 1 && !found && !stop; i--) {
          if ( singleXMLLine[i] == '\"' ) {
            if ( singleXMLLine[i - 1] == '=' )
              found = TRUE;
            else
              stop = TRUE;
          }
        }
        if ( !found )
          needsSpace = FALSE;
      }
      needsSpace |= endsWithSpace;
      if ( newLine ) {
        if ( singleXMLLine[singleXMLLine.length() - 1].isSpace() )
          singleXMLLine.remove(singleXMLLine.length() - 1, 1);
        needsSpace = FALSE;
      }
      gamelistBuffer += singleXMLLine + QString(needsSpace ? " " : "") + QString(newLine ? "\n" : "");
      if ( listXMLCache.isOpen() )
        tsListXMLCache << singleXMLLine << QString(needsSpace ? " " : "") << QString(newLine ? "\n" : "");
    }
  }

#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + s.count("<game name="));
#elif defined(QMC2_SDLMAME) || defined(QMC2_MESS)
  qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + s.count("<machine name="));
#endif
}

void Gamelist::loadReadyReadStandardError()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadReadyReadStandardError(): proc = 0x" + QString::number((ulong)proc, 16));
#endif

}

void Gamelist::loadError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadError(QProcess::ProcessError processError = " + QString::number(processError) + ")");
#endif

}

void Gamelist::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadStateChanged(QProcess::ProcessState = " + QString::number(processState) + ")");
#endif

}

void Gamelist::verifyStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyStarted()");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  romCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/ROMStateCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  romCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/ROMStateCacheFile").toString());
#endif
  romCache.open(QIODevice::WriteOnly | QIODevice::Text);
  if ( !romCache.isOpen() )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open ROM state cache for writing, path = %1").arg(romCache.fileName()));
  else {
    tsRomCache.setDevice(&romCache);
    tsRomCache.reset();
    tsRomCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
  }
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(tr("ROM check - %p%"));
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");
  qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
  qmc2MainWindow->progressBarGamelist->reset();

  if ( !verifyCurrentOnly ) {
    numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numNotFoundGames = numUnknownGames = 0;
    qmc2MainWindow->labelGamelistStatus->setText(status());
  }
}

void Gamelist::verifyFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyFinished(int exitCode = " + QString::number(exitCode) + ", QProcess::ExitStatus exitStatus = " + QString::number(exitStatus) + "): proc = 0x" + QString::number((ulong)proc, 16));
#endif

  bool cleanExit = TRUE;
  if ( exitStatus != QProcess::NormalExit && !qmc2StopParser ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));
    cleanExit = FALSE;
  }

  if ( !verifyCurrentOnly ) {
    QSet<QString> gameSet = QSet<QString>::fromList(qmc2GamelistItemMap.uniqueKeys());
    QList<QString> remainingGames = gameSet.subtract(QSet<QString>::fromList(verifiedList)).values();
    int i;
    if ( qmc2StopParser || !cleanExit ) {
      for (i = 0; i < remainingGames.count(); i++) {
        qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + 1);
        QString gameName = remainingGames[i];
        bool isBIOS = qmc2BiosROMs.contains(gameName);
        QTreeWidgetItem *romItem = qmc2GamelistItemMap[gameName];
        QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
        if ( romItem && hierarchyItem ) {
          if ( romCache.isOpen() )
            tsRomCache << gameName << " U\n";
          numUnknownGames++;
          if ( isBIOS ) {
            romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
            hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
          } else {
            romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
            hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
          }
          romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
          hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
        } else {
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(gameName));
          if ( romCache.isOpen() )
            tsRomCache << gameName << " U\n";
          numUnknownGames++;
        }
      }
    } else {
      for (i = 0; i < remainingGames.count(); i++) {
        qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + 1);
        QString gameName = remainingGames[i];
        bool isBIOS = qmc2BiosROMs.contains(gameName);
        QTreeWidgetItem *romItem = qmc2GamelistItemMap[gameName];
        QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
        if ( romItem && hierarchyItem ) {
          if ( romCache.isOpen() )
            tsRomCache << gameName << " N\n";
          numNotFoundGames++;
          if ( isBIOS ) {
            romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
            hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
          } else {
            romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
            hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
          }
          romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
          hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
        } else {
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(gameName));
          if ( romCache.isOpen() )
            tsRomCache << gameName << " U\n";
          numUnknownGames++;
        }
      }
    }
    qmc2MainWindow->labelGamelistStatus->setText(status());
  }

  if ( verifyCurrentOnly && romCache.isOpen() ) {
    QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
    while ( it.hasNext() ) {
      it.next();
      tsRomCache << it.key() << " ";
      switch ( it.value()->whatsThis(QMC2_GAMELIST_COLUMN_GAME)[0].toAscii() ) {
        case QMC2_ROMSTATE_CHAR_C:
          tsRomCache << "C\n";
          break;
        case QMC2_ROMSTATE_CHAR_M:
          tsRomCache << "M\n";
          break;
        case QMC2_ROMSTATE_CHAR_U:
          tsRomCache << "U\n";
          break;
        case QMC2_ROMSTATE_CHAR_I:
          tsRomCache << "I\n";
          break;
        case QMC2_ROMSTATE_CHAR_N:
          tsRomCache << "N\n";
          break;
      }
    }
  }

  QTime elapsedTime;
  elapsedTime = elapsedTime.addMSecs(verifyTimer.elapsed());
  if ( verifyCurrentOnly )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for '%1', elapsed time = %2)").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)).arg(elapsedTime.toString("mm:ss.zzz")));
  else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for all games, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for all machines, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  }
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state info: C:%1 M:%2 I:%3 N:%4 U:%5").arg(numCorrectGames).arg(numMostlyCorrectGames).arg(numIncorrectGames).arg(numNotFoundGames).arg(numUnknownGames));
  qmc2MainWindow->progressBarGamelist->reset();
  qmc2VerifyActive = FALSE;
  if ( verifyProc )
    delete verifyProc;
  verifyProc = NULL;

  if ( romCache.isOpen() ) {
    tsRomCache.flush();
    romCache.close();
  }

  if ( qmc2SortCriteria == QMC2_SORT_BY_ROM_STATE ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting game list by %1 in %2 order").arg(tr("ROM state")).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(tr("ROM state")).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#endif
    qApp->processEvents();
    QList<QTreeWidgetItem *> itemList = qmc2MainWindow->treeWidgetGamelist->findItems("*", Qt::MatchContains | Qt::MatchWildcard);
    for (int i = 0; i < itemList.count(); i++) {
      if ( itemList[i]->childCount() > 1 ) {
        qmc2MainWindow->treeWidgetGamelist->collapseItem(itemList[i]);
        QList<QTreeWidgetItem *> childrenList = itemList[i]->takeChildren();
        int j;
        for (j = 0; j < childrenList.count(); j++)
          delete childrenList[j];
        QTreeWidgetItem *nameItem = new QTreeWidgetItem(itemList[i]);
        nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
        nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, qmc2GamelistNameMap[itemList[i]->text(QMC2_GAMELIST_COLUMN_GAME)]);
        qApp->processEvents();
      }
    }
    qApp->processEvents();
    qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
    QTreeWidgetItem *ci = qmc2MainWindow->treeWidgetGamelist->currentItem();
    if ( ci )
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
  }

  enableWidgets(TRUE);

  filter();
}

void Gamelist::verifyReadyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyReadyReadStandardOutput(): proc = 0x" + QString::number((ulong)proc, 16));
#endif

  // process rom verification output
  int i;
  QString romName, romState, romStateLong; 
  QString s = verifyLastLine + proc->readAllStandardOutput();
#if defined(Q_WS_WIN)
  s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
  QStringList lines = s.split("\n");

  if ( s.endsWith("\n") ) {
    verifyLastLine = "";
  } else {
    verifyLastLine = lines.last();
    lines.removeLast();
  }

  for (i = 0; i < lines.count(); i++) {
    if ( lines[i].startsWith("romset ") ) {
      QStringList words = lines[i].split(" ");
      numVerifyRoms++;
      if ( words.count() > 2 ) {
        romName = words[1].remove("\"");
        bool isBIOS = qmc2BiosROMs.contains(romName);
        if ( qmc2GamelistItemMap.count(romName) == 1 ) {
          QTreeWidgetItem *romItem = qmc2GamelistItemMap[romName];
          QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[romName];
          if ( romItem && hierarchyItem ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
            if ( words.last() == "good" ) {
              romState = "C";
              romStateLong = QObject::tr("correct");
              numCorrectGames++;
              if ( isBIOS ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
            } else if ( words.last() == "bad" ) {
              romState = "I";
              romStateLong = QObject::tr("incorrect");
              numIncorrectGames++;
              if ( isBIOS ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
            } else if ( words.last() == "available" ) {
              romState = "M";
              romStateLong = QObject::tr("mostly correct");
              numMostlyCorrectGames++;
              if ( isBIOS ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
            } else if ( words.last() == "missing" || words.last() == "found!" ) {
              romState = "N";
              romStateLong = QObject::tr("not found");
              numNotFoundGames++;
              if ( isBIOS ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
            } else {
              romState = "U";
              romStateLong = QObject::tr("unknown");
              numUnknownGames++;
              if ( isBIOS ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
            }
#endif
          } else {
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(romName));
            romState = "U";
            romStateLong = QObject::tr("unknown");
            numUnknownGames++;
          }

#ifdef QMC2_DEBUG
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyReadyReadStandardOutput(): " + romName + " " + romState);
#endif

          qmc2GamelistStatusMap[romName] = romState;

#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
          verifiedList << romName;
#endif

          if ( verifyCurrentOnly ) {
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM status for '%1' is '%2'").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)).arg(romStateLong));
            numUnknownGames--;
          } else if ( romCache.isOpen() )
            tsRomCache << romName << " " << romState << "\n";
        }
      }
    }
  }
  if ( romCache.isOpen() && !verifyCurrentOnly )
    tsRomCache.flush();

  if ( qmc2StopParser && verifyProc )
    verifyProc->terminate();

  qmc2MainWindow->progressBarGamelist->setValue(numVerifyRoms);
  qmc2MainWindow->labelGamelistStatus->setText(status());
}

void Gamelist::verifyReadyReadStandardError()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyReadyReadStandardError(): proc = 0x" + QString::number((ulong)proc, 16));
#endif

}

void Gamelist::verifyError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyError(QProcess::ProcessError processError = " + QString::number(processError) + ")");
#endif

}

void Gamelist::verifyStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyStateChanged(QProcess::ProcessState = " + QString::number(processState) + ")");
#endif

}

bool Gamelist::loadIcon(QString gameName, QTreeWidgetItem *item, bool checkOnly, QString *fileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadIcon(QString gameName = %1, QTreeWidgetItem *item = %2, bool checkOnly = %3, QString *fileName = %4)").arg(gameName).arg((qulonglong)item).arg(checkOnly).arg((qulonglong)fileName));
#endif

#if QT_VERSION < 0x040600
  static QIcon icon;
  static char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
  static QPixmap pm;
#else
  QIcon icon;
  char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
#endif

  if ( fileName )
    *fileName = gameName + ".png";

  if ( qmc2IconMap.contains(gameName) ) {
    // use cached icon
    if ( !checkOnly )
      item->setIcon(QMC2_GAMELIST_COLUMN_ICON, qmc2IconMap.value(gameName));
    else
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);

    return TRUE;
  } else if ( qmc2IconsPreloaded ) {
    // icon wasn't found
    if ( !checkOnly ) {
      icon = QIcon();
      qmc2IconMap[gameName] = icon;
      item->setIcon(QMC2_GAMELIST_COLUMN_ICON, icon);
    } else
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);

    return FALSE;
  }

  if ( qmc2UseIconFile ) {
    // use icon file
    QByteArray imageData;
    int len, i;
    if ( !qmc2IconsPreloaded ) {
      QTime preloadTimer, elapsedTime;
      int iconCount = 0;
      preloadTimer.start();
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from ZIP archive"));
      unz_global_info unzGlobalInfo;
      if ( unzGetGlobalInfo(qmc2IconFile, &unzGlobalInfo) == UNZ_OK ) {
        int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
        QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
        if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
          qmc2MainWindow->progressBarGamelist->setFormat(tr("Icon cache - %p%"));
        else
          qmc2MainWindow->progressBarGamelist->setFormat("%p%");
        qmc2MainWindow->progressBarGamelist->setRange(0, unzGlobalInfo.number_entry);
        qmc2MainWindow->progressBarGamelist->reset();
        qApp->processEvents();
        if ( unzGoToFirstFile(qmc2IconFile) == UNZ_OK ) {
          do {
            char unzFileName[QMC2_MAX_PATH_LENGTH];
            iconCount++;
            if ( iconCount % QMC2_ICONCACHE_RESPONSIVENESS == 0 ) {
              qmc2MainWindow->progressBarGamelist->setValue(iconCount);
              qApp->processEvents();
            }
            if ( unzGetCurrentFileInfo(qmc2IconFile, NULL, unzFileName, QMC2_MAX_PATH_LENGTH, NULL, 0, NULL, 0) == UNZ_OK ) {
              QString gameFileName = unzFileName;
#ifdef QMC2_DEBUG
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: loading %1").arg(gameFileName));
#endif
              imageData.clear();
              if ( unzOpenCurrentFile(qmc2IconFile) == UNZ_OK ) {
                while ( (len = unzReadCurrentFile(qmc2IconFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
                  for (i = 0; i < len; i++)
                    imageData += imageBuffer[i];
                unzCloseCurrentFile(qmc2IconFile);
#if QT_VERSION < 0x040600
                if ( pm.loadFromData(imageData) )
                  qmc2IconMap[gameFileName.toLower().remove(".png")] = QIcon(pm);
#else
                QPixmap iconPixmap;
                if ( iconPixmap.loadFromData(imageData) )
                  qmc2IconMap[gameFileName.toLower().remove(".png")] = QIcon(iconPixmap);
#endif
              }
            }
            if ( iconCount % qmc2GamelistResponsiveness == 0 ) {
              qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);
              qApp->processEvents();
              qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(FALSE);
            }
          } while ( unzGoToNextFile(qmc2IconFile) != UNZ_END_OF_LIST_OF_FILE );
        }
        qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
        if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
          qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
        else
          qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      }
      elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from ZIP archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", iconCount));
      qmc2IconsPreloaded = TRUE;

      if ( checkOnly )
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);

      return loadIcon(gameName, item, checkOnly);
    }
  } else {
    // use icon directory
    if ( !qmc2IconsPreloaded ) {
      QTime preloadTimer, elapsedTime;
      preloadTimer.start();
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from directory"));
      qApp->processEvents();
#if defined(QMC2_EMUTYPE_MAME)
      QString icoDir = qmc2Config->value("MAME/FilesAndDirectories/IconDirectory").toString();
#elif defined(QMC2_EMUTYPE_MESS)
      QString icoDir = qmc2Config->value("MESS/FilesAndDirectories/IconDirectory").toString();
#endif
      QDir iconDirectory(icoDir);
      QStringList nameFilter;
      nameFilter << "*.png";
      QStringList iconFiles = iconDirectory.entryList(nameFilter, QDir::Files | QDir::Readable);
      int iconCount;
      int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
      QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("Icon cache - %p%"));
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      qmc2MainWindow->progressBarGamelist->setRange(0, iconFiles.count());
      qmc2MainWindow->progressBarGamelist->reset();
      qApp->processEvents();
      for (iconCount = 0; iconCount < iconFiles.count(); iconCount++) {
        qmc2MainWindow->progressBarGamelist->setValue(iconCount);
        if ( iconCount % 25 == 0 )
          qApp->processEvents();
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: loading %1").arg(iconFiles[iconCount]));
#endif
#if QT_VERSION < 0x040600
        if ( pm.load(icoDir + iconFiles[iconCount]) )
          icon = QIcon(pm);
        else
          icon = QIcon();
#else
        QPixmap iconPixmap;
        if ( iconPixmap.load(icoDir + iconFiles[iconCount]) )
          icon = QIcon(iconPixmap);
        else
          icon = QIcon();
#endif
        qmc2IconMap[iconFiles[iconCount].toLower().remove(".png")] = icon;
        if ( iconCount % qmc2GamelistResponsiveness == 0 ) {
          qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);
          qApp->processEvents();
          qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(FALSE);
        }
      }
      qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from directory, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", iconCount));
      qmc2IconsPreloaded = TRUE;

      if ( checkOnly )
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);

      return loadIcon(gameName, item, checkOnly);
    }
  }

  if ( checkOnly )
    qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(TRUE);

  return FALSE;
}

#if defined(QMC2_EMUTYPE_MAME)
void Gamelist::loadCatverIni()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadCatverIni()");
#endif

  qmc2CategoryMap.clear();
  qmc2VersionMap.clear();

  QTime loadTimer, elapsedTime;
  loadTimer.start();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading catver.ini"));
  qApp->processEvents();

  int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
  QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(tr("Catver.ini - %p%"));
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");
  qmc2MainWindow->progressBarGamelist->setRange(0, 2 * numTotalGames); // we can't assume that catver.ini has exactly this number of games, though!
  qmc2MainWindow->progressBarGamelist->reset();
  qApp->processEvents();

  QFile catverIniFile(qmc2Config->value("MAME/FilesAndDirectories/CatverIni").toString());
  int entryCounter = 0;
  if ( catverIniFile.open(QFile::ReadOnly) ) {
    QTextStream tsCatverIni(&catverIniFile);
    bool isVersion = FALSE, isCategory = FALSE;
    while ( !tsCatverIni.atEnd() ) {
      QString catverLine = tsCatverIni.readLine().simplified().trimmed();
      if ( catverLine.isEmpty() )
        continue;
      if ( catverLine.contains("[Category]") ) {
        isCategory = TRUE;
        isVersion = FALSE;
      } else if ( catverLine.contains("[VerAdded]") ) {
        isCategory = FALSE;
        isVersion = TRUE;
      } else {
        QStringList tokens = catverLine.split("=");
        if ( tokens.count() >= 2 ) {
          qmc2MainWindow->progressBarGamelist->setValue(++entryCounter);
          if ( isCategory )
            qmc2CategoryMap.insert(tokens[0], tokens[1]);
          else if ( isVersion ) {
            QString verStr = tokens[1];
            if ( verStr.startsWith(".") ) verStr.prepend("0");
            qmc2VersionMap.insert(tokens[0], verStr);
          }
        }
      }
    }
    catverIniFile.close();
  } else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open '%1' for reading -- no catver.ini data available").arg(qmc2Config->value("MAME/FilesAndDirectories/CatverIni").toString()));

  qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");

  elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading catver.ini, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 category / %2 version records loaded").arg(qmc2CategoryMap.count()).arg(qmc2VersionMap.count()));
}

void Gamelist::createCategoryView()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::createCategoryView()");
#endif

	qmc2CategoryItemMap.clear();
	qmc2MainWindow->treeWidgetCategoryView->hide();
	qmc2MainWindow->labelCreatingCategoryView->show();

	if ( qmc2ReloadActive && !qmc2StopParser && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_CATEGORY_INDEX ) {
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createCategoryView()));
		return;
	} else if ( qmc2MainWindow->stackedWidgetView->currentIndex() != QMC2_VIEW_CATEGORY_INDEX )
		return;

	qApp->processEvents();

	if ( !qmc2StopParser ) {
		qmc2MainWindow->treeWidgetCategoryView->clear();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("Category view - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		qmc2MainWindow->progressBarGamelist->setRange(0, qmc2CategoryMap.count());
		qmc2MainWindow->progressBarGamelist->reset();
		QMapIterator<QString, QString> it(qmc2CategoryMap);
		int counter = 0;
		while ( it.hasNext() ) {
			it.next();
			qmc2MainWindow->progressBarGamelist->setValue(counter++);
			QString gameName = it.key();
			QString category = it.value();
			if ( gameName.isEmpty() )
				continue;
			if ( category.isEmpty() )
				category = tr("?");
			QTreeWidgetItem *baseItem = qmc2GamelistItemMap[gameName];
			if ( baseItem ) {
				QList<QTreeWidgetItem *> matchItems = qmc2MainWindow->treeWidgetCategoryView->findItems(category, Qt::MatchExactly);
				QTreeWidgetItem *categoryItem = NULL;
				if ( matchItems.count() > 0 )
					categoryItem = matchItems[0];
				if ( categoryItem == NULL ) {
					categoryItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetCategoryView);
					categoryItem->setText(QMC2_GAMELIST_COLUMN_GAME, category);
				}
				QTreeWidgetItem *gameItem = new QTreeWidgetItem(categoryItem);
				gameItem->setText(QMC2_GAMELIST_COLUMN_GAME, baseItem->text(QMC2_GAMELIST_COLUMN_GAME));
				gameItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
				gameItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
				gameItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
				gameItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
				gameItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
				gameItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
				switch ( qmc2GamelistStatusMap[gameName][0].toAscii() ) {
					case 'C':
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2CorrectBIOSImageIcon : qmc2CorrectImageIcon);
						break;
					case 'M':
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2MostlyCorrectBIOSImageIcon : qmc2MostlyCorrectImageIcon);
						break;
					case 'I':
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2IncorrectBIOSImageIcon : qmc2IncorrectImageIcon);
						break;
					case 'N':
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2NotFoundBIOSImageIcon : qmc2NotFoundImageIcon);
						break;
					default:
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2UnknownBIOSImageIcon : qmc2UnknownImageIcon);
						break;
				}
				loadIcon(gameName, gameItem);
				qmc2CategoryItemMap[gameName] = gameItem;
			}
		}
		qmc2MainWindow->treeWidgetCategoryView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->progressBarGamelist->reset();
	}

	qmc2MainWindow->labelCreatingCategoryView->hide();
	qmc2MainWindow->treeWidgetCategoryView->show();

	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
}

void Gamelist::createVersionView()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::createVersionView()");
#endif

	qmc2VersionItemMap.clear();
	qmc2MainWindow->treeWidgetVersionView->hide();
	qmc2MainWindow->labelCreatingVersionView->show();

	if ( qmc2ReloadActive && !qmc2StopParser && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_VERSION_INDEX ) {
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createVersionView()));
		return;
	} else if ( qmc2MainWindow->stackedWidgetView->currentIndex() != QMC2_VIEW_VERSION_INDEX )
		return;

	qApp->processEvents();

	if ( !qmc2StopParser ) {
		qmc2MainWindow->treeWidgetVersionView->clear();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("Version view - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		qmc2MainWindow->progressBarGamelist->setRange(0, qmc2VersionMap.count());
		qmc2MainWindow->progressBarGamelist->reset();
		QMapIterator<QString, QString> it(qmc2VersionMap);
		int counter = 0;
		while ( it.hasNext() ) {
			it.next();
			qmc2MainWindow->progressBarGamelist->setValue(counter++);
			QString gameName = it.key();
			QString version = it.value();
			if ( gameName.isEmpty() )
				continue;
			if ( version.isEmpty() )
				version = tr("?");
			QTreeWidgetItem *baseItem = qmc2GamelistItemMap[gameName];
			if ( baseItem ) {
				QList<QTreeWidgetItem *> matchItems = qmc2MainWindow->treeWidgetVersionView->findItems(version, Qt::MatchExactly);
				QTreeWidgetItem *versionItem = NULL;
				if ( matchItems.count() > 0 )
					versionItem = matchItems[0];
				if ( versionItem == NULL ) {
					versionItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetVersionView);
					versionItem->setText(QMC2_GAMELIST_COLUMN_GAME, version);
				}
				QTreeWidgetItem *gameItem = new QTreeWidgetItem(versionItem);
				gameItem->setText(QMC2_GAMELIST_COLUMN_GAME, baseItem->text(QMC2_GAMELIST_COLUMN_GAME));
				gameItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
				gameItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
				gameItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
				gameItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
				gameItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
				gameItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
				switch ( qmc2GamelistStatusMap[gameName][0].toAscii() ) {
					case 'C':
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2CorrectBIOSImageIcon : qmc2CorrectImageIcon);
						break;
					case 'M':
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2MostlyCorrectBIOSImageIcon : qmc2MostlyCorrectImageIcon);
						break;
					case 'I':
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2IncorrectBIOSImageIcon : qmc2IncorrectImageIcon);
						break;
					case 'N':
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2NotFoundBIOSImageIcon : qmc2NotFoundImageIcon);
						break;
					default:
						gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2BiosROMs.contains(gameName) ? qmc2UnknownBIOSImageIcon : qmc2UnknownImageIcon);
						break;
				}
				loadIcon(gameName, gameItem);
				qmc2VersionItemMap[gameName] = gameItem;
			}
		}
		qmc2MainWindow->treeWidgetVersionView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->progressBarGamelist->reset();
	}

	qmc2MainWindow->labelCreatingVersionView->hide();
	qmc2MainWindow->treeWidgetVersionView->show();

	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
}
#endif

bool GamelistItem::operator<(const QTreeWidgetItem &otherItem) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: GamelistItem::operator<(const GamelistItem &otherItem = ...)");
#endif
  
  switch ( qmc2SortCriteria ) {
    case QMC2_SORT_BY_DESCRIPTION:
      return (text(QMC2_GAMELIST_COLUMN_GAME).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_GAME).toUpper());
      break;

    case QMC2_SORT_BY_ROM_STATE:
      return (whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() < otherItem.whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii());
      break;

    case QMC2_SORT_BY_YEAR:
      return (text(QMC2_GAMELIST_COLUMN_YEAR) < otherItem.text(QMC2_GAMELIST_COLUMN_YEAR));
      break;

    case QMC2_SORT_BY_MANUFACTURER:
      return (text(QMC2_GAMELIST_COLUMN_MANU).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_MANU).toUpper());
      break;

    case QMC2_SORT_BY_NAME:
      return (text(QMC2_GAMELIST_COLUMN_NAME).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_NAME).toUpper());
      break;

    case QMC2_SORT_BY_ROMTYPES:
      return (text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper());
      break;

#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_SORT_BY_CATEGORY:
      return (text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper());
      break;

    case QMC2_SORT_BY_VERSION:
      return (text(QMC2_GAMELIST_COLUMN_VERSION).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_VERSION).toUpper());
      break;
#endif

    default:
      return FALSE;
      break;
  }
}
