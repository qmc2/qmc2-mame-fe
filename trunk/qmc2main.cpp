#include <QMessageBox>
#include <QPixmapCache>
#include <QCache>
#include <QHeaderView>
#include <QTextStream>
#include <QScrollBar>
#include <QProcess>
#include <QTimer>
#include <QTime>
#include <QFile>
#include <QMap>
#include <QStyleFactory>
#include <QBitArray>
#include <QFileInfo>
#include <QFileDialog>
#include <QAction>
#include <QPair>
#include <QClipboard>
#include <QDateTime>
#include <QMutex>
#include <QtWebKit>
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "macros.h"
#include "qmc2main.h"
#include "options.h"
#include "emuopt.h"
#include "procmgr.h"
#include "gamelist.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "dbrowser.h"
#include "about.h"
#include "welcome.h"
#include "imgcheck.h"
#include "sampcheck.h"
#include "romalyzer.h"
#include "romstatusexport.h"
#include "detailsetup.h"
#include "miniwebbrowser.h"
#include "unzip.h"
#if QMC2_JOYSTICK == 1
#include "joystick.h"
#endif
#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
#include "messdevcfg.h"
#endif
#if QMC2_USE_PHONON_API
#include <QTest>
#endif
#include "arcade/arcadeview.h"
#include "arcade/arcadesetupdialog.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#if defined(verify) // verify() is #defined in AssertMacros.h on OS X
#undef verify
#endif
#endif

// global variables
MainWindow *qmc2MainWindow = NULL;
Gamelist *qmc2Gamelist = NULL;
Options *qmc2Options = NULL;
QSettings *qmc2Config = NULL;
EmulatorOptions *qmc2GlobalEmulatorOptions = NULL;
EmulatorOptions *qmc2EmulatorOptions = NULL;
ProcessManager *qmc2ProcessManager = NULL;
Preview *qmc2Preview = NULL;
Flyer *qmc2Flyer = NULL;
Cabinet *qmc2Cabinet = NULL;
Controller *qmc2Controller = NULL;
Marquee *qmc2Marquee = NULL;
Title *qmc2Title = NULL;
About *qmc2About = NULL;
DocBrowser *qmc2DocBrowser = NULL;
Welcome *qmc2Welcome = NULL;
ImageChecker *qmc2ImageChecker = NULL;
SampleChecker *qmc2SampleChecker = NULL;
ROMAlyzer *qmc2ROMAlyzer = NULL;
ROMStatusExporter *qmc2ROMStatusExporter = NULL;
DetailSetup *qmc2DetailSetup = NULL;
ArcadeView *qmc2ArcadeView = NULL;
ArcadeSetupDialog *qmc2ArcadeSetupDialog = NULL;
#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
MESSDeviceConfigurator *qmc2MESSDeviceConfigurator = NULL;
#endif
bool qmc2ReloadActive = FALSE;
bool qmc2ImageCheckActive = FALSE;
bool qmc2SampleCheckActive = FALSE;
bool qmc2EarlyReloadActive = FALSE;
bool qmc2VerifyActive = FALSE;
bool qmc2FilterActive = FALSE;
bool qmc2ROMAlyzerActive = FALSE;
bool qmc2ROMAlyzerPaused = FALSE;
bool qmc2GuiReady = FALSE;
bool qmc2CleaningUp = FALSE;
bool qmc2StartingUp = TRUE;
bool qmc2EarlyStartup = TRUE;
bool qmc2ScaledPreview = TRUE;
bool qmc2ScaledFlyer = TRUE;
bool qmc2ScaledCabinet = TRUE;
bool qmc2ScaledController = TRUE;
bool qmc2ScaledMarquee = TRUE;
bool qmc2ScaledTitle = TRUE;
bool qmc2SmoothScaling = FALSE;
bool qmc2RetryLoadingImages = TRUE;
bool qmc2ParentImageFallback = FALSE;
bool qmc2UsePreviewFile = FALSE;
bool qmc2UseFlyerFile = FALSE;
bool qmc2UseCabinetFile = FALSE;
bool qmc2UseControllerFile = FALSE;
bool qmc2UseIconFile = FALSE;
bool qmc2UseMarqueeFile = FALSE;
bool qmc2UseTitleFile = FALSE;
bool qmc2IconsPreloaded = FALSE;
bool qmc2CheckItemVisibility = TRUE;
bool qmc2AutomaticReload = FALSE;
bool qmc2SuppressQtMessages = FALSE;
bool qmc2LoadingGameInfoDB = FALSE;
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
bool qmc2LoadingEmuInfoDB = FALSE;
#endif
bool qmc2WidgetsEnabled = TRUE;
bool qmc2ExportingROMStatus = FALSE;
bool qmc2StatesTogglesEnabled = TRUE;
bool qmc2VariantSwitchReady = FALSE;
bool qmc2DestroyingArcadeView = FALSE;
bool qmc2IgnoreItemActivation = FALSE;
int qmc2GamelistResponsiveness = 100;
int qmc2UpdateDelay = 10;
QFile *qmc2FrontendLogFile = NULL;
QFile *qmc2EmulatorLogFile = NULL;
QFile *qmc2FifoFile = NULL;
QTextStream qmc2FrontendLogStream;
QTextStream qmc2EmulatorLogStream;
QTranslator *qmc2Translator = NULL;
QTranslator *qmc2QtTranslator = NULL;
QString qmc2LastFrontendLogMessage = "";
quint64 qmc2FrontendLogMessageRepeatCount = 0;
QString qmc2LastEmulatorLogMessage = "";
quint64 qmc2EmulatorLogMessageRepeatCount = 0;
bool qmc2StopParser = FALSE;
QTreeWidgetItem *qmc2CurrentItem = NULL;
QTreeWidgetItem *qmc2LastConfigItem = NULL;
QTreeWidgetItem *qmc2LastDeviceConfigItem = NULL;
QTreeWidgetItem *qmc2LastGameInfoItem = NULL;
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
QTreeWidgetItem *qmc2LastEmuInfoItem = NULL;
MiniWebBrowser *qmc2MAWSLookup = NULL;
QTreeWidgetItem *qmc2LastMAWSItem = NULL;
QCache<QString, QByteArray> qmc2MAWSCache;
#endif
QTreeWidgetItem *qmc2HierarchySelectedItem = NULL;
QMenu *qmc2EmulatorMenu = NULL,
      *qmc2GameMenu = NULL,
      *qmc2FavoritesMenu = NULL,
      *qmc2PlayedMenu = NULL,
      *qmc2SearchMenu = NULL;
QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
QMap<QString, QTreeWidgetItem *> qmc2GamelistItemByDescriptionMap;
QMap<QString, QString> qmc2GamelistNameMap;
QMap<QString, QString> qmc2GamelistDescriptionMap;
QMap<QString, QString> qmc2GamelistStatusMap;
QMap<QString, QStringList> qmc2HierarchyMap;
QMap<QString, QString> qmc2ParentMap;
QMap<QString, QIcon> qmc2IconMap;
QMap<QString, QPair<QString, QAction *> > qmc2ShortcutMap;
QMap<QString, QString> qmc2CustomShortcutMap;
QMap<QString, QString> qmc2JoystickFunctionMap;
QMap<QString, QByteArray *> qmc2GameInfoDB;
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
QMap<QString, QByteArray *> qmc2EmuInfoDB;
#endif
int qmc2SortCriteria = QMC2_SORT_BY_DESCRIPTION;
Qt::SortOrder qmc2SortOrder = Qt::AscendingOrder;
QBitArray qmc2Filter;
unzFile qmc2IconFile = NULL;
QStringList qmc2BiosROMs;
KeyPressFilter *qmc2KeyPressFilter = NULL;
QMap<QString, int> qmc2QtKeyMap;
#if QMC2_JOYSTICK == 1
Joystick *qmc2Joystick = NULL;
#endif
QString qmc2DefaultStyle;
QSocketNotifier *qmc2FifoNotifier = NULL;
bool qmc2ShowGameName = FALSE;
bool qmc2ShowGameNameOnlyWhenRequired = TRUE;
#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
QString qmc2MessMachineName = "";
#endif
QMutex qmc2LogMutex;
QString qmc2FileEditStartPath = "";
QString qmc2DirectoryEditStartPath = "";

// game status colors 
QColor MainWindow::qmc2StatusColorGreen = QColor("#00cc00");
QColor MainWindow::qmc2StatusColorYellowGreen = QColor("#a2c743");
QColor MainWindow::qmc2StatusColorRed = QColor("#f90000");
QColor MainWindow::qmc2StatusColorBlue = QColor("#0000f9");
QColor MainWindow::qmc2StatusColorGrey = QColor("#7f7f7f");

// extern global variables
extern bool exitArcade;

void MainWindow::log(char logOrigin, QString message)
{
  if ( !qmc2GuiReady )
    return;

  QString timeString = QTime::currentTime().toString("hh:mm:ss.zzz");

  qmc2LogMutex.lock();
  // count subsequent message duplicates
  switch ( logOrigin ) {
    case QMC2_LOG_FRONTEND:
      if ( message == qmc2LastFrontendLogMessage ) {
        qmc2FrontendLogMessageRepeatCount++;
        qmc2LogMutex.unlock();
        return;
      } else {
        qmc2LastFrontendLogMessage = message;
        if ( qmc2FrontendLogMessageRepeatCount > 0 )
          message.prepend(tr("last message repeated %n time(s)\n", "", qmc2FrontendLogMessageRepeatCount) + timeString + ": ");
        qmc2FrontendLogMessageRepeatCount = 0;
      }
      break;

    case QMC2_LOG_EMULATOR:
      if ( message == qmc2LastEmulatorLogMessage ) {
        qmc2EmulatorLogMessageRepeatCount++;
        qmc2LogMutex.unlock();
        return;
      } else {
        qmc2LastEmulatorLogMessage = message;
        if ( qmc2EmulatorLogMessageRepeatCount > 0 )
          message.prepend(tr("last message repeated %n time(s)\n", "", qmc2EmulatorLogMessageRepeatCount) + timeString + ": ");
        qmc2EmulatorLogMessageRepeatCount = 0;
      }
      break;

    default:
      break;
  }

  QString msg = timeString + ": " + message;

  switch ( logOrigin ) {
    case QMC2_LOG_FRONTEND:
      textBrowserFrontendLog->append(msg);
      textBrowserFrontendLog->horizontalScrollBar()->setValue(0);
      if ( !qmc2FrontendLogFile )
        if ( (qmc2FrontendLogFile = new QFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", "data/log/qmc2.log").toString(), this)) == NULL ) {
          qmc2LogMutex.unlock();
          return;
        }
      if ( qmc2FrontendLogFile->handle() == -1 ) {
        if ( qmc2FrontendLogFile->open(QIODevice::WriteOnly | QIODevice::Text) )
          qmc2FrontendLogStream.setDevice(qmc2FrontendLogFile);
        else {
          qmc2LogMutex.unlock();
          return;
        }
      }
      qmc2FrontendLogStream << msg << "\n";
      qmc2FrontendLogStream.flush();
      break;

    case QMC2_LOG_EMULATOR:
      textBrowserEmulatorLog->append(msg);
      textBrowserEmulatorLog->horizontalScrollBar()->setValue(0);
      if ( !qmc2EmulatorLogFile ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
        if ( (qmc2EmulatorLogFile = new QFile(qmc2Config->value("MAME/FilesAndDirectories/LogFile", "data/log/mame.log").toString(), this)) == NULL ) {
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
        if ( (qmc2EmulatorLogFile = new QFile(qmc2Config->value("MESS/FilesAndDirectories/LogFile", "data/log/mess.log").toString(), this)) == NULL ) {
#endif
          qmc2LogMutex.unlock();
          return;
        }
      }
      if ( qmc2EmulatorLogFile->handle() == -1 ) {
        if ( qmc2EmulatorLogFile->open(QIODevice::WriteOnly | QIODevice::Text) )
          qmc2EmulatorLogStream.setDevice(qmc2EmulatorLogFile);
        else {
          qmc2LogMutex.unlock();
          return;
        }
      }
      qmc2EmulatorLogStream << msg << "\n";
      qmc2EmulatorLogStream.flush();
      break;

    default:
      break;
  }

  qmc2LogMutex.unlock();
}

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::MainWindow(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "InstanceRunning"), TRUE);

  // remember the default style
  qmc2DefaultStyle = QApplication::style()->objectName();

  // initial setup of the application style
  QString myStyle = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Style", tr("Default")).toString();
  setupStyle(myStyle);

  setupUi(this);

  // updated by detail setup
  tabWidgetGameDetail->setUpdatesEnabled(FALSE);

#if defined(Q_WS_WIN)
  actionLaunchQMC2MAME->setText(tr("QMC2 for MAME"));
  actionLaunchQMC2MAME->setToolTip(tr("Launch QMC2 for MAME"));
  actionLaunchQMC2MAME->setStatusTip(tr("Launch QMC2 for MAME"));
  actionLaunchQMC2MESS->setText(tr("QMC2 for MESS"));
  actionLaunchQMC2MESS->setToolTip(tr("Launch QMC2 for MESS"));
  actionLaunchQMC2MESS->setStatusTip(tr("Launch QMC2 for MESS"));
#endif

  // FIXME: remove this when arcade mode is ready
#if QMC2_WIP_CODE != 1
  menu_Display->removeAction(menu_Arcade->menuAction());
  actionArcadeToggle->setVisible(FALSE);
#endif

  // FIXME: remove this when the download manager is ready
#if QMC2_WIP_CODE != 1 || defined(QMC2_SDLMESS) || defined(QMC2_MESS) || QT_VERSION < 0x040500
  tabWidgetLogsAndEmulators->removeTab(QMC2_DOWNLOADS_INDEX);
#endif

  labelGameStatus->setVisible(FALSE);
  labelGameStatus->setPalette(qmc2StatusColorBlue);

#if !defined(QMC2_VARIANT_LAUNCHER)
  actionLaunchQMC2MAME->setVisible(FALSE);
  actionLaunchQMC2MESS->setVisible(FALSE);
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  actionLaunchQMC2MAME->setVisible(FALSE);
  qmc2MAWSCache.setMaxCost(QMC2_MAWS_CACHE_SIZE);
#if QT_VERSION < 0x040500
  actionClearMAWSCache->setVisible(FALSE);
#endif
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  actionLaunchQMC2MESS->setVisible(FALSE);
  actionClearMAWSCache->setVisible(FALSE);
  setWindowTitle(tr("M.E.S.S. Catalog / Launcher II"));
  menu_Tools->removeAction(actionCheckIcons);
  treeWidgetGamelist->headerItem()->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Machine / Attribute"));
  treeWidgetHierarchy->headerItem()->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Machine / Clones"));
  tabWidgetGamelist->setTabText(QMC2_GAMELIST_INDEX, tr("Machi&ne list"));
  tabWidgetGameDetail->setTabText(QMC2_GAMEINFO_INDEX, tr("Machine &info"));
  textBrowserGameInfo->setToolTip(tr("Detailed machine info"));
  textBrowserGameInfo->setStatusTip(tr("Detailed machine info"));
  listWidgetFavorites->setToolTip(tr("Favorite machines"));
  listWidgetFavorites->setStatusTip(tr("Favorite machines"));
  listWidgetPlayed->setToolTip(tr("Machines last played"));
  listWidgetPlayed->setStatusTip(tr("Machines last played"));
  treeWidgetEmulators->headerItem()->setText(QMC2_EMUCONTROL_COLUMN_MACHINE, tr("Machine / Notifier"));
  actionPlay->setToolTip(tr("Play current machine"));
  actionPlay->setStatusTip(tr("Play current machine"));
  actionToFavorites->setToolTip(tr("Add current machine to favorites"));
  actionToFavorites->setStatusTip(tr("Add current machine to favorites"));
  actionReload->setToolTip(tr("Reload entire machine list"));
  actionReload->setStatusTip(tr("Reload entire machine list"));
  actionViewFullDetail->setToolTip(tr("View machine list with full detail"));
  actionViewFullDetail->setStatusTip(tr("View machine list with full detail"));
  actionCheckCurrentROM->setToolTip(tr("Check current machine's ROM state"));
  actionCheckCurrentROM->setStatusTip(tr("Check current machine's ROM state"));
  actionAnalyseCurrentROM->setToolTip(tr("Analyse current machine with ROMAlyzer"));
  actionAnalyseCurrentROM->setStatusTip(tr("Analyse current machine with ROMAlyzer"));
  menu_Game->setTitle(tr("M&achine"));
  comboBoxViewSelect->setItemText(QMC2_VIEW_DETAIL_INDEX, tr("Machine list with full detail (filtered)"));
  comboBoxViewSelect->setToolTip(tr("Select between detailed machine list and parent / clone hierarchy"));
  comboBoxViewSelect->setStatusTip(tr("Select between detailed machine list and parent / clone hierarchy"));
  labelGameStatus->setToolTip(tr("Machine status indicator"));
  labelGameStatus->setStatusTip(tr("Machine status indicator"));
  actionCheckSamples->setEnabled(FALSE);
  actionCheckSamples->setVisible(FALSE);
  progressBarGamelist->setToolTip(tr("Progress indicator for machine list processing"));
  progressBarGamelist->setStatusTip(tr("Progress indicator for machine list processing"));
  qmc2Options->checkBoxGameStatusIndicator->setText(tr("Machine status indicator"));
  qmc2Options->checkBoxGameStatusIndicator->setToolTip(tr("Show vertical machine status indicator in machine details"));
  qmc2Options->checkBoxGameStatusIndicatorOnlyWhenRequired->setToolTip(tr("Show the machine status indicator only when the machine list is not visible due to the current layout"));
  qmc2Options->checkBoxShowGameName->setText(tr("Show machine name"));
  qmc2Options->checkBoxShowGameName->setToolTip(tr("Show machine's description at the bottom of any images"));
  qmc2Options->checkBoxShowGameNameOnlyWhenRequired->setToolTip(tr("Show machine's description only when the machine list is not visible due to the current layout"));
  treeWidgetGamelist->headerItem()->setText(QMC2_GAMELIST_COLUMN_ICON, tr("Value"));
  actionCheckIcons->setVisible(FALSE);
#endif

  qmc2Gamelist = new Gamelist(this);

  statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar").toBool());

  // game/machine image widgets
  QHBoxLayout *previewLayout = new QHBoxLayout, 
              *flyerLayout = new QHBoxLayout,
              *cabinetLayout = new QHBoxLayout,
              *controllerLayout = new QHBoxLayout,
              *marqueeLayout = new QHBoxLayout,
              *titleLayout = new QHBoxLayout;

  qmc2Preview = new Preview(tabPreview);
  previewLayout->addWidget(qmc2Preview);
  tabPreview->setLayout(previewLayout);
  qmc2Flyer = new Flyer(tabFlyer);
  flyerLayout->addWidget(qmc2Flyer);
  tabFlyer->setLayout(flyerLayout);
  qmc2Cabinet = new Cabinet(tabCabinet);
  cabinetLayout->addWidget(qmc2Cabinet);
  tabCabinet->setLayout(cabinetLayout);
  qmc2Controller = new Controller(tabController);
  controllerLayout->addWidget(qmc2Controller);
  tabController->setLayout(controllerLayout);
  qmc2Marquee = new Marquee(tabMarquee);
  marqueeLayout->addWidget(qmc2Marquee);
  tabMarquee->setLayout(marqueeLayout);
  qmc2Title = new Title(tabTitle);
  titleLayout->addWidget(qmc2Title);
  tabTitle->setLayout(titleLayout);

  // restore layout
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
    log(QMC2_LOG_FRONTEND, tr("restoring main widget layout"));
    menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", TRUE).toBool());
    QList<int> hSplitterSizes, vSplitterSizes;
    QSize hSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter").toSize();
    QSize vSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitter").toSize();
    if ( hSplitterSize.width() > 0 || hSplitterSize.height() > 0 )
      hSplitterSizes << hSplitterSize.width() << hSplitterSize.height();
    else
      hSplitterSizes << 100 << 100;
    if ( vSplitterSize.width() > 0 || vSplitterSize.height() > 0 )
      vSplitterSizes << vSplitterSize.width() << vSplitterSize.height();
    else
      vSplitterSizes << 100 << 100;
    hSplitter->setSizes(hSplitterSizes);
    vSplitter->setSizes(vSplitterSizes);
    tabWidgetGamelist->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistTab", 0).toInt());
    tabWidgetLogsAndEmulators->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/LogsAndEmulatorsTab", 0).toInt());
    int i;
    QVariantList defaultGamelistColumnWidths,
                 gamelistColumnWidths;
    defaultGamelistColumnWidths << 300 << 50 << 50 << 100;
    gamelistColumnWidths = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistColumnWidths", defaultGamelistColumnWidths).toList();
    for (i = 0; i < gamelistColumnWidths.count(); i++)
      treeWidgetGamelist->header()->resizeSection(i, gamelistColumnWidths[i].toInt());
    treeWidgetGamelist->header()->setDefaultAlignment(Qt::AlignLeft);
    QVariantList defaultHierarchyColumnWidths,
                 hierarchyColumnWidths;
    defaultHierarchyColumnWidths << 300 << 50 << 50 << 100;
    hierarchyColumnWidths = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyColumnWidths", defaultHierarchyColumnWidths).toList();
    for (i = 0; i < hierarchyColumnWidths.count(); i++)
      treeWidgetHierarchy->header()->resizeSection(i, hierarchyColumnWidths[i].toInt());
    treeWidgetHierarchy->header()->setDefaultAlignment(Qt::AlignLeft);
    treeWidgetGamelist->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistHeaderState").toByteArray());
    treeWidgetHierarchy->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyHeaderState").toByteArray());
    treeWidgetEmulators->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/EmulatorControlHeaderState").toByteArray());
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Visible").toBool() ) {
      on_actionCheckPreviews_activated();
      qmc2ImageChecker->tabWidgetImageChecker->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/CurrentTab", 0).toInt());
    }
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Visible").toBool() )
      on_actionCheckSamples_activated();
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Visible").toBool() )
      on_actionROMAlyzer_activated();
    actionFullscreenToggle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", FALSE).toBool());
    tabWidgetGamelist->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::North).toInt());
    tabWidgetGameDetail->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::North).toInt());
    tabWidgetLogsAndEmulators->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::North).toInt());
  } else {
    QList<int> splitterSizes;
    splitterSizes << 100 << 100;
    hSplitter->setSizes(splitterSizes);
    vSplitter->setSizes(splitterSizes);
  }
  on_actionFullscreenToggle_activated();

  // context menus
  QAction *action;
  QString s;

  qmc2EmulatorMenu = new QMenu(0);
  s = tr("Terminate selected emulator(s) (sends TERM signal to emulator process(es))");
  action = qmc2EmulatorMenu->addAction(tr("&Terminate"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/terminate.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_terminateEmulator_triggered()));
  s = tr("Kill selected emulator(s) (sends KILL signal to emulator process(es))");
  action = qmc2EmulatorMenu->addAction(tr("&Kill"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_killEmulator_triggered()));
  qmc2EmulatorMenu->addSeparator();
  s = tr("Copy emulator command line to clipboard");
  action = qmc2EmulatorMenu->addAction(tr("&Copy command"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_copyEmulatorCommand_triggered()));

  qmc2GameMenu = new QMenu(0);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Play selected game");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Start selected machine");
#endif
  action = qmc2GameMenu->addAction(tr("&Play"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_activated()));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Add current game to favorites");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Add current machine to favorites");
#endif
  action = qmc2GameMenu->addAction(tr("To &favorites"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_activated()));
  qmc2GameMenu->addSeparator();
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Check current game's ROM state");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Check current machine's ROM state");
#endif
  action = qmc2GameMenu->addAction(tr("Check &ROM state"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_activated()));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Analyse current game's ROM set with ROMAlyzer");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Analyse current machine's ROM set with ROMAlyzer");
#endif
  action = qmc2GameMenu->addAction(tr("&Analyse ROM..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/search.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_activated()));

  qmc2SearchMenu = new QMenu(0);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Play selected game");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Start selected machine");
#endif
  action = qmc2SearchMenu->addAction(tr("&Play"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_activated()));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Add current game to favorites");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Add current machine to favorites");
#endif
  action = qmc2SearchMenu->addAction(tr("To &favorites"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_activated()));
  qmc2SearchMenu->addSeparator();
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Check current game's ROM state");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Check current machine's ROM state");
#endif
  action = qmc2SearchMenu->addAction(tr("Check &ROM state"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_activated()));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Analyse current game's ROM set with ROMAlyzer");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Analyse current machine's ROM set with ROMAlyzer");
#endif
  action = qmc2SearchMenu->addAction(tr("&Analyse ROM..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/search.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_activated()));

  qmc2FavoritesMenu = new QMenu(0);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Play selected game");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Start selected machine");
#endif
  action = qmc2FavoritesMenu->addAction(tr("&Play"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_activated()));
  qmc2FavoritesMenu->addSeparator();
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Check current game's ROM state");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Check current machine's ROM state");
#endif
  action = qmc2FavoritesMenu->addAction(tr("Check &ROM state"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_activated()));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Analyse current game's ROM set with ROMAlyzer");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Analyse current machine's ROM set with ROMAlyzer");
#endif
  action = qmc2FavoritesMenu->addAction(tr("&Analyse ROM..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/search.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_activated()));
  qmc2FavoritesMenu->addSeparator();
  s = tr("Remove from favorites");
  action = qmc2FavoritesMenu->addAction(tr("&Remove"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/editdelete.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_removeFromFavorites_triggered()));
  s = tr("Clear all favorites");
  action = qmc2FavoritesMenu->addAction(tr("&Clear"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_clearAllFavorites_triggered()));
  s = tr("Save favorites now");
  action = qmc2FavoritesMenu->addAction(tr("&Save"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesave.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_saveFavorites_triggered()));

  qmc2PlayedMenu = new QMenu(0);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Play selected game");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Start selected machine");
#endif
  action = qmc2PlayedMenu->addAction(tr("&Play"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_activated()));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Add current game to favorites");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Add current machine to favorites");
#endif
  action = qmc2PlayedMenu->addAction(tr("To &favorites"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_activated()));
  qmc2PlayedMenu->addSeparator();
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Check current game's ROM state");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Check current machine's ROM state");
#endif
  action = qmc2PlayedMenu->addAction(tr("Check &ROM state"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_activated()));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  s = tr("Analyse current game's ROM set with ROMAlyzer");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  s = tr("Analyse current machine's ROM set with ROMAlyzer");
#endif
  action = qmc2PlayedMenu->addAction(tr("&Analyse ROM..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/search.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_activated()));
  qmc2PlayedMenu->addSeparator();
  s = tr("Remove from played");
  action = qmc2PlayedMenu->addAction(tr("&Remove"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/editdelete.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_removeFromPlayed_triggered()));
  s = tr("Clear all played");
  action = qmc2PlayedMenu->addAction(tr("&Clear"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_clearAllPlayed_triggered()));
  s = tr("Save play-history now");
  action = qmc2PlayedMenu->addAction(tr("&Save"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesave.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_savePlayed_triggered()));

  // tab widget position menus
  menuTabWidgetGamelist = new QMenu(0);
  s = tr("Set tab position north");
  action = menuTabWidgetGamelist->addAction(tr("&North"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGamelist_North_activated()));
  s = tr("Set tab position south");
  action = menuTabWidgetGamelist->addAction(tr("&South"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGamelist_South_activated()));
  s = tr("Set tab position west");
  action = menuTabWidgetGamelist->addAction(tr("&West"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGamelist_West_activated()));
  s = tr("Set tab position east");
  action = menuTabWidgetGamelist->addAction(tr("&East"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGamelist_East_activated()));

  menuTabWidgetGameDetail = new QMenu(0);
  s = tr("Set tab position north");
  action = menuTabWidgetGameDetail->addAction(tr("&North"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_North_activated()));
  s = tr("Set tab position south");
  action = menuTabWidgetGameDetail->addAction(tr("&South"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_South_activated()));
  s = tr("Set tab position west");
  action = menuTabWidgetGameDetail->addAction(tr("&West"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_West_activated()));
  s = tr("Set tab position east");
  action = menuTabWidgetGameDetail->addAction(tr("&East"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_East_activated()));
  menuTabWidgetGameDetail->addSeparator();
  s = tr("Detail setup");
  action = menuTabWidgetGameDetail->addAction(tr("&Setup..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_Setup_activated()));

  menuTabWidgetLogsAndEmulators = new QMenu(0);
  s = tr("Set tab position north");
  action = menuTabWidgetLogsAndEmulators->addAction(tr("&North"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetLogsAndEmulators_North_activated()));
  s = tr("Set tab position south");
  action = menuTabWidgetLogsAndEmulators->addAction(tr("&South"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetLogsAndEmulators_South_activated()));
  s = tr("Set tab position west");
  action = menuTabWidgetLogsAndEmulators->addAction(tr("&West"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetLogsAndEmulators_West_activated()));
  s = tr("Set tab position east");
  action = menuTabWidgetLogsAndEmulators->addAction(tr("&East"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetLogsAndEmulators_East_activated()));

  // other actions
  connect(actionViewFullDetail, SIGNAL(triggered()), this, SLOT(viewFullDetail()));
  connect(actionViewParentClones, SIGNAL(triggered()), this, SLOT(viewParentClones()));
  connect(comboBoxViewSelect, SIGNAL(currentIndexChanged(int)), stackedWidgetView, SLOT(setCurrentIndex(int)));
  connect(&searchTimer, SIGNAL(timeout()), this, SLOT(on_comboBoxSearch_textChanged_delayed()));
  connect(&updateTimer, SIGNAL(timeout()), this, SLOT(on_treeWidgetGamelist_itemSelectionChanged_delayed()));

  comboBoxViewSelect->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GamelistView", QMC2_GAMELIST_INDEX).toInt());
  if ( comboBoxViewSelect->currentIndex() == QMC2_VIEW_DETAIL_INDEX )
    tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/view_detail.png")));
  else
    tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/view_tree.png")));

  // restore toolbar state
  restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ToolbarState", QByteArray()).toByteArray());

#if QMC2_JOYSTICK == 1
  joyIndex = -1;
#endif

#if !(QMC2_USE_PHONON_API)
  tabWidgetLogsAndEmulators->removeTab(tabWidgetLogsAndEmulators->indexOf(tabAudioPlayer));
  menu_Tools->removeAction(menuAudio_player->menuAction());
#else
  phononAudioPlayer = new Phonon::MediaObject(this);
  phononAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
  Phonon::createPath(phononAudioPlayer, phononAudioOutput);
  QStringList psl = qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayList").toStringList();
  listWidgetAudioPlaylist->addItems(psl);
  QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->findItems(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/LastTrack", QString()).toString(), Qt::MatchExactly);
  if ( sl.count() > 0 ) listWidgetAudioPlaylist->setCurrentItem(sl[0]);
  checkBoxAudioPlayOnStart->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayOnStart", FALSE).toBool());
  checkBoxAudioShuffle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Shuffle", FALSE).toBool());
  checkBoxAudioPause->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Pause", TRUE).toBool());
  checkBoxAudioFade->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Fade", TRUE).toBool());
  sliderAudioVolume->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Volume", 50).toInt());
  phononAudioOutput->setVolume((qreal)sliderAudioVolume->value()/100.0);
  toolButtonAudioPreviousTrack->setDefaultAction(actionAudioPreviousTrack);
  toolButtonAudioNextTrack->setDefaultAction(actionAudioNextTrack);
  toolButtonAudioFastBackward->setDefaultAction(actionAudioFastBackward);
  toolButtonAudioFastForward->setDefaultAction(actionAudioFastForward);
  toolButtonAudioStopTrack->setDefaultAction(actionAudioStopTrack);
  toolButtonAudioPauseTrack->setDefaultAction(actionAudioPauseTrack);
  toolButtonAudioPlayTrack->setDefaultAction(actionAudioPlayTrack);
  phononAudioPlayer->setTickInterval(1000);
  connect(phononAudioPlayer, SIGNAL(tick(qint64)), this, SLOT(audioTick(qint64)));
  connect(phononAudioPlayer, SIGNAL(totalTimeChanged(qint64)), this, SLOT(audioTotalTimeChanged(qint64)));
  connect(phononAudioPlayer, SIGNAL(finished()), this, SLOT(audioFinished()));
  connect(phononAudioPlayer, SIGNAL(metaDataChanged()), this, SLOT(audioMetaDataChanged()));
  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = FALSE;
  if ( checkBoxAudioPlayOnStart->isChecked() )
    on_actionAudioPlayTrack_triggered();
  else
    on_actionAudioStopTrack_triggered();
#endif

  // setup ROM status filter selector menu & toggle actions / short cuts
  menuRomStatusFilter = new QMenu(pushButtonSelectRomFilter);
  actionRomStatusFilterC = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_green.png")), tr("&Correct"));
  actionRomStatusFilterC->setCheckable(TRUE);
  actionRomStatusFilterC->setShortcut(QKeySequence("Ctrl+Alt+C"));
  actionRomStatusFilterC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC").toBool());
  connect(actionRomStatusFilterC, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterC_toggled(bool)));
  actionRomStatusFilterM = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_yellowgreen.png")), tr("&Mostly correct"));
  actionRomStatusFilterM->setCheckable(TRUE);
  actionRomStatusFilterM->setShortcut(QKeySequence("Ctrl+Alt+M"));
  actionRomStatusFilterM->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM").toBool());
  connect(actionRomStatusFilterM, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterM_toggled(bool)));
  actionRomStatusFilterI = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_red.png")), tr("&Incorrect"));
  actionRomStatusFilterI->setCheckable(TRUE);
  actionRomStatusFilterI->setShortcut(QKeySequence("Ctrl+Alt+I"));
  actionRomStatusFilterI->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI").toBool());
  connect(actionRomStatusFilterI, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterI_toggled(bool)));
  actionRomStatusFilterN = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_grey.png")), tr("&Not found"));
  actionRomStatusFilterN->setCheckable(TRUE);
  actionRomStatusFilterN->setShortcut(QKeySequence("Ctrl+Alt+N"));
  actionRomStatusFilterN->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN").toBool());
  connect(actionRomStatusFilterN, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterN_toggled(bool)));
  actionRomStatusFilterU = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_blue.png")), tr("&Unknown"));
  actionRomStatusFilterU->setCheckable(TRUE);
  actionRomStatusFilterU->setShortcut(QKeySequence("Ctrl+Alt+U"));
  actionRomStatusFilterU->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU").toBool());
  connect(actionRomStatusFilterU, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterU_toggled(bool)));
  pushButtonSelectRomFilter->setMenu(menuRomStatusFilter);

  // initialize ROM state toggles
  qmc2StatesTogglesEnabled = FALSE;
  actionRomStatusFilterC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
  actionRomStatusFilterM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
  actionRomStatusFilterI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
  actionRomStatusFilterN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
  actionRomStatusFilterU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);

  // connect header click signals
  connect(treeWidgetGamelist->header(), SIGNAL(sectionClicked(int)), this, SLOT(on_treeWidgetGamelist_headerSectionClicked(int)));
  connect(treeWidgetHierarchy->header(), SIGNAL(sectionClicked(int)), this, SLOT(on_treeWidgetHierarchy_headerSectionClicked(int)));
  treeWidgetGamelist->header()->setClickable(TRUE);
  treeWidgetHierarchy->header()->setClickable(TRUE);

#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  treeWidgetHierarchy->hideColumn(QMC2_MACHINELIST_COLUMN_ICON);
  qApp->processEvents();
#endif

  // setup intial focus widget
  on_tabWidgetGamelist_currentChanged(tabWidgetGamelist->currentIndex());

  QTimer::singleShot(0, this, SLOT(init()));
}

MainWindow::~MainWindow()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::~MainWindow()");
#endif
}

void MainWindow::on_actionPlay_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionPlay_activated()");
#endif

  if ( qmc2EarlyReloadActive )
    return;

  if ( !qmc2CurrentItem )
    return;

  QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);

  if ( qmc2BiosROMs.contains(gameName) ) {
    // ROM is a BIOS and cannot be run...
    log(QMC2_LOG_FRONTEND, tr("sorry, BIOS ROMs cannot be run")); 
    return;
  }

  qmc2LastConfigItem = NULL;
  on_tabWidgetGameDetail_currentChanged(qmc2DetailSetup->appliedDetailList.indexOf(QMC2_CONFIG_INDEX));

  QStringList args;
  QString sectionTitle;

  foreach ( sectionTitle, qmc2EmulatorOptions->optionsMap.keys() ) {
    int i;
    for (i = 0; i < qmc2EmulatorOptions->optionsMap[sectionTitle].count(); i++) {
      EmulatorOption option = qmc2EmulatorOptions->optionsMap[sectionTitle][i];
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
      QString globalOptionKey = "MAME/Configuration/Global/" + option.name;
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
      QString globalOptionKey = "MESS/Configuration/Global/" + option.name;
#endif
      switch ( option.type ) {
        case QMC2_EMUOPT_TYPE_INT: {
          int  v = option.value.toInt();
          int dv = option.dvalue.toInt();
          int gv = qmc2Config->value(globalOptionKey, dv).toInt();
          if ( !option.valid )
            v = gv;
          if ( v != dv )
            args << QString("-%1").arg(option.name) << QString("%1").arg(v);
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT: {
          double  v = option.value.toDouble();
          double dv = option.dvalue.toDouble();
          double gv = qmc2Config->value(globalOptionKey, dv).toDouble();
          if ( !option.valid )
            v = gv;
          if ( v != dv )
            args << QString("-%1").arg(option.name) << QString("%1").arg(v);
          break;
        }

        case QMC2_EMUOPT_TYPE_BOOL: {
          QString dv = option.dvalue;
          QString  v = option.value;
          QString gv = qmc2Config->value(globalOptionKey, dv).toString();
          if ( !option.valid )
            v = gv;
          if ( v != dv ) {
            if ( v == "true" )
              args << QString("-%1").arg(option.name);
            else
              args << QString("-no%1").arg(option.name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_STRING:
        default: {
          QString  v = option.value;
          QString dv = option.dvalue;
          QString gv = qmc2Config->value(globalOptionKey, dv).toString();
          if ( !option.valid )
            v = gv;
          if ( v != dv )
            args << QString("-%1").arg(option.name) << v.replace("~", "$HOME");
          break;
        }
      }
    }
  }

  args << gameName;

#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  qmc2MessMachineName = gameName;
  if ( qmc2MESSDeviceConfigurator ) {
    QString configName = qmc2MESSDeviceConfigurator->lineEditConfigurationName->text();
    if ( configName != tr("No devices") ) {
      // make sure the currently edited data is up to date
      qmc2MESSDeviceConfigurator->on_pushButtonSaveConfiguration_clicked();
      if ( qmc2MESSDeviceConfigurator->configurationMap.contains(configName) ) {
        QPair<QStringList, QStringList> valuePair = qmc2MESSDeviceConfigurator->configurationMap[configName];
        int i;
        for (i = 0; i < valuePair.first.count(); i++)
#if defined(Q_WS_WIN)
          args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i].replace('/', '\\');
#else
          args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i].replace("~", "$HOME");
#endif
      }
    }
  }
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  QString command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  QString command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
#endif

  // start game
  qmc2ProcessManager->process(qmc2ProcessManager->start(command, args));

  // add game to played list
  listWidgetPlayed->blockSignals(TRUE);
  QList<QListWidgetItem *> matches = listWidgetPlayed->findItems(qmc2GamelistItemMap[gameName]->text(QMC2_GAMELIST_COLUMN_GAME), Qt::MatchExactly);
  QListWidgetItem *playedItem;
  if ( matches.count() > 0 ) {
    playedItem = listWidgetPlayed->takeItem(listWidgetPlayed->row(matches[0]));
  } else {
    playedItem = new QListWidgetItem();
    playedItem->setText(qmc2GamelistItemMap[gameName]->text(QMC2_GAMELIST_COLUMN_GAME));
  }
  listWidgetPlayed->insertItem(0, playedItem);
  listWidgetPlayed->setCurrentItem(playedItem);
  listWidgetPlayed->blockSignals(FALSE);
}

void MainWindow::on_hSplitter_splitterMoved(int pos, int index)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_hSplitter_splitterMoved(int pos = %1, int index = %2)").arg(pos).arg(index));
#endif

  // show / hide game status indicator
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator").toBool() ) {
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired").toBool() ) {
      if ( pos == 0 ) {
          if ( !labelGameStatus->isVisible() )
            labelGameStatus->setVisible(TRUE);
      } else {
        if ( labelGameStatus->isVisible() )
          labelGameStatus->setVisible(FALSE);
      }
    } else {
      if ( !labelGameStatus->isVisible() )
        labelGameStatus->setVisible(TRUE);
    }
  } else
    if ( labelGameStatus->isVisible() )
      labelGameStatus->setVisible(FALSE);
}

void MainWindow::on_actionToFavorites_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionToFavorites_activated()");
#endif

  if ( !qmc2CurrentItem )
    return;

  QString itemText = qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME);

  if ( itemText == tr("Waiting for data...") )
    return;

  QList<QListWidgetItem *> matches = listWidgetFavorites->findItems(itemText, Qt::MatchExactly);
  if ( matches.count() <= 0 ) {
    QListWidgetItem *item = new QListWidgetItem(listWidgetFavorites);
    item->setText(itemText);
    listWidgetFavorites->sortItems();
  }
}

void MainWindow::on_actionReload_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionReload_activated()");
#endif

  if ( qmc2FilterActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
    return;
  }

  if ( qmc2VerifyActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
    return;
  }

  if ( qmc2ImageCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
    return;
  }

  if ( qmc2SampleCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
    return;
  }

  if ( qmc2ROMAlyzerActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
    return;
  }

  if ( qmc2ReloadActive ) {
    log(QMC2_LOG_FRONTEND, tr("gamelist reload already active"));
  } else {
    qmc2StopParser = FALSE;
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB").toBool() )
      if ( qmc2GameInfoDB.isEmpty() && !qmc2StopParser )
        loadGameInfoDB();

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB").toBool() )
      if ( qmc2EmuInfoDB.isEmpty() && !qmc2StopParser )
        loadEmuInfoDB();
#endif

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection").toBool() && !qmc2StartingUp ) {
      if ( qmc2CurrentItem ) {
        log(QMC2_LOG_FRONTEND, tr("saving game selection"));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
        qmc2Config->setValue("MAME/SelectedGame", qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
        qmc2Config->setValue("MESS/SelectedGame", qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME));
#endif
      } else
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
        qmc2Config->remove("MAME/SelectedGame");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
        qmc2Config->remove("MESS/SelectedGame");
#endif
    }
    if ( !qmc2StopParser )
      qmc2Gamelist->load();
  }
}

void MainWindow::on_actionExitStop_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionExitStop_activated()");
#endif

  close();
}

void MainWindow::on_actionCheckCurrentROM_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckCurrentROM_activated()");
#endif

  if ( qmc2FilterActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
  } else if ( qmc2VerifyActive ) {
    log(QMC2_LOG_FRONTEND, tr("ROM verification already active"));
  } else if ( qmc2ReloadActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
  } else if ( qmc2ImageCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
  } else if ( qmc2SampleCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
  } else if ( qmc2ROMAlyzerActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
  } else
    qmc2Gamelist->verify(TRUE);
}

void MainWindow::on_actionCheckROMs_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckROMs_activated()");
#endif

  if ( qmc2FilterActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
  } else if ( qmc2VerifyActive ) {
    log(QMC2_LOG_FRONTEND, tr("ROM verification already active"));
  } else if ( qmc2ReloadActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
  } else if ( qmc2ImageCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
  } else if ( qmc2SampleCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
  } else if ( qmc2ROMAlyzerActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
  } else {
    if ( !qmc2Gamelist->autoROMCheck ) {
      switch ( QMessageBox::question(this,
                                     tr("Confirm"),
                                     tr("The ROM verification process may be very time-consuming.\nIt will overwrite existing cached data.\n\nDo you really want to check all ROM states now?"),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) ) {
          case QMessageBox::Yes:
            qmc2Gamelist->verify();
            break;

          default:
            break;
      }
    } else {
      if ( qmc2AutomaticReload )
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("automatic ROM check triggered"));
      qmc2Gamelist->verify();
    }
  }
  qmc2Gamelist->autoROMCheck = FALSE;
  qmc2AutomaticReload = FALSE;
}

void MainWindow::on_actionExportROMStatus_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionExportROMStatus_activated()");
#endif

  if ( !qmc2ROMStatusExporter )
    qmc2ROMStatusExporter = new ROMStatusExporter(this);

  if ( qmc2ROMStatusExporter->isHidden() )
    qmc2ROMStatusExporter->show();
  else if ( qmc2ROMStatusExporter->isMinimized() )
    qmc2ROMStatusExporter->showNormal();

  QTimer::singleShot(0, qmc2ROMStatusExporter, SLOT(raise()));
}

void MainWindow::on_actionCheckSamples_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckSamples_activated()");
#endif

  if ( !qmc2SampleChecker )
    qmc2SampleChecker = new SampleChecker(this);

  if ( qmc2SampleChecker->isHidden() )
    qmc2SampleChecker->show();
  else if ( qmc2SampleChecker->isMinimized() )
    qmc2SampleChecker->showNormal();

  QTimer::singleShot(0, qmc2SampleChecker, SLOT(raise()));
}

void MainWindow::on_actionCheckPreviews_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckPreviews_activated()");
#endif

  if ( !qmc2ImageChecker )
    qmc2ImageChecker = new ImageChecker(this);

  qmc2ImageChecker->tabWidgetImageChecker->setCurrentIndex(QMC2_PREVIEW_INDEX);
  if ( qmc2ImageChecker->isHidden() )
    qmc2ImageChecker->show();
  else if ( qmc2ImageChecker->isMinimized() )
    qmc2ImageChecker->showNormal();

  QTimer::singleShot(0, qmc2ImageChecker, SLOT(raise()));
}

void MainWindow::on_actionCheckFlyers_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckFlyers_activated()");
#endif

  if ( !qmc2ImageChecker )
    qmc2ImageChecker = new ImageChecker(this);

  qmc2ImageChecker->tabWidgetImageChecker->setCurrentIndex(QMC2_FLYER_INDEX);
  if ( qmc2ImageChecker->isHidden() )
    qmc2ImageChecker->show();
  else if ( qmc2ImageChecker->isMinimized() )
    qmc2ImageChecker->showNormal();

  QTimer::singleShot(0, qmc2ImageChecker, SLOT(raise()));
}

void MainWindow::on_actionCheckIcons_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckIcons_activated()");
#endif

  if ( !qmc2ImageChecker )
    qmc2ImageChecker = new ImageChecker(this);

  qmc2ImageChecker->tabWidgetImageChecker->setCurrentIndex(QMC2_ICON_INDEX);
  if ( qmc2ImageChecker->isHidden() )
    qmc2ImageChecker->show();
  else if ( qmc2ImageChecker->isMinimized() )
    qmc2ImageChecker->showNormal();

  QTimer::singleShot(0, qmc2ImageChecker, SLOT(raise()));
}

void MainWindow::on_actionROMAlyzer_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionROMAlyzer_activated()");
#endif

  if ( !qmc2ROMAlyzer )
    qmc2ROMAlyzer = new ROMAlyzer(this);

  if ( !qmc2ROMAlyzerActive )
    qmc2ROMAlyzer->lineEditGames->setText("*");

  if ( qmc2ROMAlyzer->isHidden() )
    qmc2ROMAlyzer->show();
  else if ( qmc2ROMAlyzer->isMinimized() )
    qmc2ROMAlyzer->showNormal();

  QTimer::singleShot(0, qmc2ROMAlyzer, SLOT(raise()));
}

void MainWindow::on_actionAnalyseCurrentROM_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAnalyseCurrentROM_activated()");
#endif

  if ( !qmc2CurrentItem )
    return;

  if ( qmc2ROMAlyzerActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
    return;
  }

  if ( !qmc2ROMAlyzer )
    qmc2ROMAlyzer = new ROMAlyzer(0);

  qmc2ROMAlyzer->lineEditGames->setText(qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON));

  if ( qmc2ROMAlyzer->isHidden() )
    qmc2ROMAlyzer->show();
  else if ( qmc2ROMAlyzer->isMinimized() )
    qmc2ROMAlyzer->showNormal();

  QTimer::singleShot(0, qmc2ROMAlyzer, SLOT(raise()));
  QTimer::singleShot(0, qmc2ROMAlyzer->pushButtonAnalyze, SLOT(animateClick()));
}

void MainWindow::on_actionClearImageCache_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearImageCache_activated()");
#endif

  QPixmapCache::clear();
  log(QMC2_LOG_FRONTEND, tr("image cache cleared"));
}

void MainWindow::on_actionClearIconCache_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearIconCache_activated()");
#endif

  qmc2IconMap.clear();
  qmc2IconsPreloaded = FALSE;
  log(QMC2_LOG_FRONTEND, tr("icon cache cleared"));
}

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
void MainWindow::on_actionClearMAWSCache_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearMAWSCache_activated()");
#endif

  QString cacheStatus = tr("freed %n byte(s) in %1", "", qmc2MAWSCache.totalCost()).arg(tr("%n entry(s)", "", qmc2MAWSCache.count()));
  qmc2MAWSCache.clear();
  log(QMC2_LOG_FRONTEND, tr("MAWS in-memory cache cleared (%1)").arg(cacheStatus));
  QDir mawsCacheDir(qmc2Config->value("MAME/FilesAndDirectories/MAWSCacheDirectory").toString());
  qulonglong removedBytes = 0;
  qulonglong removedFiles = 0;
  if ( mawsCacheDir.exists() ) {
    QStringList webCacheFiles = mawsCacheDir.entryList(QStringList("*.wc"));
    foreach (QString webCacheFile, webCacheFiles) {
      QFileInfo fi(mawsCacheDir.filePath(webCacheFile));
      qint64 fSize = fi.size();
      if ( mawsCacheDir.remove(webCacheFile) ) {
        removedBytes += fSize;
        removedFiles++;
      }
      qApp->processEvents();
    }
  }
  cacheStatus = tr("removed %n byte(s) in %1", "", removedBytes).arg(tr("%n file(s)", "", removedFiles));
  log(QMC2_LOG_FRONTEND, tr("MAWS on-disk cache cleared (%1)").arg(cacheStatus));
}
#endif

void MainWindow::on_actionRecreateTemplateMap_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionRecreateTemplateMap_activated()");
#endif

  if ( qmc2GlobalEmulatorOptions != NULL )
    qmc2GlobalEmulatorOptions->createTemplateMap();
}

void MainWindow::on_actionOptions_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionOptions_activated()");
#endif

  if ( qmc2Options->isHidden() )
    qmc2Options->show();
  else if ( qmc2Options->isMinimized() )
    qmc2Options->showNormal();

  QTimer::singleShot(0, qmc2Options, SLOT(raise()));
}

void MainWindow::on_actionFullscreenToggle_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionFullscreenToggle_activated()");
#endif

  if ( qmc2ArcadeView )
    if ( qmc2ArcadeView->isActiveWindow() ) {
      qmc2ArcadeView->raise();
      qApp->processEvents();
      qmc2ArcadeView->toggleFullscreen();
      if ( windowState() & Qt::WindowFullScreen )
        actionFullscreenToggle->setChecked(TRUE);
      else
        actionFullscreenToggle->setChecked(FALSE);
      return;
    }

  if ( !qmc2EarlyStartup ) {
    // saftey checks
    if ( windowState() & Qt::WindowFullScreen )
      actionFullscreenToggle->setChecked(FALSE);
    else
      actionFullscreenToggle->setChecked(TRUE);
  }

  qApp->processEvents();

  if ( actionFullscreenToggle->isChecked() ) {
    if ( isVisible() && !(windowState() & Qt::WindowFullScreen) ) {
      if ( isMaximized() ) {
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", TRUE);
      } else {
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry", saveGeometry());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position", pos());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size", size());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", FALSE);
      }
    }
    showFullScreen();
  } else {
    hide();
    qApp->processEvents();

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", FALSE).toBool() ) {
        showMaximized();
      } else {
        if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size") )
          resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size").toSize());
        if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position") )
          move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position").toPoint());
        if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry") )
          restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry").toByteArray());
        showNormal();
      }
    } else {
      resize(640, 480);
      move((qApp->desktop()->width() - width()) / 2, (qApp->desktop()->height() - height()) / 2);
      showNormal();
    }
  }
  raise();
  qApp->processEvents();
}

void MainWindow::on_actionLaunchQMC2MAME_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: on_actionLaunchQMC2MAME_activated()");
#endif

  if ( !qmc2VariantSwitchReady )
    return;

  bool launched = false;

#if defined(Q_WS_MAC)
  OSStatus err;
  FSRef appRef;
  err = LSFindApplicationForInfo(kLSUnknownCreator, CFSTR(QMC2_VARIANT_SDLMAME_BUNDLE_ID), NULL, &appRef, NULL);
  if ( err == noErr ) {
    err = LSOpenFSRef(&appRef, NULL);
  }
  launched = err == noErr;
#else
  QStringList args;
  args << QMC2_VARIANT_SDLMAME_NAME << QMC2_VARIANT_SDLMAME_TITLE << QMC2_VARIANT_SDLMAME_NAME;
  launched = QProcess::startDetached(QMC2_COMMAND_RUNONCE, args);
#endif

  if ( launched ) {
    log(QMC2_LOG_FRONTEND, tr("variant '%1' launched").arg(QMC2_VARIANT_SDLMAME_NAME));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnVariantLaunch").toBool() ) {
      if ( qmc2Options )
        if ( qmc2Options->isVisible() )
          qmc2Options->showMinimized();
      if ( qmc2ROMAlyzer )
        if ( qmc2ROMAlyzer->isVisible() )
          qmc2ROMAlyzer->showMinimized();
      if ( qmc2ImageChecker )
        if ( qmc2ImageChecker->isVisible() )
          qmc2ImageChecker->showMinimized();
      if ( qmc2SampleChecker )
        if ( qmc2SampleChecker->isVisible() )
          qmc2SampleChecker->showMinimized();
      if ( qmc2ArcadeView )
        if ( qmc2ArcadeView->isVisible() )
          qmc2ArcadeView->close();
      if ( qmc2ArcadeSetupDialog )
        if ( qmc2ArcadeSetupDialog->isVisible() )
          qmc2ArcadeSetupDialog->showMinimized();
      showMinimized();
    }
  } else
    log(QMC2_LOG_FRONTEND, tr("WARNING: failed to launch variant '%1'").arg(QMC2_VARIANT_SDLMAME_NAME));
}

void MainWindow::on_actionLaunchQMC2MESS_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: on_actionLaunchQMC2MESS_activated()");
#endif

  if ( !qmc2VariantSwitchReady )
    return;

  bool launched = false;

#if defined(Q_WS_MAC)
  OSStatus err;
  FSRef appRef;
  err = LSFindApplicationForInfo(kLSUnknownCreator, CFSTR(QMC2_VARIANT_SDLMESS_BUNDLE_ID), NULL, &appRef, NULL);
  if ( err == noErr ) {
    err = LSOpenFSRef(&appRef, NULL);
  }
  launched = err == noErr;
#else
  QStringList args;
  args << QMC2_VARIANT_SDLMESS_NAME << QMC2_VARIANT_SDLMESS_TITLE << QMC2_VARIANT_SDLMESS_NAME;
  launched = QProcess::startDetached(QMC2_COMMAND_RUNONCE, args);
#endif

  if ( launched ) {
    log(QMC2_LOG_FRONTEND, tr("variant '%1' launched").arg(QMC2_VARIANT_SDLMESS_NAME));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnVariantLaunch").toBool() ) {
      if ( qmc2Options )
        if ( qmc2Options->isVisible() )
          qmc2Options->showMinimized();
      if ( qmc2ROMAlyzer )
        if ( qmc2ROMAlyzer->isVisible() )
          qmc2ROMAlyzer->showMinimized();
      if ( qmc2ImageChecker )
        if ( qmc2ImageChecker->isVisible() )
          qmc2ImageChecker->showMinimized();
      if ( qmc2SampleChecker )
        if ( qmc2SampleChecker->isVisible() )
          qmc2SampleChecker->showMinimized();
      if ( qmc2ArcadeView )
        if ( qmc2ArcadeView->isVisible() )
          qmc2ArcadeView->close();
      if ( qmc2ArcadeSetupDialog )
        if ( qmc2ArcadeSetupDialog->isVisible() )
          qmc2ArcadeSetupDialog->showMinimized();
      showMinimized();
    }
  } else
    log(QMC2_LOG_FRONTEND, tr("WARNING: failed to launch variant '%1'").arg(QMC2_VARIANT_SDLMESS_NAME));
}

void MainWindow::on_actionDocumentation_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionDocumentation_activated()");
#endif

  if ( !qmc2DocBrowser ) {
    qmc2DocBrowser = new DocBrowser(this);
    QStringList searchPaths;
    searchPaths << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString() +
                   "doc/html/" +
                   qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language").toString();
    qmc2DocBrowser->manualBrowser->setSearchPaths(searchPaths);
    qmc2DocBrowser->manualBrowser->setSource(QUrl("index.html"));
  }

  qmc2DocBrowser->show();
  qmc2DocBrowser->raise();
}

void MainWindow::on_actionAbout_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAbout_activated()");
#endif

  if ( !qmc2About )
    qmc2About = new About(this);

  qmc2About->show();
  qmc2About->raise();
}

void MainWindow::on_actionAboutQt_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAboutQt_activated()");
#endif

  QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::on_actionArcadeSetup_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionArcadeSetup_activated()");
#endif

  log(QMC2_LOG_FRONTEND, tr("WARNING: this feature is not yet working!"));

  if ( !qmc2ArcadeSetupDialog )
    qmc2ArcadeSetupDialog = new ArcadeSetupDialog(this);

  qmc2ArcadeSetupDialog->show();
  qmc2ArcadeSetupDialog->raise();
}

void MainWindow::on_actionArcadeToggle_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionArcadeToggle_activated()");
#endif

  if ( qmc2DestroyingArcadeView )
    return;

  if ( !qmc2ArcadeView ) {
    log(QMC2_LOG_FRONTEND, tr("WARNING: this feature is not yet working!"));
    qmc2ArcadeView = new ArcadeView(0);
  }

  if ( qmc2ArcadeView->isVisible() ) {
    qmc2DestroyingArcadeView = TRUE;
    qmc2ArcadeView->close();
    QTimer::singleShot(0, this, SLOT(destroyArcadeView()));
  } else {
    qmc2ArcadeView->show();
    qmc2ArcadeView->raise();
  }
}

void MainWindow::destroyArcadeView()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::destroyArcadeView()");
#endif

  if ( qmc2ArcadeView ) {
    delete qmc2ArcadeView;
    qmc2ArcadeView = NULL;
    qmc2DestroyingArcadeView = FALSE;
  }
}

void MainWindow::on_comboBoxSearch_textChanged(QString)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_comboBoxSearch_textChanged(QString)");
#endif

  searchTimer.start(QMC2_SEARCH_DELAY);
}

void MainWindow::on_comboBoxSearch_textChanged_delayed()
{
  searchTimer.stop();

  QString pattern = comboBoxSearch->currentText();

  // easy pattern match
  if ( !pattern.isEmpty() ) {
    pattern = "*" + pattern.replace(' ', "* *") + "*";
    pattern.replace(QString("*^"), "");
    pattern.replace(QString("$*"), "");
  }

  listWidgetSearch->clear();
  QList<QTreeWidgetItem *> matches = treeWidgetGamelist->findItems(pattern, Qt::MatchContains | Qt::MatchWildcard);
  
  int i;
  for (i = 0; i < matches.count(); i++) {
    QListWidgetItem *item = new QListWidgetItem(listWidgetSearch);
    item->setText(matches.at(i)->text(QMC2_GAMELIST_COLUMN_GAME));
  }

  qmc2Gamelist->numSearchGames = matches.count();
  labelGamelistStatus->setText(qmc2Gamelist->status());

#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_comboBoxSearch_textChanged_delayed(QString pattern = \"" + pattern + "\"): matches.count() = " + QString::number(matches.count()));
#endif

  QTimer::singleShot(0, this, SLOT(checkCurrentSearchSelection()));
}

void MainWindow::on_comboBoxSearch_activated(QString pattern)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_comboBoxSearch_activated(QString pattern = \"" + pattern + "\")");
#endif

  on_comboBoxSearch_textChanged_delayed();
  if ( tabWidgetGamelist->currentWidget() != tabSearch )
    tabWidgetGamelist->setCurrentWidget(tabSearch);
  QTimer::singleShot(0, listWidgetSearch, SLOT(setFocus()));
}

void MainWindow::on_listWidgetSearch_currentTextChanged(QString s)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetSearch_currentTextChanged(QString s = \""+ s + "\")");
#endif

  QTreeWidgetItem *matchItem = qmc2GamelistItemByDescriptionMap[s];
  if ( matchItem ) {
    qmc2CheckItemVisibility = FALSE;
    treeWidgetGamelist->clearSelection();
    treeWidgetGamelist->setCurrentItem(matchItem);
    qmc2CurrentItem = matchItem;
  }
#ifdef QMC2_DEBUG
  else
    log(QMC2_LOG_FRONTEND, "DEBUG: ERROR: no match found (?)");
#endif
}

void MainWindow::on_listWidgetSearch_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_listWidgetSearch_currentItemChanged(QListWidgetItem *current = %1, QListWidgetItem *previous = %2)").arg((qulonglong)current).arg((qulonglong)previous));
#endif

  QTreeWidgetItem *glItem = NULL;
  if ( current )
    glItem = qmc2GamelistItemByDescriptionMap[current->text()];
  if ( glItem ) {
    qmc2CheckItemVisibility = FALSE;
    treeWidgetGamelist->clearSelection();
    qmc2CurrentItem = glItem;
    treeWidgetGamelist->setCurrentItem(glItem);
    processEvents();
  }
}

void MainWindow::on_listWidgetSearch_itemPressed(QListWidgetItem *current)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_listWidgetSearch_itemPressed(QListWidgetItem *current = %1)").arg((qulonglong)current));
#endif

  on_listWidgetSearch_currentItemChanged(current, NULL);
}

void MainWindow::on_listWidgetSearch_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetSearch_itemSelectionChanged()");
#endif

  QList<QListWidgetItem *> selected = listWidgetSearch->selectedItems();
  if ( selected.count() > 0 )
    on_listWidgetSearch_currentItemChanged(selected[0], NULL);
}

void MainWindow::on_listWidgetSearch_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetSearch_itemActivated(QListWidgetItem *item = 0x"+ QString::number((ulong)item, 16) + ")");
#endif

  if ( item == NULL )
    return;

  QTreeWidgetItem *matchItem = qmc2GamelistItemByDescriptionMap[item->text()];
  if ( matchItem ) {
    qmc2CheckItemVisibility = FALSE;
    tabWidgetGamelist->setCurrentIndex(0);
    treeWidgetGamelist->clearSelection();
    treeWidgetGamelist->setCurrentItem(matchItem);
    treeWidgetGamelist->scrollToItem(matchItem, QAbstractItemView::PositionAtTop);
    qmc2CurrentItem = matchItem;
    if ( !qmc2ReloadActive )
      treeWidgetGamelist->expandItem(matchItem);
  }
  else
    log(QMC2_LOG_FRONTEND, tr("ERROR: no match found (?)"));
}

void MainWindow::on_listWidgetFavorites_currentTextChanged(QString s)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetFavorites_currentTextChanged(QString s = \""+ s + "\")");
#endif

  on_listWidgetSearch_currentTextChanged(s);
}

void MainWindow::on_listWidgetFavorites_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetFavorites_itemSelectionChanged()");
#endif

  QList<QListWidgetItem *> selected = listWidgetFavorites->selectedItems();
  if ( selected.count() > 0 )
    on_listWidgetSearch_currentItemChanged(selected[0], NULL);
}

void MainWindow::on_listWidgetFavorites_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetFavorites_itemActivated(QListWidgetItem *item = 0x"+ QString::number((ulong)item, 16) + ")");
#endif

  on_listWidgetSearch_itemActivated(item);
}

void MainWindow::on_listWidgetPlayed_currentTextChanged(QString s)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetPlayed_currentTextChanged(QString s = \""+ s + "\")");
#endif

  on_listWidgetSearch_currentTextChanged(s);
}

void MainWindow::on_listWidgetPlayed_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetPlayed_itemSelectionChanged()");
#endif

  QList<QListWidgetItem *> selected = listWidgetPlayed->selectedItems();
  if ( selected.count() > 0 )
    on_listWidgetSearch_currentItemChanged(selected[0], NULL);
}

void MainWindow::on_listWidgetPlayed_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetPlayed_itemActivated(QListWidgetItem *item = 0x"+ QString::number((ulong)item, 16) + ")");
#endif

  on_listWidgetSearch_itemActivated(item);
}

void MainWindow::on_tabWidgetGamelist_currentChanged(int currentIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetGamelist_currentChanged(int i = " + QString::number(currentIndex) + ")");
#endif

  switch ( currentIndex ) {
    case QMC2_GAMELIST_INDEX:
      QTimer::singleShot(0, this, SLOT(scrollToCurrentItem()));
      if ( stackedWidgetView->currentIndex() == QMC2_VIEW_DETAIL_INDEX )
        treeWidgetGamelist->setFocus();
      else
        treeWidgetHierarchy->setFocus();
      break;

    case QMC2_SEARCH_INDEX:
      QTimer::singleShot(0, this, SLOT(checkCurrentSearchSelection()));
      listWidgetSearch->setFocus();
      break;

    case QMC2_FAVORITES_INDEX:
      QTimer::singleShot(0, this, SLOT(checkCurrentFavoritesSelection()));
      break;

    case QMC2_PLAYED_INDEX:
      QTimer::singleShot(0, this, SLOT(checkCurrentPlayedSelection()));
      break;

    default:
      break;
  }

  qmc2Preview->repaint();
  qmc2Flyer->repaint();
  qmc2Cabinet->repaint();
  qmc2Controller->repaint();
  qmc2Marquee->repaint();
  qmc2Title->repaint();

  // show / hide game status indicator
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator").toBool() ) {
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired").toBool() ) {
      if ( currentIndex != QMC2_GAMELIST_INDEX ) {
        labelGameStatus->setVisible(TRUE);
      } else {
        labelGameStatus->setVisible(FALSE);
      }
    } else {
      labelGameStatus->setVisible(TRUE);
    }
  } else
    labelGameStatus->setVisible(FALSE);
}

void MainWindow::scrollToCurrentItem()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::scrollToCurrentItem()");
#endif

  QTreeWidgetItem *ci = treeWidgetGamelist->currentItem();

  if ( qmc2CurrentItem )
    ci = qmc2CurrentItem;

  if ( ci ) {
    if ( ci->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
      return;

    switch ( stackedWidgetView->currentIndex() ) {
      case QMC2_VIEWHIERARCHY_INDEX:
        ci = qmc2HierarchyItemMap[ci->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)];
        if ( ci ) {
          treeWidgetHierarchy->clearSelection();
          treeWidgetHierarchy->setCurrentItem(ci);
          treeWidgetHierarchy->scrollToItem(ci, QAbstractItemView::PositionAtTop);
        }
        break;

      case QMC2_VIEWGAMELIST_INDEX:
      default:
        qmc2CheckItemVisibility = FALSE;
        treeWidgetGamelist->clearSelection();
        if ( !qmc2ReloadActive )
          treeWidgetGamelist->setCurrentItem(ci);
        treeWidgetGamelist->scrollToItem(ci, QAbstractItemView::PositionAtTop);
        break;
    }
    if ( !qmc2ReloadActive )
      ci->setSelected(TRUE);
  }
}

void MainWindow::checkCurrentSearchSelection()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::checkCurrentSearchSelection()");
#endif

  listWidgetSearch->blockSignals(TRUE);
  listWidgetSearch->setCurrentIndex(QModelIndex());
  listWidgetSearch->clearSelection();

  if ( !qmc2CurrentItem )
    return;

  QList<QListWidgetItem *> searchMatches = listWidgetSearch->findItems(qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME), Qt::MatchExactly);
  if ( searchMatches.count() > 0 ) {
    QListWidgetItem *matchedItem = searchMatches[0];
    if ( matchedItem != NULL ) {
      listWidgetSearch->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
      listWidgetSearch->scrollToItem(matchedItem, QAbstractItemView::PositionAtTop);
      qApp->processEvents();
    }
  }

  listWidgetSearch->blockSignals(FALSE);
}

void MainWindow::checkCurrentFavoritesSelection()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::checkCurrentFavoritesSelection()");
#endif

  listWidgetFavorites->blockSignals(TRUE);
  listWidgetFavorites->setCurrentIndex(QModelIndex());
  listWidgetFavorites->clearSelection();

  if ( !qmc2CurrentItem )
    return;

  QList<QListWidgetItem *> favoritesMatches = listWidgetFavorites->findItems(qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME), Qt::MatchExactly);
  if ( favoritesMatches.count() > 0 ) {
    QListWidgetItem *matchedItem = favoritesMatches[0];
    if ( matchedItem != NULL ) {
      listWidgetFavorites->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
      listWidgetFavorites->scrollToItem(matchedItem, QAbstractItemView::PositionAtTop);
      qApp->processEvents();
    }
  }

  listWidgetFavorites->setFocus();
  listWidgetFavorites->blockSignals(FALSE);
}

void MainWindow::checkCurrentPlayedSelection()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::checkCurrentPlayedSelection()");
#endif

  listWidgetPlayed->blockSignals(TRUE);
  listWidgetPlayed->setCurrentIndex(QModelIndex());
  listWidgetPlayed->clearSelection();

  if ( !qmc2CurrentItem )
    return;

  QList<QListWidgetItem *> playedMatches = listWidgetPlayed->findItems(qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME), Qt::MatchExactly);
  if ( playedMatches.count() > 0 ) {
    QListWidgetItem *matchedItem = playedMatches[0];
    if ( matchedItem != NULL ) {
      listWidgetPlayed->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
      listWidgetPlayed->scrollToItem(matchedItem, QAbstractItemView::PositionAtTop);
      qApp->processEvents();
    }
  }

  listWidgetPlayed->setFocus();
  listWidgetPlayed->blockSignals(FALSE);
}

void MainWindow::on_tabWidgetGameDetail_currentChanged(int currentIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetGameDetail_currentChanged(int currentIndex = " + QString::number(currentIndex) + ")");
#endif

  if ( !qmc2CurrentItem || qmc2EarlyReloadActive ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    qmc2LastGameInfoItem = qmc2LastEmuInfoItem = qmc2LastConfigItem = qmc2LastDeviceConfigItem = NULL;
#else
    qmc2LastGameInfoItem = qmc2LastConfigItem = qmc2LastDeviceConfigItem = NULL;
#endif
#if QMC2_OPENGL == 1
    // images painted through OpenGL need extra "clear()'s", otherwise garbage is displayed
    switch ( qmc2DetailSetup->appliedDetailList[currentIndex] ) {
      case QMC2_PREVIEW_INDEX: {
          QPainter pPreview(qmc2Preview);
          qmc2Preview->drawCenteredImage(0, &pPreview);
        }
        break;

      case QMC2_FLYER_INDEX: {
          QPainter pFlyer(qmc2Flyer);
          qmc2Flyer->drawCenteredImage(0, &pFlyer);
        }

      case QMC2_CABINET_INDEX: {
          QPainter pCabinet(qmc2Cabinet);
          qmc2Cabinet->drawCenteredImage(0, &pCabinet);
        }

      case QMC2_CONTROLLER_INDEX: {
          QPainter pController(qmc2Controller);
          qmc2Controller->drawCenteredImage(0, &pController);
        }

      case QMC2_MARQUEE_INDEX: {
          QPainter pMarquee(qmc2Marquee);
          qmc2Marquee->drawCenteredImage(0, &pMarquee);
        }

      case QMC2_TITLE_INDEX: {
          QPainter pTitle(qmc2Title);
          qmc2Title->drawCenteredImage(0, &pTitle);
        }

      default:
        break;
    }
#endif
    return;
  }

  // paranoia :)
  QTreeWidgetItem *ci = treeWidgetGamelist->currentItem();
  if ( !ci )
    return;
  if ( !ci->isSelected() )
    return;
  QTreeWidgetItem *topLevelItem = ci;
  while ( topLevelItem->parent() )
    topLevelItem = topLevelItem->parent();
  if ( topLevelItem )
    qmc2CurrentItem = topLevelItem;
  else
    return;
  if ( qmc2CurrentItem->childCount() <= 0 )
    return;
  if ( qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON) == tr("Waiting for data...") )
    return;
 
  // show / hide game status indicator
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator").toBool() ) {
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired").toBool() ) {
      if ( hSplitter->sizes()[0] == 0 || tabWidgetGamelist->currentIndex() != QMC2_GAMELIST_INDEX) {
        labelGameStatus->setVisible(TRUE);
      } else {
        labelGameStatus->setVisible(FALSE);
      }
    } else {
      labelGameStatus->setVisible(TRUE);
    }
  } else
    labelGameStatus->setVisible(FALSE);

  // setup status indicator color
  switch ( qmc2CurrentItem->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() ) {
    case QMC2_ROMSTATE_CHAR_C:
      labelGameStatus->setPalette(qmc2StatusColorGreen);
      break;

    case QMC2_ROMSTATE_CHAR_M:
      labelGameStatus->setPalette(qmc2StatusColorYellowGreen);
      break;

    case QMC2_ROMSTATE_CHAR_I:
      labelGameStatus->setPalette(qmc2StatusColorRed);
      break;

    case QMC2_ROMSTATE_CHAR_N:
      labelGameStatus->setPalette(qmc2StatusColorGrey);
      break;

    case QMC2_ROMSTATE_CHAR_U:
      labelGameStatus->setPalette(qmc2StatusColorBlue);
    default:
      break;
  }

  switch ( qmc2DetailSetup->appliedDetailList[currentIndex] ) {
#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    case QMC2_DEVICE_INDEX:
      if ( qmc2CurrentItem != qmc2LastDeviceConfigItem ) {
        tabDevices->setUpdatesEnabled(FALSE);
        if ( qmc2MESSDeviceConfigurator ) {
          qmc2MESSDeviceConfigurator->save();
          QLayout *vbl = tabDevices->layout();
          if ( vbl )
            delete vbl;
          delete qmc2MESSDeviceConfigurator;
          qmc2MESSDeviceConfigurator = NULL;
        }
        QString machineName = qmc2CurrentItem->child(0)->text(QMC2_MACHINELIST_COLUMN_ICON);
        QVBoxLayout *layout = new QVBoxLayout;
        qmc2MESSDeviceConfigurator = new MESSDeviceConfigurator(machineName, tabDevices);
        qmc2MESSDeviceConfigurator->load();
        layout->addWidget(qmc2MESSDeviceConfigurator);
        tabDevices->setLayout(layout);
        qmc2MESSDeviceConfigurator->show();
        qmc2LastDeviceConfigItem = qmc2CurrentItem;
        tabDevices->setUpdatesEnabled(TRUE);
      }
      break;
#elif defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    case QMC2_MAWS_INDEX:
      if ( qmc2CurrentItem != qmc2LastMAWSItem ) {
        tabMAWS->setUpdatesEnabled(FALSE);
        if ( qmc2MAWSLookup ) {
          QLayout *vbl = tabMAWS->layout();
          if ( vbl )
            delete vbl;
          delete qmc2MAWSLookup;
          qmc2MAWSLookup = NULL;
        }
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        QVBoxLayout *layout = new QVBoxLayout;
        qmc2MAWSLookup = new MiniWebBrowser(tabMAWS);
        qmc2MAWSLookup->webViewBrowser->settings()->setFontFamily(QWebSettings::StandardFont, qApp->font().family());
        qmc2MAWSLookup->webViewBrowser->settings()->setFontSize(QWebSettings::MinimumFontSize, qApp->font().pointSize());
        qmc2MAWSLookup->webViewBrowser->setStatusTip(tr("MAWS page for '%1'").arg(qmc2GamelistDescriptionMap[gameName]));
        layout->addWidget(qmc2MAWSLookup);
        tabMAWS->setLayout(layout);
        QString mawsUrl = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName);

        // lookup in disk cache first
        bool foundInDiskCache = FALSE;
        QDir mawsCacheDir(qmc2Config->value("MAME/FilesAndDirectories/MAWSCacheDirectory").toString());
        if ( mawsCacheDir.exists(gameName + ".wc") ) {
          QFile mawsCacheFile(mawsCacheDir.filePath(gameName + ".wc"));
          if ( mawsCacheFile.open(QIODevice::ReadOnly) ) {
            QTextStream ts(&mawsCacheFile);
            ts.readLine();
            QString mawsCacheAge = ts.readLine().split('\t')[1];
            if ( QDateTime::currentDateTime().toTime_t() - mawsCacheAge.toULong() < QMC2_MAWS_MAX_CACHE_AGE ) {
              if ( !qmc2MAWSCache.contains(gameName) ) {
#if defined(QMC2_WC_COMPRESSION_ENABLED)
                QString mawsCacheData = ts.read(QMC2_ONE_MEGABYTE);
                qmc2MAWSLookup->webViewBrowser->setHtml(QString(qUncompress(mawsCacheData.toLatin1())), QUrl(mawsUrl));
#else
                QString mawsCacheData = ts.read(16 * QMC2_ONE_MEGABYTE);
                qmc2MAWSLookup->webViewBrowser->setHtml(mawsCacheData, QUrl(mawsUrl));
#endif
                foundInDiskCache = TRUE;
              }
            }
            mawsCacheFile.close();
          }
        }

        if ( !foundInDiskCache ) {
          // now check in memory cache and fetch data if unavailable
          if ( !qmc2MAWSCache.contains(gameName) ) {
            qmc2MAWSLookup->webViewBrowser->setHtml("<html><head></head><body><center><p><b>" +
                                    tr("Fetching MAWS page for '%1', please wait...").arg(qmc2GamelistDescriptionMap[gameName]) +
                                    "</b></p><p>" + QString("(<a href=\"%1\">%1</a>)").arg(mawsUrl) + "</p></center></body></html>", QUrl(mawsUrl));
            qmc2MAWSLookup->webViewBrowser->load(QUrl(mawsUrl));
          } else {
            qmc2MAWSLookup->webViewBrowser->setHtml(QString(qUncompress(*qmc2MAWSCache[gameName])), QUrl(mawsUrl));
            qmc2MAWSLookup->webViewBrowser->stop();
          }
        }
        qmc2LastMAWSItem = qmc2CurrentItem;
        connect(qmc2MAWSLookup->webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(mawsLoadFinished(bool)));
        tabMAWS->setUpdatesEnabled(TRUE);
      }
      break;
#endif

    case QMC2_CONFIG_INDEX:
      if ( qmc2CurrentItem != qmc2LastConfigItem ) {
        QWidget *configWidget = qmc2DetailSetup->tabWidgetsMap[QMC2_CONFIG_INDEX];
        configWidget->setUpdatesEnabled(FALSE);
        if ( qmc2EmulatorOptions ) {
          qmc2EmulatorOptions->save();
          QLayout *vbl = configWidget->layout();
          if ( vbl )
            delete vbl;
          delete qmc2EmulatorOptions;
          delete pushButtonCurrentEmulatorOptionsExportToFile;
          delete pushButtonCurrentEmulatorOptionsImportFromFile;
          qmc2EmulatorOptions = NULL;
        }
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        QVBoxLayout *layout = new QVBoxLayout;
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
        qmc2EmulatorOptions = new EmulatorOptions("MAME/Configuration/" + gameName, configWidget);
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
        qmc2EmulatorOptions = new EmulatorOptions("MESS/Configuration/" + gameName, configWidget);
#endif
        qmc2EmulatorOptions->load();
        layout->addWidget(qmc2EmulatorOptions);
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        pushButtonCurrentEmulatorOptionsExportToFile = new QPushButton(tr("Export to..."), this);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
        pushButtonCurrentEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export game-specific MAME configuration"));
        pushButtonCurrentEmulatorOptionsExportToFile->setStatusTip(QObject::tr("Export game-specific MAME configuration"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
        pushButtonCurrentEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export machine-specific MESS configuration"));
        pushButtonCurrentEmulatorOptionsExportToFile->setStatusTip(QObject::tr("Export machine-specific MESS configuration"));
#endif
        pushButtonCurrentEmulatorOptionsImportFromFile = new QPushButton(QObject::tr("Import from..."), this);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
        pushButtonCurrentEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import game-specific MAME configuration"));
        pushButtonCurrentEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import game-specific MAME configuration"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
        pushButtonCurrentEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import machine-specific MESS configuration"));
        pushButtonCurrentEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import machine-specific MESS configuration"));
#endif
        buttonLayout->addWidget(pushButtonCurrentEmulatorOptionsExportToFile);
        buttonLayout->addWidget(pushButtonCurrentEmulatorOptionsImportFromFile);
        layout->addLayout(buttonLayout);
        configWidget->setLayout(layout);
        qmc2EmulatorOptions->show();
        pushButtonCurrentEmulatorOptionsExportToFile->show();
        pushButtonCurrentEmulatorOptionsImportFromFile->show();
        // export/import menus
        qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile = new QMenu(qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile);
        connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), tr("<inipath>/%1.ini").arg(gameName)), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonCurrentEmulatorOptionsExportToFile_clicked()));
        connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonCurrentEmulatorOptionsSelectExportFile_clicked()));
        qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile->setMenu(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile);
        qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile = new QMenu(qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile);
        connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), tr("<inipath>/%1.ini").arg(gameName)), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked()));
        connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonCurrentEmulatorOptionsSelectImportFile_clicked()));
        qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile->setMenu(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile);
#endif
        qmc2EmulatorOptions->resizeColumnToContents(0);
        qmc2EmulatorOptions->pseudoConstructor();
        qmc2LastConfigItem = qmc2CurrentItem;
        configWidget->setUpdatesEnabled(TRUE);
      }
      break;

    case QMC2_GAMEINFO_INDEX:
      if ( qmc2CurrentItem != qmc2LastGameInfoItem ) {
        tabGameInfo->setUpdatesEnabled(FALSE);
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        if ( qmc2GameInfoDB.contains(gameName) || qmc2GameInfoDB.contains(qmc2ParentMap[gameName]) ) {
          // update game/machine info if it points to a different DB record
          bool updateInfo = TRUE;
          QByteArray *newGameInfo = qmc2GameInfoDB[gameName];
          if ( !newGameInfo ) {
            // fall back to parent's game/machine info, if applicable
            newGameInfo = qmc2GameInfoDB[qmc2ParentMap[gameName]];
          }
          if ( qmc2LastGameInfoItem )
            if ( qmc2LastGameInfoItem->child(0) ) {
              QByteArray *oldGameInfo = qmc2GameInfoDB[qmc2LastGameInfoItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)];
              updateInfo = (newGameInfo != oldGameInfo || !oldGameInfo);
            }
          if ( updateInfo ) {
            if ( newGameInfo ) {
              if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool() )
                textBrowserGameInfo->setHtml(QString(qUncompress(*newGameInfo)));
              else
                textBrowserGameInfo->setHtml(QString(*newGameInfo));
            } else
              textBrowserGameInfo->setHtml("<b>" + qmc2GamelistDescriptionMap[gameName] + "</b><p>" + tr("No data available"));
          }
        } else
          textBrowserGameInfo->setHtml("<b>" + qmc2GamelistDescriptionMap[gameName] + "</b><p>" + tr("No data available"));
        qmc2LastGameInfoItem = qmc2CurrentItem;
        tabGameInfo->setUpdatesEnabled(TRUE);
      }
      break;

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    case QMC2_EMUINFO_INDEX:
      if ( qmc2CurrentItem != qmc2LastEmuInfoItem ) {
        tabEmuInfo->setUpdatesEnabled(FALSE);
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        if ( qmc2EmuInfoDB.contains(gameName) || qmc2EmuInfoDB.contains(qmc2ParentMap[gameName]) ) {
          // update emulator info if it points to a different DB record
          bool updateInfo = TRUE;
          QByteArray *newEmuInfo = qmc2EmuInfoDB[gameName];
          if ( !newEmuInfo ) {
            // fall back to parent's emulator info, if applicable
            newEmuInfo = qmc2EmuInfoDB[qmc2ParentMap[gameName]];
          }
          if ( qmc2LastEmuInfoItem )
            if ( qmc2LastEmuInfoItem->child(0) ) {
              QByteArray *oldEmuInfo = qmc2EmuInfoDB[qmc2LastEmuInfoItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)];
              updateInfo = (newEmuInfo != oldEmuInfo || !oldEmuInfo);
            }
          if ( updateInfo ) {
            if ( newEmuInfo ) {
              if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB").toBool() )
                textBrowserEmuInfo->setHtml(QString(qUncompress(*newEmuInfo)));
              else
                textBrowserEmuInfo->setHtml(QString(*newEmuInfo));
            } else
              textBrowserEmuInfo->setHtml(tr("No data available"));
          }
        } else
          textBrowserEmuInfo->setHtml(tr("No data available"));
        qmc2LastEmuInfoItem = qmc2CurrentItem;
        tabEmuInfo->setUpdatesEnabled(TRUE);
      }
      break;
#endif

    default:
      // if local emulator options exits and they are no longer needed, close & destroy them...
      if ( qmc2EmulatorOptions ) {
        QWidget *configWidget = qmc2DetailSetup->tabWidgetsMap[QMC2_CONFIG_INDEX];
        qmc2EmulatorOptions->save();
        QLayout *vbl = configWidget->layout();
        if ( vbl )
          delete vbl;
        delete qmc2EmulatorOptions;
        delete pushButtonCurrentEmulatorOptionsExportToFile;
        delete pushButtonCurrentEmulatorOptionsImportFromFile;
        qmc2EmulatorOptions = NULL;
      }
      qmc2LastConfigItem = NULL;
      break;
  }
}

void MainWindow::on_treeWidgetGamelist_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetGamelist_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

  if ( !qmc2IgnoreItemActivation )
    on_actionPlay_activated();
  qmc2IgnoreItemActivation = FALSE;
}

void MainWindow::on_treeWidgetHierarchy_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetHierarchy_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

  if ( !qmc2IgnoreItemActivation )
    on_actionPlay_activated();
  qmc2IgnoreItemActivation = FALSE;
}

void MainWindow::on_treeWidgetGamelist_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetGamelist_itemDoubleClicked(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

  if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() )
    qmc2IgnoreItemActivation = TRUE;
}

void MainWindow::on_treeWidgetHierarchy_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetHierarchy_itemDoubleClicked(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

  if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() )
    qmc2IgnoreItemActivation = TRUE;
}

void MainWindow::on_treeWidgetGamelist_itemExpanded(QTreeWidgetItem *item)
{
  if ( qmc2ReloadActive ) {
    treeWidgetGamelist->collapseItem(item);
    log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return;
  }

#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetGamelist_itemExpanded(QTreeWidgetItem *item = 0x"+ QString::number((ulong)item, 16) + ")");
#endif

  if ( item->child(0) ) {
    if ( item->child(0)->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
      qmc2Gamelist->parseGameDetail(item);
  }
}

void MainWindow::on_treeWidgetGamelist_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetGamelist_currentItemChanged(QTreeWidgetItem *current = 0x" + QString::number((ulong)current, 16) + ", QTreeWidgetItem *previous = 0x" + QString::number((ulong)previous, 16) + ")");
#endif

  // workaround for a Qt bug: when POS1/Home is pressed, QTreeWidget & QTreeView don't correctly select the first VISIBLE item,
  // if the top item is HIDDEN
  if ( current ) {
    if ( qmc2CheckItemVisibility ) {
      if ( current->isHidden() ) {
        int i = treeWidgetGamelist->indexOfTopLevelItem(current);
        if ( i >= 0 ) {
          while ( current->isHidden() && i < treeWidgetGamelist->topLevelItemCount() ) {
            current = treeWidgetGamelist->topLevelItem(++i);
            if ( current == NULL ) 
              break;
          }
          if ( current ) {
            treeWidgetGamelist->setCurrentItem(current);
          }
        }
      }
    } else
      qmc2CurrentItem = current;
  }
  qmc2CheckItemVisibility = TRUE;
  updateTimer.start(qmc2UpdateDelay);
}

void MainWindow::on_treeWidgetHierarchy_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetHierarchy_currentItemChanged(QTreeWidgetItem *current = 0x" + QString::number((ulong)current, 16) + ", QTreeWidgetItem *previous = 0x" + QString::number((ulong)previous, 16) + ")");
#endif

  qmc2CheckItemVisibility = FALSE;
  updateTimer.start(qmc2UpdateDelay);
}

void MainWindow::on_treeWidgetGamelist_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetGamelist_itemSelectionChanged()");
#endif

  updateTimer.start(qmc2UpdateDelay);
}

void MainWindow::on_treeWidgetGamelist_itemSelectionChanged_delayed()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetGamelist_itemSelectionChanged_delayed()");
#endif

  updateTimer.stop();
  QList<QTreeWidgetItem *>selected;
  if ( qmc2HierarchySelectedItem != NULL )
    selected.append(qmc2HierarchySelectedItem);
  else
    selected = treeWidgetGamelist->selectedItems();
  if ( selected.count() == 0 )
    if ( treeWidgetGamelist->currentItem() )
      selected.append(treeWidgetGamelist->currentItem());
  if ( selected.count() > 0 ) {
    QTreeWidgetItem *topLevelItem = selected.at(0);
    while ( topLevelItem->parent() )
      topLevelItem = topLevelItem->parent();
    if ( topLevelItem ) {
      qmc2CurrentItem = topLevelItem;
      qmc2Preview->repaint();
      qmc2Flyer->repaint();
      qmc2Cabinet->repaint();
      qmc2Controller->repaint();
      qmc2Marquee->repaint();
      qmc2Title->repaint();
      on_tabWidgetGameDetail_currentChanged(tabWidgetGameDetail->currentIndex());
    }
  } else
    qmc2CurrentItem = NULL;
  qmc2HierarchySelectedItem = NULL;
}

void MainWindow::on_treeWidgetHierarchy_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetHierarchy_itemSelectionChanged()");
#endif

  qmc2HierarchySelectedItem = NULL;
  QList<QTreeWidgetItem *>selected = treeWidgetHierarchy->selectedItems();
  if ( selected.count() > 0 ) {
    if ( selected.at(0)->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
      return;
    qmc2HierarchySelectedItem = qmc2GamelistItemByDescriptionMap[selected.at(0)->text(QMC2_GAMELIST_COLUMN_GAME)];
    qmc2CheckItemVisibility = FALSE;
    treeWidgetGamelist->setCurrentItem(qmc2HierarchySelectedItem);
  }
}

void MainWindow::on_textBrowserFrontendLog_textChanged()
{
  textBrowserFrontendLog->verticalScrollBar()->setSliderPosition(textBrowserFrontendLog->verticalScrollBar()->maximum());
  textBrowserFrontendLog->horizontalScrollBar()->setSliderPosition(textBrowserFrontendLog->horizontalScrollBar()->minimum());
}

void MainWindow::on_textBrowserEmulatorLog_textChanged()
{
  textBrowserEmulatorLog->verticalScrollBar()->setSliderPosition(textBrowserEmulatorLog->verticalScrollBar()->maximum());
  textBrowserEmulatorLog->horizontalScrollBar()->setSliderPosition(textBrowserEmulatorLog->horizontalScrollBar()->minimum());
}

void MainWindow::on_treeWidgetEmulators_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetEmulators_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QTreeWidgetItem *item = treeWidgetEmulators->itemAt(p);
  if ( item ) {
    treeWidgetEmulators->setItemSelected(item, TRUE);
    qmc2EmulatorMenu->move(treeWidgetEmulators->viewport()->mapToGlobal(p));
    qmc2EmulatorMenu->show();
  }
}

void MainWindow::action_terminateEmulator_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_terminateEmulator_triggered()");
#endif

  QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
  int i;
  for (i = 0; i < sl.count(); i++) {
    QTreeWidgetItem *item = sl[i];
    while ( item->parent() ) item = item->parent();
    qmc2ProcessManager->terminate(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
  }

  if ( sl.count() == 0 )
    if ( treeWidgetEmulators->currentItem() ) {
      QTreeWidgetItem *item = treeWidgetEmulators->currentItem();
      while ( item->parent() ) item = item->parent();
      qmc2ProcessManager->terminate(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
    }
}

void MainWindow::action_killEmulator_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_killEmulator_triggered()");
#endif

  QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
  int i;
  for (i = 0; i < sl.count(); i++) {
    QTreeWidgetItem *item = sl[i];
    while ( item->parent() ) item = item->parent();
    qmc2ProcessManager->kill(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
  }

  if ( sl.count() == 0 )
    if ( treeWidgetEmulators->currentItem() ) {
      QTreeWidgetItem *item = treeWidgetEmulators->currentItem();
      while ( item->parent() ) item = item->parent();
      qmc2ProcessManager->kill(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
    }
}

void MainWindow::action_copyEmulatorCommand_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_copyEmulatorCommand_triggered()");
#endif

  QString commandString = "";
  QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
  QList<QTreeWidgetItem *> tl;
  int i;

  // find toplevel items...
  for (i = 0; i < sl.count(); i++) {
    QTreeWidgetItem *item = sl[i];
    while ( item->parent() ) item = item->parent();
    if ( !tl.contains(item) )
      tl.append(item);
  }
  // ... and copy their commands
  for (i = 0; i < tl.count(); i++) {
    if ( i > 0 ) commandString += "\n";
    commandString += tl[i]->text(QMC2_EMUCONTROL_COLUMN_COMMAND);
  }

  QApplication::clipboard()->setText(commandString);
}

void MainWindow::action_removeFromFavorites_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_removeFromFavorites_triggered()");
#endif

  QListWidgetItem *i = listWidgetFavorites->currentItem();
  if ( i ) {
    QListWidgetItem *item = listWidgetFavorites->takeItem(listWidgetFavorites->row(i));
    delete item;
  }
}

void MainWindow::action_clearAllFavorites_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_clearAllFavorites_triggered()");
#endif

  switch ( QMessageBox::question(this, tr("Confirm"),
                                 tr("Are you sure you want to clear the favorites list?"),
                                 QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) ) {
    case QMessageBox::No:
      return;
      break;

    case QMessageBox::Yes:
      listWidgetFavorites->clear();
      qmc2Gamelist->saveFavorites();
      break;

    default:
      break;
  }
}

void MainWindow::action_saveFavorites_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_saveFavorites_triggered()");
#endif

  qmc2Gamelist->saveFavorites();
}

void MainWindow::action_removeFromPlayed_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_removeFromPlayed_triggered()");
#endif

  QListWidgetItem *i = listWidgetPlayed->currentItem();
  if ( i ) {
    QListWidgetItem *item = listWidgetPlayed->takeItem(listWidgetPlayed->row(i));
    delete item;
  }
}

void MainWindow::action_clearAllPlayed_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_clearAllPlayed_triggered()");
#endif

  switch ( QMessageBox::question(this, tr("Confirm"),
                                 tr("Are you sure you want to clear the play history?"),
                                 QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) ) {
    case QMessageBox::No:
      return;
      break;

    case QMessageBox::Yes:
      listWidgetPlayed->clear();
      qmc2Gamelist->savePlayHistory();
      break;

    default:
      break;
  }
}

void MainWindow::action_savePlayed_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_savePlayed_triggered()");
#endif

  qmc2Gamelist->savePlayHistory();
}

void MainWindow::on_listWidgetSearch_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetSearch_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QListWidgetItem *item = listWidgetSearch->itemAt(p);
  if ( item ) {
    listWidgetSearch->setItemSelected(item, TRUE);
    qmc2SearchMenu->move(listWidgetSearch->viewport()->mapToGlobal(p));
    qmc2SearchMenu->show();
  }
}

void MainWindow::on_listWidgetFavorites_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetFavorites_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QListWidgetItem *item = listWidgetFavorites->itemAt(p);
  if ( item ) {
    listWidgetFavorites->setItemSelected(item, TRUE);
    qmc2FavoritesMenu->move(listWidgetFavorites->viewport()->mapToGlobal(p));
    qmc2FavoritesMenu->show();
  }
}

void MainWindow::on_listWidgetPlayed_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetPlayed_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QListWidgetItem *item = listWidgetPlayed->itemAt(p);
  if ( item ) {
    listWidgetPlayed->setItemSelected(item, TRUE);
    qmc2PlayedMenu->move(listWidgetPlayed->viewport()->mapToGlobal(p));
    qmc2PlayedMenu->show();
  }
}

void MainWindow::on_treeWidgetGamelist_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetGamelist_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QTreeWidgetItem *item = treeWidgetGamelist->itemAt(p);
  if ( !item )
    return;
  if ( item->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
    return;
  if ( item ) {
    treeWidgetGamelist->setItemSelected(item, TRUE);
    qmc2GameMenu->move(treeWidgetGamelist->viewport()->mapToGlobal(p));
    qmc2GameMenu->show();
  }
}

void MainWindow::on_treeWidgetHierarchy_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetHierarchy_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QTreeWidgetItem *item = treeWidgetHierarchy->itemAt(p);
  if ( !item )
    return;
  if ( item->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
    return;
  if ( item ) {
    treeWidgetHierarchy->setItemSelected(item, TRUE);
    qmc2GameMenu->move(treeWidgetHierarchy->viewport()->mapToGlobal(p));
    qmc2GameMenu->show();
  }
}

void MainWindow::on_stackedWidgetView_currentChanged(int index)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_tackedWidgetView_currentChanged(int index = %1)").arg(index));
#endif

  if ( !qmc2CurrentItem )
    return;

  QTreeWidgetItem *ci = treeWidgetGamelist->currentItem();
  if ( !ci )
    return;

  if ( qmc2CurrentItem != ci && qmc2ReloadActive )
    qmc2CurrentItem = ci;

  if ( qmc2CurrentItem->childCount() <= 0 )
    return;

  switch ( index ) {
    case QMC2_VIEWHIERARCHY_INDEX:
      if ( !qmc2ReloadActive ) {
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
        treeWidgetHierarchy->clearSelection();
        if ( hierarchyItem ) {
          treeWidgetHierarchy->setCurrentItem(hierarchyItem);
          treeWidgetHierarchy->scrollToItem(hierarchyItem, QAbstractItemView::PositionAtTop);
          hierarchyItem->setSelected(TRUE);
        }
      }
      break;

    case QMC2_VIEWGAMELIST_INDEX:
    default:
      scrollToCurrentItem();
      break;
  }
}

void MainWindow::on_pushButtonGlobalEmulatorOptionsSelectExportFile_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonGlobalEmulatorOptionsSelectExportFile_clicked()");
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  QString fileName = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mame.ini";
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  QString fileName = qmc2Config->value("MESS/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mess.ini";
#else
  QString fileName;
#endif
  QString s = QFileDialog::getSaveFileName(qmc2Options, tr("Choose export file"), fileName, tr("All files (*)"));
  if ( !s.isNull() )
    on_pushButtonGlobalEmulatorOptionsExportToFile_clicked(s);
}

void MainWindow::on_pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName = %1)").arg(useFileName));
#endif

  qmc2GlobalEmulatorOptions->exportToIni(TRUE, useFileName);
}

void MainWindow::on_pushButtonGlobalEmulatorOptionsSelectImportFile_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonGlobalEmulatorOptionsSelectImportFile_clicked()");
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  QString fileName = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mame.ini";
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  QString fileName = qmc2Config->value("MESS/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mess.ini";
#else
  QString fileName;
#endif
  QString s = QFileDialog::getOpenFileName(qmc2Options, tr("Choose import file"), fileName, tr("All files (*)"));
  if ( !s.isNull() )
    on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked(s);
}

void MainWindow::on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName = %1)").arg(useFileName));
#endif

  qmc2GlobalEmulatorOptions->importFromIni(TRUE, useFileName);
}

void MainWindow::on_pushButtonCurrentEmulatorOptionsSelectExportFile_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonCurrentEmulatorOptionsSelectExportFile_clicked()");
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  QStringList iniPaths = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().split(";");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  QStringList iniPaths = qmc2Config->value("MESS/Configuration/Global/inipath", QDir::homePath()).toString().split(";");
#endif
  QString iniPath;
  if ( iniPaths.count() > 0 ) {
    iniPath = iniPaths[0].replace("~", QDir::homePath());
  } else {
    iniPath = ".";
    log(QMC2_LOG_FRONTEND, tr("WARNING: invalid inipath"));
  }
  QString s = QFileDialog::getSaveFileName(this, tr("Choose export file"), iniPath, tr("All files (*)"));
  if ( !s.isNull() )
    on_pushButtonCurrentEmulatorOptionsExportToFile_clicked(s);
}

void MainWindow::on_pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName = %1)").arg(useFileName));
#endif

  qmc2EmulatorOptions->exportToIni(FALSE, useFileName);
}

void MainWindow::on_pushButtonCurrentEmulatorOptionsSelectImportFile_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonCurrentEmulatorOptionsSelectImportFile_clicked()");
#endif

  if ( !qmc2CurrentItem )
    return;

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  QStringList iniPaths = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().split(";");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  QStringList iniPaths = qmc2Config->value("MESS/Configuration/Global/inipath", QDir::homePath()).toString().split(";");
#endif
  QString iniPath;
  if ( iniPaths.count() > 0 ) {
    iniPath = iniPaths[0].replace("~", QDir::homePath());
  } else {
    iniPath = ".";
    log(QMC2_LOG_FRONTEND, tr("WARNING: invalid inipath"));
  }
  iniPath += "/" + qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON) + ".ini";
  QString s = QFileDialog::getOpenFileName(this, tr("Choose import file"), iniPath, tr("All files (*)"));
  if ( !s.isNull() )
    on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked(s);
}

void MainWindow::on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName = %1)").arg(useFileName));
#endif

  qmc2EmulatorOptions->importFromIni(FALSE, useFileName);
}

#if QMC2_JOYSTICK == 1
void MainWindow::mapJoystickFunction(QString function)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::mapJoystickFunction(QString function = %1)").arg(function));
#endif

  if ( qmc2CleaningUp )
    return;

  // don't map joystick functions while calibration or test are active
  if ( qmc2Options->treeWidgetJoystickMappings->isHidden() )
    return;

  QString shortcut = qmc2CustomShortcutMap[qmc2JoystickFunctionMap[function]];

#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: matched function = %1").arg(shortcut.isEmpty() ? "none" : shortcut));
#endif

  if ( shortcut.isEmpty() )
    return;
  
  QWidget *focusWindow = QApplication::focusWidget();

  if ( focusWindow ) {
    QKeySequence keySeq(shortcut);
    uint i, key = 0;
    for (i = 0; i < keySeq.count(); i++)
      key += keySeq[i];
    QKeyEvent *emulatedKeyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier);
    qApp->postEvent(focusWindow, emulatedKeyEvent);
  }
}

void MainWindow::on_joystickAxisValueChanged(int axis, int value)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_joystickAxisValueChanged(int axis = %1, int value = %2)").arg(axis).arg(value));
#endif

  if ( qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(axis), TRUE).toBool() ) {
    if ( value != 0 )
      mapJoystickFunction(QString("A%1%2").arg(axis).arg(value < 0 ? "-" : "+"));
  }
}

void MainWindow::on_joystickButtonValueChanged(int button, bool value)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_joystickButtonValueChanged(int button = %1, bool value = %2)").arg(button).arg(value));
#endif

   if ( value )
     mapJoystickFunction(QString("B%1").arg(button));
}

void MainWindow::on_joystickHatValueChanged(int hat, int value)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_joystickHatValueChanged(int hat = %1, int value = %2)").arg(hat).arg(value));
#endif

   if ( value != 0 )
     mapJoystickFunction(QString("H%1:%2").arg(hat).arg(value));
}

void MainWindow::on_joystickTrackballValueChanged(int trackball, int deltaX, int deltaY)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_joystickTrackballValueChanged(int trackball = %1, int deltaX = %2, int deltaY = %3)").arg(trackball).arg(deltaX).arg(deltaY));
#endif

  mapJoystickFunction(QString("T%1:X%2,Y%3").arg(trackball)
                             .arg(deltaX < 0 ? "-" : deltaX > 0 ? "+" : "=")
                             .arg(deltaY < 0 ? "-" : deltaY > 0 ? "+" : "="));
}
#endif

void MainWindow::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::closeEvent(QCloseEvent *e = 0x" + QString::number((qulonglong)e, 16) + ")");
#endif

  if ( qmc2CleaningUp ) {
    e->ignore();
    return;
  }

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  if ( qmc2ReloadActive ||
       qmc2VerifyActive ||
       qmc2FilterActive ||
       qmc2ImageCheckActive ||
       qmc2SampleCheckActive ||
       qmc2ROMAlyzerActive ||
       qmc2LoadingGameInfoDB ||
       qmc2LoadingEmuInfoDB )
#else
  if ( qmc2ReloadActive ||
       qmc2VerifyActive ||
       qmc2FilterActive ||
       qmc2ImageCheckActive ||
       qmc2ROMAlyzerActive ||
       qmc2LoadingGameInfoDB )
#endif
  {
    qmc2StopParser = TRUE;
    log(QMC2_LOG_FRONTEND, tr("stopping current processing upon user request"));
    e->ignore();
    return;
  }

  if ( !qmc2Options->applied ) {
    switch ( QMessageBox::question(this, tr("Confirm"), tr("Your configuration changes have not been applied yet.\nReally quit?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) ) {
      case QMessageBox::No:
        e->ignore();
        return;
        break;

      default:
        break;
    }
  }
 
  bool doKillEmulators = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/KillEmulatorsOnExit").toBool();
  if ( qmc2ProcessManager->procMap.count() > 0 && !doKillEmulators ) {
    switch ( QMessageBox::question(this, tr("Confirm"), tr("There are one or more emulators still running.\nShould they be killed on exit?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel) ) {
      case QMessageBox::Yes:
        doKillEmulators = TRUE;
        break;

      case QMessageBox::No:
        doKillEmulators = FALSE;
        break;

      case QMessageBox::Cancel:
        e->ignore();
        return;
        break;

      default:
        break;
    }
  }

  qmc2CleaningUp = TRUE;
  log(QMC2_LOG_FRONTEND, tr("cleaning up"));

  if ( listWidgetFavorites->count() > 0 )
    qmc2Gamelist->saveFavorites();

  if ( listWidgetPlayed->count() > 0 )
    qmc2Gamelist->savePlayHistory();

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "GUI/GamelistView", comboBoxViewSelect->currentIndex());
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection").toBool() ) {
    if ( qmc2CurrentItem ) {
      log(QMC2_LOG_FRONTEND, tr("saving game selection"));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
      qmc2Config->setValue("MAME/SelectedGame", qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
      qmc2Config->setValue("MESS/SelectedGame", qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME));
#endif
    } else
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
      qmc2Config->remove("MAME/SelectedGame");
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
      qmc2Config->remove("MESS/SelectedGame");
#endif
  }

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() ) {
    log(QMC2_LOG_FRONTEND, tr("saving main widget layout"));
    if ( windowState() & Qt::WindowFullScreen ) {
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", TRUE);
    } else {
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", FALSE);
      if ( isMaximized() ) {
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", TRUE);
      } else {
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", FALSE);
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry", saveGeometry());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position", pos());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size", size());
      }
    }
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter", QSize(hSplitter->sizes().at(0), hSplitter->sizes().at(1)));
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitter", QSize(vSplitter->sizes().at(0), vSplitter->sizes().at(1)));
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistTab", tabWidgetGamelist->currentIndex());
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GameDetailTab", tabWidgetGameDetail->currentIndex());
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineDetailTab", tabWidgetGameDetail->currentIndex());
#endif
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/LogsAndEmulatorsTab", tabWidgetLogsAndEmulators->currentIndex());
    // save toolbar state
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ToolbarState", saveState());
    int i;
    QVariantList gamelistColumnWidths;
    for (i = 0; i < treeWidgetGamelist->header()->count(); i++)
      gamelistColumnWidths << treeWidgetGamelist->header()->sectionSize(i);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistColumnWidths", gamelistColumnWidths);
    QVariantList hierarchyColumnWidths;
    for (i = 0; i < treeWidgetHierarchy->header()->count(); i++)
      hierarchyColumnWidths << treeWidgetHierarchy->header()->sectionSize(i);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyColumnWidths", hierarchyColumnWidths);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistHeaderState", treeWidgetGamelist->header()->saveState());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyHeaderState", treeWidgetHierarchy->header()->saveState());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/EmulatorControlHeaderState", treeWidgetEmulators->header()->saveState());
  }

#if QMC2_USE_PHONON_API
  QList<QListWidgetItem *> pl = listWidgetAudioPlaylist->findItems("*", Qt::MatchWildcard);
  QStringList psl;
  foreach (QListWidgetItem *item, pl) psl << item->text();
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayList", psl);
  if ( listWidgetAudioPlaylist->currentItem() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/LastTrack", listWidgetAudioPlaylist->currentItem()->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayOnStart", checkBoxAudioPlayOnStart->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Shuffle", checkBoxAudioShuffle->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Pause", checkBoxAudioPause->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Fade", checkBoxAudioFade->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Volume", sliderAudioVolume->value());
#endif

  if ( qmc2ArcadeView ) {
    log(QMC2_LOG_FRONTEND, tr("destroying arcade view"));
    qmc2ArcadeView->close();
    delete qmc2ArcadeView;
  }
  if ( qmc2ArcadeSetupDialog ) {
    log(QMC2_LOG_FRONTEND, tr("destroying arcade setup dialog"));
    qmc2ArcadeSetupDialog->close();
    delete qmc2ArcadeSetupDialog;
  }

#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  if ( qmc2MESSDeviceConfigurator ) {
    log(QMC2_LOG_FRONTEND, tr("saving current machine's device configurations"));
    qmc2MESSDeviceConfigurator->save();
  }
#endif

  if ( qmc2EmulatorOptions ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    log(QMC2_LOG_FRONTEND, tr("destroying current game's emulator configuration"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    log(QMC2_LOG_FRONTEND, tr("destroying current machine's emulator configuration"));
#endif
    qmc2EmulatorOptions->save();
    delete qmc2EmulatorOptions;
  }
  log(QMC2_LOG_FRONTEND, tr("destroying global emulator options"));
  //delete qmc2GlobalEmulatorOptions; <- doing so will end up in heavy CPU load and a close-timeout at exit for Qt 4.3+
  //                              <- qmc2GlobalEmulatorOptions->setParent(0) fixes this (this is strange but true :)
  qmc2GlobalEmulatorOptions->pseudoDestructor();
  qmc2GlobalEmulatorOptions->setParent(0);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  log(QMC2_LOG_FRONTEND, tr("destroying game list"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  log(QMC2_LOG_FRONTEND, tr("destroying machine list"));
#endif
  delete qmc2Gamelist;
  if ( qmc2Preview ) {
    log(QMC2_LOG_FRONTEND, tr("destroying preview"));
    delete qmc2Preview;
  }
  if ( qmc2Flyer ) {
    log(QMC2_LOG_FRONTEND, tr("destroying flyer"));
    delete qmc2Flyer;
  }
  if ( qmc2Cabinet ) {
    log(QMC2_LOG_FRONTEND, tr("destroying cabinet"));
    delete qmc2Cabinet;
  }
  if ( qmc2Controller ) {
    log(QMC2_LOG_FRONTEND, tr("destroying controller"));
    delete qmc2Controller;
  }
  if ( qmc2Marquee ) {
    log(QMC2_LOG_FRONTEND, tr("destroying marquee"));
    delete qmc2Marquee;
  }
  if ( qmc2Title ) {
    log(QMC2_LOG_FRONTEND, tr("destroying title"));
    delete qmc2Title;
  }
  if ( qmc2About ) {
    log(QMC2_LOG_FRONTEND, tr("destroying about dialog"));
    delete qmc2About;
  }
  if ( qmc2DocBrowser ) {
    log(QMC2_LOG_FRONTEND, tr("destroying documentation browser"));
    delete qmc2DocBrowser;
  }
  if ( qmc2ImageChecker ) {
    log(QMC2_LOG_FRONTEND, tr("destroying image checker"));
    qmc2ImageChecker->close();
    delete qmc2ImageChecker;
  }
  if ( qmc2SampleChecker ) {
    log(QMC2_LOG_FRONTEND, tr("destroying sample checker"));
    qmc2SampleChecker->close();
    delete qmc2SampleChecker;
  }
  if ( qmc2ROMAlyzer ) {
    log(QMC2_LOG_FRONTEND, tr("destroying ROMAlyzer"));
    qmc2ROMAlyzer->saveState();
    delete qmc2ROMAlyzer;
  }
  if ( qmc2ROMStatusExporter ) {
    log(QMC2_LOG_FRONTEND, tr("destroying ROM status exporter"));
    qmc2ROMStatusExporter->close();
    delete qmc2ROMStatusExporter;
  }
  if ( qmc2DetailSetup ) {
    log(QMC2_LOG_FRONTEND, tr("destroying detail setup"));
    qmc2DetailSetup->close();
    delete qmc2DetailSetup;
  }
  if ( !qmc2GameInfoDB.isEmpty() ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    log(QMC2_LOG_FRONTEND, tr("destroying game info DB"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    log(QMC2_LOG_FRONTEND, tr("destroying machine info DB"));
#endif
    QMapIterator<QString, QByteArray *> it(qmc2GameInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2GameInfoDB.clear();
  }
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  if ( !qmc2EmuInfoDB.isEmpty() ) {
    log(QMC2_LOG_FRONTEND, tr("destroying emulator info DB"));
    QMapIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2EmuInfoDB.clear();
  }
#endif

  log(QMC2_LOG_FRONTEND, tr("destroying process manager"));
  if ( qmc2ProcessManager->procMap.count() > 0 ) {
    if ( doKillEmulators ) {
      log(QMC2_LOG_FRONTEND, tr("killing %n running emulator(s) on exit", "", qmc2ProcessManager->procMap.count()));
      delete qmc2ProcessManager;
    } else
      log(QMC2_LOG_FRONTEND, tr("keeping %n running emulator(s) alive", "", qmc2ProcessManager->procMap.count()));
  } else 
    delete qmc2ProcessManager;

#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
  if ( qmc2FifoFile ) {
    if ( qmc2FifoNotifier )
      delete qmc2FifoNotifier;
    if ( qmc2FifoFile->isOpen() )
      qmc2FifoFile->close();
    delete qmc2FifoFile;
    ::unlink(QMC2_SDLMAME_OUTPUT_FIFO);
  }
#endif

  log(QMC2_LOG_FRONTEND, tr("so long and thanks for all the fish"));

  qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "InstanceRunning"), FALSE);

  delete qmc2KeyPressFilter;
  delete qmc2Options;
  e->accept();
}

void MainWindow::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::showEvent(QShowEvent *e = 0x" + QString::number((qulonglong)e, 16) + ")");
#endif

  e->accept();
}

void MainWindow::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::hideEvent(QHideEvent *e = 0x" + QString::number((qulonglong)e, 16) + ")");
#endif

  e->accept();
}

void MainWindow::init()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::init()");
#endif

  createFifo();
  qApp->processEvents();
  qmc2GhostImagePixmap.load(":/data/img/ghost.png");
  // setup application style
  QString myStyle = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Style", tr("Default")).toString();
  setupStyle(myStyle);
  qmc2EarlyStartup = FALSE;
  on_actionReload_activated();
}

void MainWindow::setupStyle(QString styleName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::setupStyle(QString &styleName = %1").arg(styleName));
#endif

  static QPalette customPalette = QApplication::palette();

  QStyle *newStyle;
  if ( styleName != tr("Default") ) {
    if ( QStyleFactory::keys().contains(styleName) )
      newStyle = QStyleFactory::create(styleName);
    else
      newStyle = QStyleFactory::create(qmc2DefaultStyle);
  } else
    newStyle = QStyleFactory::create(qmc2DefaultStyle);

  QApplication::setStyle(newStyle);

  // work around for an annoying Qt bug...
  if ( !qmc2EarlyStartup ) {
    menuBar()->setStyle(newStyle);
    toolbar->setStyle(newStyle);
  }

  qApp->processEvents();

  QPalette newPalette;

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette").toBool() )
    newPalette = QApplication::style()->standardPalette();
  else
    newPalette = customPalette;

  QApplication::setPalette(newPalette);

  // work around for the same annoying Qt bug...
  if ( !qmc2EarlyStartup ) {
    menuBar()->setPalette(newPalette);
    toolbar->setPalette(newPalette);
  }

  qApp->processEvents();
}

void MainWindow::viewFullDetail()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::viewFullDetail()");
#endif

  comboBoxViewSelect->setCurrentIndex(QMC2_VIEWGAMELIST_INDEX);
  qApp->processEvents();
  tabWidgetGamelist->setCurrentIndex(QMC2_GAMELIST_INDEX);
  tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/view_detail.png")));
  treeWidgetGamelist->setFocus();
}

void MainWindow::viewParentClones()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::viewParentClones()");
#endif

  comboBoxViewSelect->setCurrentIndex(QMC2_VIEWHIERARCHY_INDEX);
  qApp->processEvents();
  tabWidgetGamelist->setCurrentIndex(QMC2_GAMELIST_INDEX);
  tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/view_tree.png")));
  treeWidgetHierarchy->setFocus();
}

bool KeyPressFilter::eventFilter(QObject *object, QEvent *event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

#ifdef QMC2_DEBUG
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: KeyPressFilter::eventFilter(QObject *object = %1, QEvent *event = %2)").arg((qulonglong)object).arg((qulonglong)event));
#endif
    
    if ( keyEvent->text() == QString("QMC2_EMULATED_KEY") ) {
#ifdef QMC2_DEBUG
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: emulated key event"));
#endif
      return FALSE;
    }

    int myKeySeq = 0;
    if ( keyEvent->modifiers() & Qt::ShiftModifier )
      myKeySeq += Qt::SHIFT;
    if ( keyEvent->modifiers() & Qt::ControlModifier )
      myKeySeq += Qt::CTRL;
    if ( keyEvent->modifiers() & Qt::AltModifier )
      myKeySeq += Qt::ALT;
    if ( keyEvent->modifiers() & Qt::MetaModifier )
      myKeySeq += Qt::META;
    myKeySeq += keyEvent->key();

    QString pressedKeySeq = QKeySequence(myKeySeq).toString();
#ifdef QMC2_DEBUG
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: current key-sequence = '%1'").arg(pressedKeySeq));
#endif

    QString matchedKeySeq = qmc2CustomShortcutMap.key(pressedKeySeq);
    if ( !matchedKeySeq.isEmpty() ) {
      if ( !qmc2MainWindow->menuBar()->isVisible() ) {
        QPair<QString, QAction *> actionPair = qmc2ShortcutMap[qmc2CustomShortcutMap.key(pressedKeySeq)];
        if ( actionPair.second )
        {
#ifdef QMC2_DEBUG
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, emulating key event (due to no menu bar)").arg(pressedKeySeq));
#endif
          actionPair.second->trigger();
          return TRUE;
        }
      }

      if ( matchedKeySeq != pressedKeySeq ) {
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: emulating key event for '%1'").arg(matchedKeySeq));
#endif
        // emulate a key event for the mapped key
        QKeySequence emulatedKeySequence(matchedKeySeq);
        QKeyEvent *emulatedKeyEvent = new QKeyEvent(QEvent::KeyPress, emulatedKeySequence[0], Qt::NoModifier, QString("QMC2_EMULATED_KEY"));
        qApp->postEvent(object, emulatedKeyEvent);
        return TRUE;
      } else {
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
        // default key event processing
        return FALSE;
      }
    }

    QMap<QString, QString>::const_iterator it = qmc2CustomShortcutMap.find(pressedKeySeq);
    if ( it != qmc2CustomShortcutMap.end() ) {
      if ( !it.value().isEmpty() ) {
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, key event suppressed").arg(pressedKeySeq));
#endif
        return TRUE;
      } else {
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
        return FALSE;
      }
    }

#ifdef QMC2_DEBUG
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
    return FALSE;
  } else {
    // default event processing
    return QObject::eventFilter(object, event);
  }
}

void MainWindow::loadGameInfoDB()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::loadGameInfoDB()");
#endif

  QTime gameInfoElapsedTime,
        gameInfoTimer;

  qmc2LoadingGameInfoDB = TRUE;
  qmc2StopParser = FALSE;
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  log(QMC2_LOG_FRONTEND, tr("loading game info DB"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  log(QMC2_LOG_FRONTEND, tr("loading machine info DB"));
#endif

  gameInfoTimer.start();

  // clear game/machine info DB
  QMapIterator<QString, QByteArray *> it(qmc2GameInfoDB);
  QList<QByteArray *> deletedRecords;
  while ( it.hasNext() ) {
    it.next();
    if ( !deletedRecords.contains(it.value()) ) {
      if ( it.value() )
        delete it.value();
      deletedRecords.append(it.value());
    }
  }
  deletedRecords.clear();
  qmc2GameInfoDB.clear();

  bool compressData = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool();
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  QString pathToGameInfoDB = qmc2Config->value("MAME/FilesAndDirectories/GameInfoDB").toString();
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  QString pathToGameInfoDB = qmc2Config->value("MESS/FilesAndDirectories/GameInfoDB").toString();
#endif
  QFile gameInfoDB(pathToGameInfoDB);
  gameInfoDB.open(QIODevice::ReadOnly | QIODevice::Text);
  if ( gameInfoDB.isOpen() ) {
    qmc2MainWindow->progressBarGamelist->reset();
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
      progressBarGamelist->setFormat(tr("Game Info - %p%"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
      progressBarGamelist->setFormat(tr("Machine Info - %p%"));
#endif
    else
      progressBarGamelist->setFormat("%p%");
    progressBarGamelist->setRange(0, gameInfoDB.size());
    qApp->processEvents();
    QTextStream ts(&gameInfoDB);
    int recordsProcessed = 0;
    while ( !ts.atEnd() && !qmc2StopParser ) {
      QString singleLine = ts.readLine();
      while ( !singleLine.simplified().startsWith("$info=") && !ts.atEnd() ) {
        singleLine = ts.readLine();
        progressBarGamelist->setValue(gameInfoDB.pos());
        if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 )
          qApp->processEvents();
      }
      if ( singleLine.simplified().startsWith("$info=") ) {
        QStringList gameWords = singleLine.simplified().mid(6).split(",");
        while ( !singleLine.simplified().startsWith("$bio") && !ts.atEnd() ) {
          singleLine = ts.readLine();
          progressBarGamelist->setValue(gameInfoDB.pos());
          if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 )
            qApp->processEvents();
        }
        if ( singleLine.simplified().startsWith("$bio") ) {
          QString gameInfoString;
          bool firstLine = TRUE;
          while ( !singleLine.simplified().startsWith("$end") && !ts.atEnd() ) {
            singleLine = ts.readLine();
            if ( !singleLine.simplified().startsWith("$end") ) {
              if ( !firstLine ) {
                gameInfoString.append(singleLine + "<br>");
              } else if ( firstLine && !singleLine.isEmpty() ) {
                gameInfoString.append("<b>" + singleLine + "</b><br>");
                firstLine = FALSE;
              }
            }
            progressBarGamelist->setValue(gameInfoDB.pos());
            if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 )
              qApp->processEvents();
          }
          if ( singleLine.simplified().startsWith("$end") ) {
            // convert "two (or more) empty lines" to a paragraph delimiter
            gameInfoString = gameInfoString.replace("<br><br><br>", "<p>").replace("<br><br>", "<p>");
            if ( gameInfoString.endsWith("<p>") )
              gameInfoString.remove(gameInfoString.length() - 3, gameInfoString.length() - 1);
            QByteArray *gameInfo;
            if ( compressData )
              gameInfo = new QByteArray(qCompress(gameInfoString.toAscii())); 
            else
              gameInfo = new QByteArray(gameInfoString.toAscii());
            int i;
            for (i = 0; i < gameWords.count(); i++) {
              if ( !gameWords[i].isEmpty() )
                qmc2GameInfoDB[gameWords[i]] = gameInfo;
            }
          } else {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
            log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$end' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
            log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$end' in machine info DB %1").arg(pathToGameInfoDB));
#endif
          }
        } else {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
          log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$bio' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
          log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$bio' in machine info DB %1").arg(pathToGameInfoDB));
#endif
        }
      } else if ( !ts.atEnd() ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
        log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$info' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
        log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$info' in machine info DB %1").arg(pathToGameInfoDB));
#endif
      }
      progressBarGamelist->setValue(gameInfoDB.pos());
      if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 )
        qApp->processEvents();
    }
    gameInfoDB.close();
  } else {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't open game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't open machine info DB %1").arg(pathToGameInfoDB));
#endif
  }

  gameInfoElapsedTime = gameInfoElapsedTime.addMSecs(gameInfoTimer.elapsed());
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  log(QMC2_LOG_FRONTEND, tr("done (loading game info DB, elapsed time = %1)").arg(gameInfoElapsedTime.toString("mm:ss.zzz")));
  log(QMC2_LOG_FRONTEND, tr("%n game info record(s) loaded", "", qmc2GameInfoDB.count()));
  if ( qmc2StopParser ) {
    log(QMC2_LOG_FRONTEND, tr("invalidating game info DB"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  log(QMC2_LOG_FRONTEND, tr("done (loading machine info DB, elapsed time = %1)").arg(gameInfoElapsedTime.toString("mm:ss.zzz")));
  log(QMC2_LOG_FRONTEND, tr("%n machine info record(s) loaded", "", qmc2GameInfoDB.count()));
  if ( qmc2StopParser ) {
    log(QMC2_LOG_FRONTEND, tr("invalidating machine info DB"));
#endif
    QMapIterator<QString, QByteArray *> it(qmc2GameInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2GameInfoDB.clear();
  }
  qmc2LoadingGameInfoDB = FALSE;
  qmc2MainWindow->progressBarGamelist->reset();
}

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
void MainWindow::loadEmuInfoDB()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::loadEmuInfoDB()");
#endif

  QTime emuInfoElapsedTime,
        emuInfoTimer;

  qmc2LoadingEmuInfoDB = TRUE;
  qmc2StopParser = FALSE;
  log(QMC2_LOG_FRONTEND, tr("loading emulator info DB"));
  emuInfoTimer.start();

  // clear emulator info DB
  QMapIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
  QList<QByteArray *> deletedRecords;
  while ( it.hasNext() ) {
    it.next();
    if ( !deletedRecords.contains(it.value()) ) {
      if ( it.value() )
        delete it.value();
      deletedRecords.append(it.value());
    }
  }
  deletedRecords.clear();
  qmc2EmuInfoDB.clear();

  bool compressData = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB").toBool();
  QString pathToEmuInfoDB = qmc2Config->value("MAME/FilesAndDirectories/EmuInfoDB").toString();
  QFile emuInfoDB(pathToEmuInfoDB);
  emuInfoDB.open(QIODevice::ReadOnly | QIODevice::Text);
  if ( emuInfoDB.isOpen() ) {
    qmc2MainWindow->progressBarGamelist->reset();
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      progressBarGamelist->setFormat(tr("Emu Info - %p%"));
    else
      progressBarGamelist->setFormat("%p%");
    progressBarGamelist->setRange(0, emuInfoDB.size());
    qApp->processEvents();
    QTextStream ts(&emuInfoDB);
    int recordsProcessed = 0;
    while ( !ts.atEnd() && !qmc2StopParser ) {
      QString singleLine = ts.readLine();
      while ( !singleLine.simplified().startsWith("$info=") && !ts.atEnd() ) {
        singleLine = ts.readLine();
        progressBarGamelist->setValue(emuInfoDB.pos());
        if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 )
          qApp->processEvents();
      }
      if ( singleLine.simplified().startsWith("$info=") ) {
        QStringList gameWords = singleLine.simplified().mid(6).split(",");
        while ( !singleLine.simplified().startsWith("$mame") && !ts.atEnd() ) {
          singleLine = ts.readLine();
          progressBarGamelist->setValue(emuInfoDB.pos());
          if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 )
            qApp->processEvents();
        }
        if ( singleLine.simplified().startsWith("$mame") ) {
          QString emuInfoString;
          while ( !singleLine.simplified().startsWith("$end") && !ts.atEnd() ) {
            singleLine = ts.readLine();
            if ( !singleLine.simplified().startsWith("$end") )
              emuInfoString.append(singleLine + "<br>");
            progressBarGamelist->setValue(emuInfoDB.pos());
            if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 )
              qApp->processEvents();
          }
          if ( singleLine.simplified().startsWith("$end") ) {
            // convert "two (or more) empty lines" to a paragraph delimiter
            emuInfoString = emuInfoString.replace("<br><br><br>", "<p>").replace("<br><br>", "<p>");
            if ( emuInfoString.startsWith("<br>") )
              emuInfoString.remove(0, 4);
            if ( emuInfoString.endsWith("<p>") )
              emuInfoString.remove(emuInfoString.length() - 3, emuInfoString.length() - 1);
            QByteArray *emuInfo;
            if ( compressData )
              emuInfo = new QByteArray(qCompress(emuInfoString.toAscii())); 
            else
              emuInfo = new QByteArray(emuInfoString.toAscii());
            int i;
            for (i = 0; i < gameWords.count(); i++) {
              if ( !gameWords[i].isEmpty() )
                qmc2EmuInfoDB[gameWords[i]] = emuInfo;
            }
          } else {
            log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$end' in emulator info DB %1").arg(pathToEmuInfoDB));
          }
        } else if ( !ts.atEnd() ) {
          log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$mame' in emulator info DB %1").arg(pathToEmuInfoDB));
        }
      } else if ( !ts.atEnd() ) {
        log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$info' in emulator info DB %1").arg(pathToEmuInfoDB));
      }
      progressBarGamelist->setValue(emuInfoDB.pos());
      if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 )
        qApp->processEvents();
    }
    emuInfoDB.close();
  } else
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't open emulator info DB %1").arg(pathToEmuInfoDB));

  emuInfoElapsedTime = emuInfoElapsedTime.addMSecs(emuInfoTimer.elapsed());
  log(QMC2_LOG_FRONTEND, tr("done (loading emulator info DB, elapsed time = %1)").arg(emuInfoElapsedTime.toString("mm:ss.zzz")));
  log(QMC2_LOG_FRONTEND, tr("%n emulator info record(s) loaded", "", qmc2EmuInfoDB.count()));
  if ( qmc2StopParser ) {
    log(QMC2_LOG_FRONTEND, tr("invalidating emulator info DB"));
    QMapIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2EmuInfoDB.clear();
  }
  qmc2LoadingEmuInfoDB = FALSE;
  qmc2MainWindow->progressBarGamelist->reset();
}
#endif

#if QMC2_USE_PHONON_API
void MainWindow::on_actionAudioPreviousTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioPreviousTrack_triggered(bool checked = ...)");
#endif

  toolButtonAudioPreviousTrack->setDown(TRUE);
  QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(on_toolButtonAudioPreviousTrack_resetButton()));
  audioFastForwarding = audioFastBackwarding = FALSE;
  audioSkippingTracks = TRUE;

  if ( listWidgetAudioPlaylist->count() > 0 ) {
    QListWidgetItem *ci = listWidgetAudioPlaylist->currentItem();
    int row;
    if ( ci )
      row = listWidgetAudioPlaylist->currentRow() - 1;
    else
      row = listWidgetAudioPlaylist->count() - 1;
    if ( row < 0 )
      row = listWidgetAudioPlaylist->count() - 1;
    listWidgetAudioPlaylist->clearSelection();
    listWidgetAudioPlaylist->setCurrentRow(row);
    ci = listWidgetAudioPlaylist->currentItem();
    Phonon::State audioState = phononAudioPlayer->state();
    switch ( audioState ) {
      case Phonon::PlayingState:
        on_actionAudioPlayTrack_triggered();
        break;

      default:
        on_actionAudioStopTrack_triggered();
        break;
    }
  }
}

void MainWindow::on_toolButtonAudioPreviousTrack_resetButton()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioPreviousTrack_resetButton()");
#endif

  toolButtonAudioPreviousTrack->setDown(FALSE);
}

void MainWindow::on_actionAudioNextTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioNextTrack_triggered(bool checked = ...)");
#endif

  toolButtonAudioNextTrack->setDown(TRUE);
  QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(on_toolButtonAudioNextTrack_resetButton()));
  audioFastForwarding = audioFastBackwarding = FALSE;
  audioSkippingTracks = TRUE;

  if ( listWidgetAudioPlaylist->count() > 0 ) {
    QListWidgetItem *ci = listWidgetAudioPlaylist->currentItem();
    int row;
    if ( ci )
      row = listWidgetAudioPlaylist->currentRow() + 1;
    else
      row = 0;
    if ( row > listWidgetAudioPlaylist->count() - 1 )
      row = 0;
    listWidgetAudioPlaylist->clearSelection();
    listWidgetAudioPlaylist->setCurrentRow(row);
    ci = listWidgetAudioPlaylist->currentItem();
    Phonon::State audioState = phononAudioPlayer->state();
    switch ( audioState ) {
      case Phonon::PlayingState:
        on_actionAudioPlayTrack_triggered();
        break;

      default:
        on_actionAudioStopTrack_triggered();
        break;
    }
  }
}

void MainWindow::on_toolButtonAudioNextTrack_resetButton()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioNextTrack_resetButton()");
#endif

  toolButtonAudioNextTrack->setDown(FALSE);
}

void MainWindow::on_actionAudioFastBackward_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioFastBackward_triggered(bool checked = ...)");
#endif

  toolButtonAudioFastBackward->setDown(TRUE);
  QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(on_toolButtonAudioFastBackward_resetButton()));

  on_toolButtonAudioFastBackward_clicked(checked);
}

void MainWindow::on_toolButtonAudioFastBackward_clicked(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioFastBackward_clicked(bool checked = ...)");
#endif

  qint64 newTime = phononAudioPlayer->currentTime();
  if ( newTime > 0 ) {
    newTime -= QMC2_AUDIOPLAYER_SEEK_OFFSET;
    audioFastBackwarding = TRUE;
    phononAudioPlayer->seek(newTime);
    audioTick(newTime);
  }
}

void MainWindow::on_toolButtonAudioFastBackward_resetButton()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioFastBackward_resetButton()");
#endif

  toolButtonAudioFastBackward->setDown(FALSE);
  audioFastForwarding = FALSE;
}

void MainWindow::on_actionAudioFastForward_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioFastForward_triggered(bool checked = ...)");
#endif

  toolButtonAudioFastForward->setDown(TRUE);
  QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(on_toolButtonAudioFastForward_resetButton()));

  on_toolButtonAudioFastForward_clicked(checked);
}

void MainWindow::on_toolButtonAudioFastForward_clicked(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonFastForward_clicked(bool checked = ...)");
#endif

  qint64 newTime = phononAudioPlayer->currentTime();
  if ( newTime > 0 ) {
    newTime += QMC2_AUDIOPLAYER_SEEK_OFFSET;
    audioFastForwarding = TRUE;
    phononAudioPlayer->seek(newTime);
    audioTick(newTime);
  }
}

void MainWindow::on_toolButtonAudioFastForward_resetButton()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioFastForward_resetButton()");
#endif

  toolButtonAudioFastForward->setDown(FALSE);
  audioFastForwarding = FALSE;
}

void MainWindow::on_actionAudioStopTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioStopTrack_triggered(bool checked = ...)");
#endif

  actionAudioStopTrack->setChecked(TRUE);
  actionAudioPauseTrack->setChecked(FALSE);
  actionAudioPlayTrack->setChecked(FALSE);
  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = FALSE;

  phononAudioPlayer->stop();
  progressBarAudioProgress->reset();
}

void MainWindow::on_actionAudioPauseTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioPauseTrack_triggered(bool checked = ...)");
#endif

  actionAudioPauseTrack->setChecked(TRUE);
  actionAudioStopTrack->setChecked(FALSE);
  actionAudioPlayTrack->setChecked(FALSE);
  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = FALSE;

  if ( checkBoxAudioFade->isChecked() && phononAudioPlayer->state() == Phonon::PlayingState )
    audioFade(QMC2_AUDIOPLAYER_FADER_PAUSE);
  else
    phononAudioPlayer->pause();
}

void MainWindow::on_actionAudioPlayTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioPlayTrack_triggered(bool checked = ...)");
#endif

  static QString audioPlayerCurrentTrack;

  audioFastForwarding = audioFastBackwarding = FALSE;

  if ( phononAudioPlayer->state() == Phonon::PausedState ) {
    if ( qmc2ProcessManager->sentPlaySignal && qmc2ProcessManager->procMap.count() > 0 ) {
      qmc2ProcessManager->musicWasPlaying = TRUE;
    } else if ( checkBoxAudioFade->isChecked() ) {
      audioFade(QMC2_AUDIOPLAYER_FADER_PLAY);
    } else {
      phononAudioPlayer->play();
      actionAudioPlayTrack->setChecked(TRUE);
      actionAudioStopTrack->setChecked(FALSE);
      actionAudioPauseTrack->setChecked(FALSE);
    }
    qmc2ProcessManager->sentPlaySignal = FALSE;
  } else if ( listWidgetAudioPlaylist->count() > 0 ) {
    QListWidgetItem *ci = listWidgetAudioPlaylist->currentItem();
    if ( !ci ) listWidgetAudioPlaylist->setCurrentRow(0);
    ci = listWidgetAudioPlaylist->currentItem();
    if ( ci->text() != audioPlayerCurrentTrack ) {
      progressBarAudioProgress->reset();
      audioPlayerCurrentTrack = ci->text();
      phononAudioPlayer->setCurrentSource(Phonon::MediaSource(audioPlayerCurrentTrack));
    }
    phononAudioPlayer->play();
    actionAudioPlayTrack->setChecked(TRUE);
    actionAudioStopTrack->setChecked(FALSE);
    actionAudioPauseTrack->setChecked(FALSE);
  } else
    on_actionAudioStopTrack_triggered(TRUE);
}

void MainWindow::on_toolButtonAudioAddTracks_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioAddTracks_clicked()");
#endif

  QStringList sl = QFileDialog::getOpenFileNames(this, tr("Select one or more audio files"), QString(), tr("All files (*)"));
  if ( sl.count() > 0 )
    listWidgetAudioPlaylist->addItems(sl);
}

void MainWindow::on_toolButtonAudioRemoveTracks_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioRemoveTracks_clicked()");
#endif

  QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
  foreach (QListWidgetItem *item, sl) {
    item = listWidgetAudioPlaylist->takeItem(listWidgetAudioPlaylist->row(item));
    delete item;
  }
}

void MainWindow::on_listWidgetAudioPlaylist_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetAudioPlaylist_itemSelectionChanged()");
#endif

  QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
  if ( sl.count() > 0 )
    toolButtonAudioRemoveTracks->setEnabled(TRUE);
  else
    toolButtonAudioRemoveTracks->setEnabled(FALSE);

  if ( sl.count() == 1 && !audioSkippingTracks && !qmc2EarlyStartup ) {
    Phonon::State audioState = phononAudioPlayer->state();
    switch ( audioState ) {
      case Phonon::PlayingState:
        on_actionAudioPlayTrack_triggered();
        break;

      default:
        on_actionAudioStopTrack_triggered();
        break;
    }
  }
}

void MainWindow::on_actionAudioRaiseVolume_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioRaiseVolume_triggered(bool checked = ...)");
#endif

  sliderAudioVolume->setValue(sliderAudioVolume->value() + sliderAudioVolume->pageStep());
}

void MainWindow::on_actionAudioLowerVolume_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioLowerVolume_triggered(bool checked = ...)");
#endif

  sliderAudioVolume->setValue(sliderAudioVolume->value() - sliderAudioVolume->pageStep());
}

void MainWindow::on_sliderAudioVolume_valueChanged(int value)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_sliderAudioVolume_valueChanged(int value = %1)").arg(value));
#endif

  phononAudioOutput->setVolume((qreal)value/100.0);
}

void MainWindow::audioFinished()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::audioFinished()");
#endif

  static QStringList shuffleSelectionList;

  if ( audioFastBackwarding )
    on_actionAudioPreviousTrack_triggered();
  else if ( audioFastForwarding )
    on_actionAudioNextTrack_triggered();
  else if ( checkBoxAudioShuffle->isChecked() ) {
    if ( shuffleSelectionList.count() >= listWidgetAudioPlaylist->count() )
      shuffleSelectionList.clear();
    int newTrackIndex = qrand() % listWidgetAudioPlaylist->count();
    while ( shuffleSelectionList.contains(listWidgetAudioPlaylist->item(newTrackIndex)->text()) ) {
      qApp->processEvents();
      newTrackIndex = qrand() % listWidgetAudioPlaylist->count();
    }
    shuffleSelectionList << listWidgetAudioPlaylist->item(newTrackIndex)->text();
    listWidgetAudioPlaylist->setCurrentRow(newTrackIndex);
    on_actionAudioPlayTrack_triggered();
  } else
    on_actionAudioNextTrack_triggered();
}

void MainWindow::audioTick(qint64 newTime)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::audioTick(qint64 newTime = %1)").arg(newTime));
#endif

  progressBarAudioProgress->setValue(newTime/1000);
}

void MainWindow::audioTotalTimeChanged(qint64 newTotalTime)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::audioTotalTimeChanged(qint64 newTotalTime = %1)").arg(newTotalTime));
#endif

  progressBarAudioProgress->setRange(0, newTotalTime/1000);
  progressBarAudioProgress->setValue(phononAudioPlayer->currentTime()/1000);
}

void MainWindow::audioFade(int faderFunction)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::audioFade(int faderFunction = %1)").arg(faderFunction));
#endif

  int currentVolume = sliderAudioVolume->value();
  double vol;
  double volStep = (double)currentVolume / (double)QMC2_AUDIOPLAYER_FADER_TIMEOUT;
  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = FALSE;
  switch ( faderFunction ) {
    case QMC2_AUDIOPLAYER_FADER_PAUSE:
      actionAudioPauseTrack->setChecked(TRUE);
      actionAudioStopTrack->setChecked(FALSE);
      actionAudioPlayTrack->setChecked(FALSE);
      actionAudioPauseTrack->setEnabled(FALSE);
      toolButtonAudioPauseTrack->setEnabled(FALSE);
      actionAudioPlayTrack->setEnabled(FALSE);
      toolButtonAudioPlayTrack->setEnabled(FALSE);
      actionAudioStopTrack->setEnabled(FALSE);
      toolButtonAudioStopTrack->setEnabled(FALSE);
      for (vol = currentVolume; vol > 0.0; vol -= volStep) {
        sliderAudioVolume->setValue((int)vol);
        qApp->processEvents();
        QTest::qSleep(1);
      }
      phononAudioPlayer->pause();
      actionAudioPauseTrack->setEnabled(TRUE);
      toolButtonAudioPauseTrack->setEnabled(TRUE);
      actionAudioPlayTrack->setEnabled(TRUE);
      toolButtonAudioPlayTrack->setEnabled(TRUE);
      actionAudioStopTrack->setEnabled(TRUE);
      toolButtonAudioStopTrack->setEnabled(TRUE);
      break;

    case QMC2_AUDIOPLAYER_FADER_PLAY:
      sliderAudioVolume->setValue(0);
      actionAudioPauseTrack->setChecked(FALSE);
      actionAudioStopTrack->setChecked(FALSE);
      actionAudioPlayTrack->setChecked(TRUE);
      actionAudioPauseTrack->setEnabled(FALSE);
      toolButtonAudioPauseTrack->setEnabled(FALSE);
      actionAudioPlayTrack->setEnabled(FALSE);
      toolButtonAudioPlayTrack->setEnabled(FALSE);
      actionAudioStopTrack->setEnabled(FALSE);
      toolButtonAudioStopTrack->setEnabled(FALSE);
      qApp->processEvents();
      phononAudioPlayer->play();
      for (vol = 0; vol <= currentVolume; vol += volStep) {
        sliderAudioVolume->setValue((int)vol);
        qApp->processEvents();
        QTest::qSleep(1);
      }
      actionAudioPauseTrack->setEnabled(TRUE);
      toolButtonAudioPauseTrack->setEnabled(TRUE);
      actionAudioPlayTrack->setEnabled(TRUE);
      toolButtonAudioPlayTrack->setEnabled(TRUE);
      actionAudioStopTrack->setEnabled(TRUE);
      toolButtonAudioStopTrack->setEnabled(TRUE);
      break;
  }
  sliderAudioVolume->setValue(currentVolume);
}

void MainWindow::audioMetaDataChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::audioMetaDataChanged()");
#endif

  QString titleMetaData = phononAudioPlayer->metaData(Phonon::TitleMetaData).join(", ");
  QString artistMetaData = phononAudioPlayer->metaData(Phonon::ArtistMetaData).join(", ");
  QString albumMetaData = phononAudioPlayer->metaData(Phonon::AlbumMetaData).join(", ");
  QString genreMetaData = phononAudioPlayer->metaData(Phonon::GenreMetaData).join(", ");
  
  static QString lastTrackInfo = "";
  QString trackInfo = tr("audio player: track info: title = '%1', artist = '%2', album = '%3', genre = '%4'").arg(titleMetaData).arg(artistMetaData).arg(albumMetaData).arg(genreMetaData);
  if ( trackInfo != lastTrackInfo ) {
    log(QMC2_LOG_FRONTEND, trackInfo);
    lastTrackInfo = trackInfo;
  }
}
#else
void MainWindow::on_actionAudioPreviousTrack_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioPreviousTrack_resetButton() { ; }
void MainWindow::on_actionAudioNextTrack_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioNextTrack_resetButton() { ; }
void MainWindow::on_actionAudioFastBackward_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioFastBackward_clicked(bool) { ; }
void MainWindow::on_toolButtonAudioFastBackward_resetButton() { ; }
void MainWindow::on_actionAudioFastForward_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioFastForward_clicked(bool) { ; }
void MainWindow::on_toolButtonAudioFastForward_resetButton() { ; }
void MainWindow::on_actionAudioStopTrack_triggered(bool) { ; }
void MainWindow::on_actionAudioPauseTrack_triggered(bool) { ; }
void MainWindow::on_actionAudioPlayTrack_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioAddTracks_clicked() { ; }
void MainWindow::on_toolButtonAudioRemoveTracks_clicked() { ; }
void MainWindow::on_listWidgetAudioPlaylist_itemSelectionChanged() { ; }
void MainWindow::on_sliderAudioVolume_valueChanged(int) { ; }
void MainWindow::on_actionAudioRaiseVolume_triggered(bool) { ; }
void MainWindow::on_actionAudioLowerVolume_triggered(bool) { ; }
void MainWindow::audioFinished() { ; }
void MainWindow::audioTick(qint64) { ; }
void MainWindow::audioTotalTimeChanged(qint64) { ; }
void MainWindow::audioFade(int) { ; }
void MainWindow::audioMetaDataChanged() { ; }
#endif

void MainWindow::createFifo(bool logFifoCreation)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::createFifo(bool logFifoCreation = ...)");
#endif

#if defined(Q_WS_WIN)
  // FIXME: implement Windows specific notifier FIFO support
#else
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
#if defined(QMC2_SDLMAME)
  mkfifo(QMC2_SDLMAME_OUTPUT_FIFO, S_IRUSR | S_IWUSR | S_IRGRP);
  if ( !EXISTS(QMC2_SDLMAME_OUTPUT_FIFO) ) {
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't create SDLMAME output notifier FIFO, path = %1").arg(QMC2_SDLMAME_OUTPUT_FIFO));
  } else {
    qmc2FifoFile = new QFile(QMC2_SDLMAME_OUTPUT_FIFO);
#if defined(O_ASYNC)
    int fd = ::open(QMC2_SDLMAME_OUTPUT_FIFO, O_ASYNC | O_NONBLOCK);
#else
    int fd = ::open(QMC2_SDLMAME_OUTPUT_FIFO, O_NONBLOCK);
#endif
#elif defined(QMC2_SDLMESS)
  mkfifo(QMC2_SDLMESS_OUTPUT_FIFO, S_IRUSR | S_IWUSR | S_IRGRP);
  if ( !EXISTS(QMC2_SDLMESS_OUTPUT_FIFO) ) {
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't create SDLMESS output notifier FIFO, path = %1").arg(QMC2_SDLMESS_OUTPUT_FIFO));
  } else {
    qmc2FifoFile = new QFile(QMC2_SDLMESS_OUTPUT_FIFO);
#if defined(O_ASYNC)
    int fd = ::open(QMC2_SDLMESS_OUTPUT_FIFO, O_ASYNC | O_NONBLOCK);
#else
    int fd = ::open(QMC2_SDLMESS_OUTPUT_FIFO, O_NONBLOCK);
#endif
#endif
    if ( fd >= 0 ) {
      if ( qmc2FifoFile->open(fd, QIODevice::ReadOnly | QIODevice::Text) ) {
        qmc2FifoNotifier = new QSocketNotifier(qmc2FifoFile->handle(), QSocketNotifier::Read);
        connect(qmc2FifoNotifier, SIGNAL(activated(int)), this, SLOT(processFifoData()));
        qmc2FifoNotifier->setEnabled(TRUE);
        if ( logFifoCreation )
#if defined(QMC2_SDLMAME)
          log(QMC2_LOG_FRONTEND, tr("SDLMAME output notifier FIFO created"));
      } else {
        delete qmc2FifoFile;
        qmc2FifoFile = NULL;
        log(QMC2_LOG_FRONTEND, tr("WARNING: can't open SDLMAME output notifier FIFO for reading, path = %1").arg(QMC2_SDLMAME_OUTPUT_FIFO));
      }
    } else {
      log(QMC2_LOG_FRONTEND, tr("WARNING: can't open SDLMAME output notifier FIFO for reading, path = %1").arg(QMC2_SDLMAME_OUTPUT_FIFO));
    }
  }
#elif defined(QMC2_SDLMESS)
          log(QMC2_LOG_FRONTEND, tr("SDLMESS output notifier FIFO created"));
      } else {
        delete qmc2FifoFile;
        qmc2FifoFile = NULL;
        log(QMC2_LOG_FRONTEND, tr("WARNING: can't open SDLMESS output notifier FIFO for reading, path = %1").arg(QMC2_SDLMESS_OUTPUT_FIFO));
      }
    } else {
      log(QMC2_LOG_FRONTEND, tr("WARNING: can't open SDLMESS output notifier FIFO for reading, path = %1").arg(QMC2_SDLMESS_OUTPUT_FIFO));
    }
  }
#endif
#endif
#endif
}

void MainWindow::recreateFifo()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::recreateFifo()");
#endif

#if defined(Q_WS_WIN)
  // FIXME: implement Windows specific notifier FIFO support
#else
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
  disconnect(qmc2FifoNotifier);
  delete qmc2FifoNotifier;
  qmc2FifoNotifier = NULL;
  if ( qmc2FifoFile->isOpen() )
    qmc2FifoFile->close();
  delete qmc2FifoFile;
#if defined(QMC2_SDLMAME)
  ::unlink(QMC2_SDLMAME_OUTPUT_FIFO);
#elif defined(QMC2_SDLMESS)
  ::unlink(QMC2_SDLMESS_OUTPUT_FIFO);
#endif
  qmc2FifoFile = NULL;
  createFifo(FALSE);
#endif
#endif
}

void MainWindow::processFifoData()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::processFifoData()");
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
  QTextStream ts(qmc2FifoFile);
  QString data = ts.readAll();
  int i;

  if ( data.isEmpty() ) {
    if ( qmc2ProcessManager->procMap.count() <= 0 ) {
      // last emulator exited... recreate & reconnect FIFO to circumvent endless loops due to NULL data
      QTimer::singleShot(0, this, SLOT(recreateFifo()));
    }
    return;
  }

  QStringList sl = data.split("\n");

  for (i = 0; i < sl.count(); i++) { 
    if ( !sl[i].isEmpty() ) {
      QString msgClass, msgPid, msgWhat, msgState;
      QStringList words = sl[i].trimmed().split(" ");
      if ( words.count() > 0 )
        msgClass = words[0];
      if ( words.count() > 1 )
        msgPid = words[1];
      if ( words.count() > 2 )
        msgWhat = words[2];
      if ( words.count() > 3 )
        msgState = words[3];
      if ( !msgPid.isEmpty() ) {
        QList<QTreeWidgetItem *> il = treeWidgetEmulators->findItems(msgPid, Qt::MatchExactly, QMC2_EMUCONTROL_COLUMN_PID);
        if ( il.count() > 0 ) {
          if ( msgClass == "MAME" ) {
            if ( msgWhat == "START" ) {
              il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("running"));
            } else if ( msgWhat == "STOP" ) {
              il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("stopped"));
            } else {
#if defined(QMC2_SDLMAME)
              log(QMC2_LOG_FRONTEND, tr("unhandled MAME output notification: game = %1, class = %2, what = %3, state = %4").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_GAME)).arg(msgClass).arg(msgWhat).arg(msgState));
#elif defined(QMC2_SDLMESS)
              log(QMC2_LOG_FRONTEND, tr("unhandled MESS output notification: game = %1, class = %2, what = %3, state = %4").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_GAME)).arg(msgClass).arg(msgWhat).arg(msgState));
#endif
            }
          } else if ( msgClass == "OUT" ) {
            // refresh static output notifiers
            if ( msgWhat == "led0" ) {
              if ( msgState == "1" )
                il[0]->setIcon(QMC2_EMUCONTROL_COLUMN_LED0, QIcon(QString::fromUtf8(":/data/img/led_on.png")));
              else
                il[0]->setIcon(QMC2_EMUCONTROL_COLUMN_LED0, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
            } else if ( msgWhat == "led1" ) {
              if ( msgState == "1" )
                il[0]->setIcon(QMC2_EMUCONTROL_COLUMN_LED1, QIcon(QString::fromUtf8(":/data/img/led_on.png")));
              else
                il[0]->setIcon(QMC2_EMUCONTROL_COLUMN_LED1, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
            } else if ( msgWhat == "pause" ) {
              if ( msgState == "1" )
                il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("paused"));
              else
                il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("running"));
            } else {
               // add or refresh dynamic output notifiers
               QTreeWidgetItem *itemFound = NULL;
               int i;
               for (i = 0; i < il[0]->childCount() && itemFound == NULL; i++) {
                 QTreeWidgetItem *item = il[0]->child(i);
                 if ( item->text(QMC2_EMUCONTROL_COLUMN_GAME) == msgWhat )
                   itemFound = item;
               }
               if ( itemFound != NULL ) {
                 itemFound->setText(QMC2_EMUCONTROL_COLUMN_STATUS, msgState);
               } else {
                 itemFound = new QTreeWidgetItem(il[0]);
                 itemFound->setText(QMC2_EMUCONTROL_COLUMN_GAME, msgWhat);
                 itemFound->setText(QMC2_EMUCONTROL_COLUMN_STATUS, msgState);
                 if ( il[0]->childCount() == 1 ) {
                   // this is a workaround for a minor Qt bug: the root decoration
                   // isn't updated correctly on the first child item insertion
                   treeWidgetEmulators->setRootIsDecorated(FALSE);
                   treeWidgetEmulators->setRootIsDecorated(TRUE);
                 }
               }
            }
          } else {
#if defined(QMC2_SDLMAME)
            log(QMC2_LOG_FRONTEND, tr("unhandled MAME output notification: game = %1, class = %2, what = %3, state = %4").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_GAME)).arg(msgClass).arg(msgWhat).arg(msgState));
#elif defined(QMC2_SDLMESS)
            log(QMC2_LOG_FRONTEND, tr("unhandled MESS output notification: game = %1, class = %2, what = %3, state = %4").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_GAME)).arg(msgClass).arg(msgWhat).arg(msgState));
#endif
          }
        }
      }
    }
  }
#endif
}

void MainWindow::on_romStateFilterC_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterC_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowC->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_C, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_romStateFilterM_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterM_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowM->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_M, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_romStateFilterI_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterI_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowI->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_I, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_romStateFilterN_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterN_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowN->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_N, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_romStateFilterU_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterU_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowU->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_U, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_treeWidgetGamelist_headerSectionClicked(int logicalIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetGamelist_headerSectionClicked(int logicalIndex = %1)").arg(logicalIndex));
#endif

  qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(FALSE);
  qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(FALSE);

  switch ( logicalIndex ) {
    case QMC2_GAMELIST_COLUMN_GAME:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_DESCRIPTION )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_DESCRIPTION);
      break;

    case QMC2_GAMELIST_COLUMN_YEAR:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_YEAR )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_YEAR);
      break;

    case QMC2_GAMELIST_COLUMN_MANU:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_MANUFACTURER )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_MANUFACTURER);
      break;

    case QMC2_GAMELIST_COLUMN_NAME:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_GAMENAME )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_GAMENAME);
      break;

    default:
      break;
  }

  QTimer::singleShot(0, qmc2Options, SLOT(on_pushButtonApply_clicked()));
}

void MainWindow::on_menuTabWidgetGamelist_North_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGamelist_North_activated()"));
#endif

  tabWidgetGamelist->setTabPosition(QTabWidget::North);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::North);
}

void MainWindow::on_menuTabWidgetGamelist_South_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGamelist_South_activated()"));
#endif

  tabWidgetGamelist->setTabPosition(QTabWidget::South);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::South);
}

void MainWindow::on_menuTabWidgetGamelist_West_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGamelist_West_activated()"));
#endif

  tabWidgetGamelist->setTabPosition(QTabWidget::West);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::West);
}

void MainWindow::on_menuTabWidgetGamelist_East_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGamelist_East_activated()"));
#endif

  tabWidgetGamelist->setTabPosition(QTabWidget::East);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::East);
}

void MainWindow::on_tabWidgetGamelist_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetGamelist_customContextMenuRequested(const QPoint &p = ...)");
#endif

  if ( !tabWidgetGamelist->currentWidget()->childrenRect().contains(p, TRUE) ) {
    menuTabWidgetGamelist->move(tabWidgetGamelist->mapToGlobal(p));
    menuTabWidgetGamelist->show();
  }
}

void MainWindow::on_menuTabWidgetGameDetail_North_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_North_activated()"));
#endif

  tabWidgetGameDetail->setTabPosition(QTabWidget::North);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::North);
}

void MainWindow::on_menuTabWidgetGameDetail_South_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_South_activated()"));
#endif

  tabWidgetGameDetail->setTabPosition(QTabWidget::South);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::South);
}

void MainWindow::on_menuTabWidgetGameDetail_West_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_West_activated()"));
#endif

  tabWidgetGameDetail->setTabPosition(QTabWidget::West);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::West);
}

void MainWindow::on_menuTabWidgetGameDetail_East_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_East_activated()"));
#endif

  tabWidgetGameDetail->setTabPosition(QTabWidget::East);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::East);
}

void MainWindow::on_menuTabWidgetGameDetail_Setup_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_Setup_activated()"));
#endif

  if ( qmc2DetailSetup->isHidden() )
    qmc2DetailSetup->show();
  else if ( qmc2DetailSetup->isMinimized() )
    qmc2DetailSetup->showNormal();

  QTimer::singleShot(0, qmc2DetailSetup, SLOT(raise()));
}

void MainWindow::on_tabWidgetGameDetail_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetGameDetail_customContextMenuRequested(const QPoint &p = ...)");
#endif

  if ( tabWidgetGameDetail->currentWidget() ) {
    if ( !tabWidgetGameDetail->currentWidget()->childrenRect().contains(p, TRUE) ) {
      menuTabWidgetGameDetail->move(tabWidgetGameDetail->mapToGlobal(p));
      menuTabWidgetGameDetail->show();
    }
  } else {
    menuTabWidgetGameDetail->move(tabWidgetGameDetail->mapToGlobal(p));
    menuTabWidgetGameDetail->show();
  }
}

void MainWindow::on_menuTabWidgetLogsAndEmulators_North_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetLogsAndEmulators_North_activated()"));
#endif

  tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::North);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::North);
}

void MainWindow::on_menuTabWidgetLogsAndEmulators_South_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetLogsAndEmulators_South_activated()"));
#endif

  tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::South);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::South);
}

void MainWindow::on_menuTabWidgetLogsAndEmulators_West_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetLogsAndEmulators_West_activated()"));
#endif

  tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::West);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::West);
}

void MainWindow::on_menuTabWidgetLogsAndEmulators_East_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetLogsAndEmulators_East_activated()"));
#endif

  tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::East);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::East);
}

void MainWindow::on_tabWidgetLogsAndEmulators_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetLogsAndEmulators_customContextMenuRequested(const QPoint &p = ...)");
#endif

  if ( !tabWidgetLogsAndEmulators->currentWidget()->childrenRect().contains(p, TRUE) ) {
    menuTabWidgetLogsAndEmulators->move(tabWidgetLogsAndEmulators->mapToGlobal(p));
    menuTabWidgetLogsAndEmulators->show();
  }
}

void MainWindow::on_treeWidgetHierarchy_headerSectionClicked(int logicalIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetHierarchy_headerSectionClicked(int logicalIndex = %1)").arg(logicalIndex));
#endif

  on_treeWidgetGamelist_headerSectionClicked(logicalIndex);
}

void MainWindow::on_actionArcadeShowFPS_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_actionArcadeShowFPS_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2ArcadeView )
    qmc2ArcadeView->menuScene->toggleFps();
}

void MainWindow::on_actionArcadeTakeScreenshot_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_actionArcadeTakeScreenshot_triggered()"));
#endif

  if ( qmc2ArcadeView ) {
    if ( !exitArcade )
      qmc2ArcadeView->takeScreenshot();
    else
      log(QMC2_LOG_FRONTEND, tr("ArcadeView is not currently active, can't take screen shot"));
  }
}

void MainWindow::on_comboBoxViewSelect_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_comboBoxViewSelect_currentIndexChanged(int index = %1)").arg(index));
#endif

  if ( index == QMC2_VIEWHIERARCHY_INDEX )
    pushButtonSelectRomFilter->setVisible(FALSE);
  else
    pushButtonSelectRomFilter->setVisible(TRUE);
}

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
void MainWindow::mawsLoadFinished(bool ok)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::mawsLoadFinished(bool ok = %1)").arg(ok));
#endif

#if QT_VERSION >= 0x040500
  if ( qmc2CurrentItem && qmc2MAWSLookup && ok ) {
    QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
    // only cache the ROM set page, don't cache followed pages
    if ( qmc2MAWSLookup->webViewBrowser->url().toString() == qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName) ) {
      QString mawsHtml = qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->toHtml();
      mawsHtml.replace("href=\"#top\"", QString("href=\"%1#top\"").arg(qmc2MAWSLookup->webViewBrowser->url().path()));
      int startIndex = mawsHtml.indexOf("<div class=\"ifFoot\"");
      mawsHtml.remove(startIndex, mawsHtml.indexOf("</div>") + 6);
      mawsHtml.insert(startIndex, QString("<p><font size=\"-1\">Copyright &copy; 2004 - %1 <a href=\"%2\"><b>MAWS</b></a>, All Rights Reserved</font></p>").arg(QDate::currentDate().year()).arg(QMC2_MAWS_HOMEPAGE_URL));
      QPoint scrollPos = qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->scrollPosition();
      qmc2MAWSLookup->webViewBrowser->setUpdatesEnabled(FALSE);
      qmc2MAWSLookup->webViewBrowser->setHtml(mawsHtml, QUrl(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName)));
      qmc2MAWSLookup->webViewBrowser->stop();
      qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->setScrollPosition(scrollPos);
      qmc2MAWSLookup->webViewBrowser->setUpdatesEnabled(TRUE);
      QByteArray mawsData = qCompress(qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->toHtml().toLatin1());
      if ( qmc2MAWSCache.contains(gameName) ) {
        qmc2MAWSCache.remove(gameName);
#ifdef QMC2_DEBUG
        log(QMC2_LOG_FRONTEND, QString("DEBUG: MAWS cache: URL exists, updating cache entry for '%1'").arg(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName)));
#endif
      } else {
#ifdef QMC2_DEBUG
        log(QMC2_LOG_FRONTEND, QString("DEBUG: MAWS cache: URL not found, creating cache entry for '%1'").arg(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName)));
#endif
      }
      qmc2MAWSCache.insert(gameName, new QByteArray(mawsData), mawsData.size());
#ifdef QMC2_DEBUG
      log(QMC2_LOG_FRONTEND, QString("DEBUG: MAWS cache: %1% filled").arg((double)100.0 * ((double)qmc2MAWSCache.totalCost()/(double)qmc2MAWSCache.maxCost()), 0, 'f', 2));
#endif

      // save compressed page to MAWS disk cache
      QDir mawsCacheDir(qmc2Config->value("MAME/FilesAndDirectories/MAWSCacheDirectory").toString());
      if ( mawsCacheDir.exists() ) {
        QFile mawsCacheFile(mawsCacheDir.filePath(gameName + ".wc"));
        if ( mawsCacheFile.open(QIODevice::WriteOnly) ) {
          QTextStream ts(&mawsCacheFile);
#if defined(Q_WS_WIN)
          ts << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\r\n";
          ts << "TIMESTAMP\t" + QString::number(QDateTime::currentDateTime().toTime_t()) + "\r\n";
#else
          ts << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
          ts << "TIMESTAMP\t" + QString::number(QDateTime::currentDateTime().toTime_t()) + "\n";
#endif
#if defined(QMC2_WC_COMPRESSION_ENABLED)
          ts << mawsData;
#else
          ts << qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->toHtml();
#endif
          ts.flush();
          mawsCacheFile.close();
        }
      }
    }
#ifdef QMC2_DEBUG
    else
      log(QMC2_LOG_FRONTEND, QString("DEBUG: MAWS cache: ignoring URL '%1'").arg(qmc2MAWSLookup->webViewBrowser->url().toString()));
#endif
  }
#endif
}
#endif

void myQtMessageHandler(QtMsgType type, const char *msg)
{
  if ( qmc2SuppressQtMessages )
    return;

  if ( !qmc2GuiReady )
    return;

  switch ( type ) {
    case QtDebugMsg:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, "QtDebugMsg: " + QString(msg));
      break;

    case QtWarningMsg:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, "QtWarningMsg: " + QString(msg));
      break;

    case QtCriticalMsg:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, "QtCriticalMsg: " + QString(msg));
      break;

    case QtFatalMsg:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, "QtFatalMsg: " + QString(msg));
      break;

    default:
      break;
  }
}

void prepareShortcuts()
{
  // shortcuts
  qmc2ShortcutMap["Ctrl+1"].second = qmc2MainWindow->actionCheckROMs;
  qmc2ShortcutMap["Ctrl+2"].second = qmc2MainWindow->actionCheckSamples;
  qmc2ShortcutMap["Ctrl+3"].second = qmc2MainWindow->actionCheckPreviews;
  qmc2ShortcutMap["Ctrl+4"].second = qmc2MainWindow->actionCheckFlyers;
  qmc2ShortcutMap["Ctrl+5"].second = qmc2MainWindow->actionCheckIcons;
  qmc2ShortcutMap["Ctrl+A"].second = qmc2MainWindow->actionAbout;
  qmc2ShortcutMap["Ctrl+D"].second = qmc2MainWindow->actionAnalyseCurrentROM;
  qmc2ShortcutMap["Ctrl+E"].second = qmc2MainWindow->actionExportROMStatus;
  qmc2ShortcutMap["Ctrl+F"].second = qmc2MainWindow->actionToFavorites;
  qmc2ShortcutMap["Ctrl+H"].second = qmc2MainWindow->actionDocumentation;
  qmc2ShortcutMap["Ctrl+I"].second = qmc2MainWindow->actionClearImageCache;
  qmc2ShortcutMap["Ctrl+Shift+A"].second = qmc2MainWindow->actionArcadeSetup;
#if QT_VERSION >= 0x040500
  qmc2ShortcutMap["Ctrl+M"].second = qmc2MainWindow->actionClearMAWSCache;
#endif
  qmc2ShortcutMap["Ctrl+N"].second = qmc2MainWindow->actionClearIconCache;
  qmc2ShortcutMap["Ctrl+O"].second = qmc2MainWindow->actionOptions;
  qmc2ShortcutMap["Ctrl+P"].second = qmc2MainWindow->actionPlay;
  qmc2ShortcutMap["Ctrl+Q"].second = qmc2MainWindow->actionAboutQt;
  qmc2ShortcutMap["Ctrl+R"].second = qmc2MainWindow->actionReload;
  qmc2ShortcutMap["Ctrl+S"].second = qmc2MainWindow->actionCheckCurrentROM;
  qmc2ShortcutMap["Ctrl+T"].second = qmc2MainWindow->actionRecreateTemplateMap;
  qmc2ShortcutMap["Ctrl+X"].second = qmc2MainWindow->actionExitStop;
  qmc2ShortcutMap["Ctrl+Z"].second = qmc2MainWindow->actionROMAlyzer;
  qmc2ShortcutMap["Ctrl+Alt+C"].second = qmc2MainWindow->actionRomStatusFilterC;
  qmc2ShortcutMap["Ctrl+Alt+M"].second = qmc2MainWindow->actionRomStatusFilterM;
  qmc2ShortcutMap["Ctrl+Alt+I"].second = qmc2MainWindow->actionRomStatusFilterI;
  qmc2ShortcutMap["Ctrl+Alt+N"].second = qmc2MainWindow->actionRomStatusFilterN;
  qmc2ShortcutMap["Ctrl+Alt+U"].second = qmc2MainWindow->actionRomStatusFilterU;
#if defined(QMC2_VARIANT_LAUNCHER)
  qmc2ShortcutMap["Ctrl+Alt+1"].second = qmc2MainWindow->actionLaunchQMC2MAME;
  qmc2ShortcutMap["Ctrl+Alt+2"].second = qmc2MainWindow->actionLaunchQMC2MESS;
#endif
  qmc2ShortcutMap["F5"].second = qmc2MainWindow->actionViewFullDetail;
  qmc2ShortcutMap["F6"].second = qmc2MainWindow->actionViewParentClones;
  qmc2ShortcutMap["F11"].second = qmc2MainWindow->actionFullscreenToggle;
  qmc2ShortcutMap["F12"].second = qmc2MainWindow->actionArcadeToggle;
  qmc2ShortcutMap["Meta+F"].second = qmc2MainWindow->actionArcadeShowFPS;
  qmc2ShortcutMap["Meta+F12"].second = qmc2MainWindow->actionArcadeTakeScreenshot;
#if QMC2_USE_PHONON_API
  qmc2ShortcutMap["Ctrl+Alt+-"].second = qmc2MainWindow->actionAudioPreviousTrack;
  qmc2ShortcutMap["Ctrl+Alt++"].second = qmc2MainWindow->actionAudioNextTrack;
  qmc2ShortcutMap["Ctrl+Alt+B"].second = qmc2MainWindow->actionAudioFastBackward;
  qmc2ShortcutMap["Ctrl+Alt+F"].second = qmc2MainWindow->actionAudioFastForward;
  qmc2ShortcutMap["Ctrl+Alt+S"].second = qmc2MainWindow->actionAudioStopTrack;
  qmc2ShortcutMap["Ctrl+Alt+#"].second = qmc2MainWindow->actionAudioPauseTrack;
  qmc2ShortcutMap["Ctrl+Alt+P"].second = qmc2MainWindow->actionAudioPlayTrack;
  qmc2ShortcutMap["Ctrl+Alt+PgUp"].second = qmc2MainWindow->actionAudioRaiseVolume;
  qmc2ShortcutMap["Ctrl+Alt+PgDown"].second = qmc2MainWindow->actionAudioLowerVolume;
#endif

  // special keys
  qmc2ShortcutMap["+"].second = NULL;
  qmc2QtKeyMap["+"] = Qt::Key_Plus;
  qmc2ShortcutMap["-"].second = NULL;
  qmc2QtKeyMap["-"] = Qt::Key_Minus;
  qmc2ShortcutMap["Down"].second = NULL;
  qmc2QtKeyMap["Down"] = Qt::Key_Down;
  qmc2ShortcutMap["End"].second = NULL;
  qmc2QtKeyMap["End"] = Qt::Key_End;
  qmc2ShortcutMap["Esc"].second = NULL;
  qmc2QtKeyMap["Esc"] = Qt::Key_Escape;
  qmc2ShortcutMap["Left"].second = NULL;
  qmc2QtKeyMap["Left"] = Qt::Key_Left;
  qmc2ShortcutMap["Home"].second = NULL;
  qmc2QtKeyMap["Home"] = Qt::Key_Home;
  qmc2ShortcutMap["PgDown"].second = NULL;
  qmc2QtKeyMap["PgDown"] = Qt::Key_PageDown;
  qmc2ShortcutMap["PgUp"].second = NULL;
  qmc2QtKeyMap["PgUp"] = Qt::Key_PageUp;
  qmc2ShortcutMap["Return"].second = NULL;
  qmc2QtKeyMap["Return"] = Qt::Key_Return;
  qmc2ShortcutMap["Right"].second = NULL;
  qmc2QtKeyMap["Right"] = Qt::Key_Right;
  qmc2ShortcutMap["Tab"].second = NULL;
  qmc2QtKeyMap["Tab"] = Qt::Key_Tab;
  qmc2ShortcutMap["Up"].second = NULL;
  qmc2QtKeyMap["Up"] = Qt::Key_Up;

  qmc2Options->setupShortcutActions();
}

int main(int argc, char *argv[])
{
  qsrand(QDateTime::currentDateTime().toTime_t());

  // install message handler
  qInstallMsgHandler(myQtMessageHandler);

  // prepare application and resources
  QApplication qmc2App(argc, argv);
  Q_INIT_RESOURCE(qmc2);
#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  qmc2App.setWindowIcon(QIcon(QString::fromUtf8(":/data/img/mess.png")));
#elif defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  qmc2App.setWindowIcon(QIcon(QString::fromUtf8(":/data/img/mame.png")));
#endif

  // check configuration
  int checkReturn = QDialog::Accepted;
  qmc2Welcome = new Welcome(0);
  if ( !qmc2Welcome->checkOkay )
    checkReturn = qmc2Welcome->exec();
  delete qmc2Welcome;
  if ( checkReturn != QDialog::Accepted )
    exit(1);

  // setup key event filter
  qmc2KeyPressFilter = new KeyPressFilter(0);
  qmc2App.installEventFilter(qmc2KeyPressFilter);

  // create mandatory objects and prepare shortcuts
  qmc2Options = new Options(0);
  qmc2Config = qmc2Options->config;
  qmc2ProcessManager = new ProcessManager(0);
  qmc2MainWindow = new MainWindow(0);

  // prepare & restore game/machine detail setup
  qmc2DetailSetup = new DetailSetup(qmc2MainWindow);
  qmc2DetailSetup->saveDetail();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    qmc2MainWindow->tabWidgetGameDetail->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GameDetailTab", 0).toInt());
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    qmc2MainWindow->tabWidgetGameDetail->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineDetailTab", 0).toInt());
#endif
  }

  // finalize initial setup
  qmc2Options->apply();
  qmc2GuiReady = TRUE;
  prepareShortcuts();
  QTimer::singleShot(0, qmc2Options, SLOT(on_pushButtonApply_clicked()));
  QTimer::singleShot(0, qmc2Options, SLOT(checkShortcuts()));
#if QMC2_JOYSTICK == 1
  QTimer::singleShot(0, qmc2Options, SLOT(checkJoystickMappings()));
#endif

  // create & show greeting string
  QString greeting;
#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  greeting = QObject::tr("M.E.S.S. Catalog / Launcher II v") +
#elif defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  greeting = QObject::tr("M.A.M.E. Catalog / Launcher II v") +
#else
  greeting = QObject::tr("M.A.M.E. Catalog / Launcher II v") +
#endif
             QString::number(MAJOR) + "." + QString::number(MINOR) +
#ifdef BETA
             ".b" + QString::number(BETA) +
#endif
             " (Qt " + qVersion() +
#if defined(QMC2_SDLMAME)
             ", SDLMAME, " +
#elif defined(QMC2_SDLMESS)
             ", SDLMESS, " +
#elif defined(QMC2_MAME)
             ", MAME, " +
#elif defined(QMC2_MESS)
             ", MESS, " +
#else
             ", ???, " +
#endif
             qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language").toString() + ")";
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, greeting);

#if QMC2_OPENGL == 1
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("OpenGL features enabled"));
#endif

#if QMC2_USE_PHONON_API
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("Phonon features enabled - using Phonon v%1").arg(Phonon::phononVersion()));
#endif

#if QMC2_JOYSTICK == 1
  const SDL_version *sdlVersion = SDL_Linked_Version();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("SDL joystick support enabled - using SDL v%1.%2.%3").arg(sdlVersion->major).arg(sdlVersion->minor).arg(sdlVersion->patch));
#endif

  // process global emulator configuration and create import/export popup menus
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("processing global emulator configuration"));
  QVBoxLayout *layout = new QVBoxLayout;
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  qmc2GlobalEmulatorOptions = new EmulatorOptions("MAME/Configuration/Global", qmc2Options->tabGlobalConfiguration);
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  qmc2GlobalEmulatorOptions = new EmulatorOptions("MESS/Configuration/Global", qmc2Options->tabGlobalConfiguration);
#endif
  qmc2GlobalEmulatorOptions->load();
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
  layout->addWidget(qmc2GlobalEmulatorOptions);
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile = new QPushButton(QObject::tr("Export to..."), qmc2Options);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export global MAME configuration"));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export global MAME configuration"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export global MESS configuration"));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export global MESS configuration"));
#endif
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile = new QPushButton(QObject::tr("Import from..."), qmc2Options);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import global MAME configuration"));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import global MAME configuration"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import global MESS configuration"));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import global MESS configuration"));
#endif
  buttonLayout->addWidget(qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile);
  buttonLayout->addWidget(qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile);
  layout->addLayout(buttonLayout);
  qmc2Options->tabGlobalConfiguration->setLayout(layout);
  qmc2GlobalEmulatorOptions->show();
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->show();
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->show();
  // export/import menus
  qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile = new QMenu(qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), QObject::tr("<inipath>/mame.ini")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsExportToFile_clicked()));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), QObject::tr("<inipath>/mess.ini")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsExportToFile_clicked()));
#endif
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), QObject::tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsSelectExportFile_clicked()));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setMenu(qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile);
  qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile = new QMenu(qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile);
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), QObject::tr("<inipath>/mame.ini")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked()));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), QObject::tr("<inipath>/mess.ini")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked()));
#endif
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), QObject::tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsSelectImportFile_clicked()));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setMenu(qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile);
#endif
  qmc2GlobalEmulatorOptions->pseudoConstructor();

  // run the application
  return qmc2App.exec();
}
