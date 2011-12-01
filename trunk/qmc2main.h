#ifndef _QMC2_MAIN_H_
#define _QMC2_MAIN_H_

#include <QApplication>
#include <QCloseEvent>
#include <QSocketNotifier>
#include <QTimer>
#include <QTime>
#include <QNetworkReply>
#include <QWidgetAction>
#include "ui_qmc2main.h"
#include "ui_options.h"
#include "macros.h"
#if QMC2_USE_PHONON_API
#include "qmc2_phonon.h"
#endif

class KeyPressFilter : public QObject
{
  Q_OBJECT

  public:
    KeyPressFilter(QObject *parent = NULL) : QObject(parent) { ; }

  protected:
    bool eventFilter(QObject *, QEvent *);
};

#if defined(QMC2_EMUTYPE_MAME)
class AutoPopupToolButton : public QToolButton
{
  Q_OBJECT

  public:
    QTimer menuTimer;

    AutoPopupToolButton(QWidget *parent = 0) : QToolButton(parent)
    {
      connect(&menuTimer, SIGNAL(timeout()), this, SLOT(hideMenu()));
    }

  signals:
    void paintFinished();
    void menuHidden();

  public slots:
    void hideMenu()
    {
      if ( menu() )
        if ( menu()->activeAction() == NULL ) {
          QTimer::singleShot(0, menu(), SLOT(hide()));
          emit menuHidden();
        }
    }

  protected:
    void enterEvent(QEvent *e)
    {
      QToolButton::enterEvent(e);
      QTimer::singleShot(0, this, SLOT(showMenu()));
      menuTimer.stop();
    }
    void leaveEvent(QEvent *e)
    {
      QToolButton::leaveEvent(e);
      menuTimer.start(1000);
    }
    void paintEvent(QPaintEvent *e)
    {
      QToolButton::paintEvent(e);
      emit paintFinished();
    }
};
#endif

class MainWindow : public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT

  public:
    bool isActiveState;
    QTimer searchTimer;
    QTimer updateTimer;
    QPixmap qmc2GhostImagePixmap;
    QPushButton *pushButtonGlobalEmulatorOptionsExportToFile;
    QMenu *selectMenuGlobalEmulatorOptionsExportToFile;
    QPushButton *pushButtonGlobalEmulatorOptionsImportFromFile;
    QMenu *selectMenuGlobalEmulatorOptionsImportFromFile;
    QPushButton *pushButtonCurrentEmulatorOptionsExportToFile;
    QMenu *selectMenuCurrentEmulatorOptionsExportToFile;
    QPushButton *pushButtonCurrentEmulatorOptionsImportFromFile;
    QMenu *selectMenuCurrentEmulatorOptionsImportFromFile;
    QLabel *labelEmuSelector;
    QComboBox *comboBoxEmuSelector;
    QMenu *menuRomStatusFilter;
    QMenu *menuTabWidgetGamelist;
    QMenu *menuTabWidgetGameDetail;
    QMenu *menuTabWidgetLogsAndEmulators;
    QMenu *menuHorizontalSplitter;
    QMenu *menuVerticalSplitter;
    QMenu *menuGamelistHeader;
    QMenu *menuHierarchyHeader;
#if defined(QMC2_EMUTYPE_MAME)
    QMenu *menuCategoryHeader;
    QMenu *menuVersionHeader;
    QAction *actionMenuGamelistHeaderCategory;
    QAction *actionMenuGamelistHeaderVersion;
    QAction *actionMenuHierarchyHeaderCategory;
    QAction *actionMenuHierarchyHeaderVersion;
#endif
    QAction *actionRomStatusFilterC;
    QAction *actionRomStatusFilterM;
    QAction *actionRomStatusFilterI;
    QAction *actionRomStatusFilterN;
    QAction *actionRomStatusFilterU;
    QList<QAction *> contextMenuPlayActions;
#if QMC2_JOYSTICK == 1
    int joyIndex;
#endif
#if QMC2_USE_PHONON_API
    Phonon::MediaObject *phononAudioPlayer;
    Phonon::AudioOutput *phononAudioOutput;
    Phonon::Path phononAudioPath;
    bool audioFastForwarding;
    bool audioFastBackwarding;
    bool audioSkippingTracks;
    Phonon::State audioState;
#endif
#if defined(QMC2_EMUTYPE_MAME)
    AutoPopupToolButton *toolButtonMAWSQuickLinks;
    QMenu *menuMAWSQuickLinks;
    QMap<QString, QAction *> mawsQDLActions;
#endif
#if defined(QMC2_SHOWMEMINFO)
    QTimer memoryUpdateTimer;
#endif
#if defined(Q_WS_X11)
    QWidget *widgetEmbeddedEmus;
    QWidget *embedderCornerWidget;
    QHBoxLayout *embedderCornerLayout;
    QToolButton *toolButtonEmbedderMaximizeToggle;
    QToolButton *toolButtonEmbedderAutoPause;
    QMenu *menuAutoPause;
#endif
    QList<int> hSplitterSizes;
    QRect desktopGeometry;
    QWidget *hSplitterWidget0;
    QWidget *vSplitterWidget0;
    QTimer activityCheckTimer;
    bool activityState;
    QString urlSectionRegExp;

    QWidgetAction *widgetActionToolbarSearch;
    QComboBox *comboBoxToolbarSearch;

    static QColor qmc2StatusColorGreen;
    static QColor qmc2StatusColorYellowGreen;
    static QColor qmc2StatusColorRed;
    static QColor qmc2StatusColorBlue;
    static QColor qmc2StatusColorGrey;

    int sortCriteriaLogicalIndex();
    QPoint adjustedWidgetPosition(QPoint, QWidget *);
    QStringList &getXmlChoices(QString, QString, QString optionAttribute = QString());

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

  public slots:
    // game menu
    void on_actionPlay_activated();
    void on_actionPlayEmbedded_activated();
    void on_actionPlayTagged_activated();
    void on_actionPlayEmbeddedTagged_activated();
    void on_actionToFavorites_activated();
    void on_actionToFavoritesTagged_activated();
    void on_actionReload_activated();
    void on_actionExitStop_activated();
    void on_actionCheckCurrentROM_activated();
    void on_actionCheckROMStateTagged_activated();
    void on_actionAnalyseCurrentROM_activated();
    void on_actionAnalyseROMTagged_activated();
    void on_actionRunRomTool_activated();
    void on_actionRunRomToolTagged_activated();
    void on_actionSetTag_activated();
    void on_actionUnsetTag_activated();
    void on_actionToggleTag_activated();
    void on_actionTagAll_activated();
    void on_actionUntagAll_activated();
    void on_actionInvertTags_activated();

    // arcade menu
    void on_actionArcadeToggle_activated();
    void on_actionArcadeSetup_activated();

    // tools menu
    void on_actionCheckROMs_activated();
    void on_actionCheckSamples_activated();
    void on_actionCheckPreviews_activated();
    void on_actionCheckFlyers_activated();
    void on_actionCheckIcons_activated();
    void on_actionROMAlyzer_activated();
    void on_actionExportROMStatus_activated();
    void on_actionDemoMode_activated();
    void on_actionClearImageCache_activated();
    void on_actionClearIconCache_activated();
#if defined(QMC2_EMUTYPE_MAME)
    void on_actionClearMAWSCache_activated();
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
    void on_actionClearYouTubeCache_activated();
#endif
    void on_actionRecreateTemplateMap_activated();
    void on_actionCheckTemplateMap_activated();
    void on_actionClearROMStateCache_activated();
    void on_actionClearGamelistCache_activated();
    void on_actionClearXMLCache_activated();
    void on_actionClearSoftwareListCache_activated();
    void on_actionClearAllEmulatorCaches_activated();
    void on_actionOptions_activated();

    // display menu
    void on_actionFullscreenToggle_activated();
    void on_actionLaunchQMC2MAME_activated();
    void on_actionLaunchQMC2MESS_activated();

    // help menu
    void on_actionDocumentation_activated();
    void on_actionAbout_activated();
    void on_actionAboutQt_activated();

    // search widget
    void on_comboBoxSearch_editTextChanged(const QString &);
    void on_comboBoxSearch_editTextChanged_delayed();
    void on_comboBoxSearch_activated(const QString &);
    void on_listWidgetSearch_currentTextChanged(QString);
    void on_listWidgetSearch_itemActivated(QListWidgetItem *);
    void on_listWidgetSearch_currentItemChanged(QListWidgetItem *, QListWidgetItem *);
    void on_listWidgetSearch_itemPressed(QListWidgetItem *);
    void on_listWidgetSearch_itemSelectionChanged();

    // favorites && played widgets
    void on_listWidgetFavorites_currentTextChanged(QString);
    void on_listWidgetFavorites_itemSelectionChanged();
    void on_listWidgetFavorites_itemActivated(QListWidgetItem *);
    void on_listWidgetPlayed_currentTextChanged(QString);
    void on_listWidgetPlayed_itemSelectionChanged();
    void on_listWidgetPlayed_itemActivated(QListWidgetItem *);

    // context menus
    void on_treeWidgetGamelist_customContextMenuRequested(const QPoint &);
    void on_treeWidgetHierarchy_customContextMenuRequested(const QPoint &);
    void on_listWidgetSearch_customContextMenuRequested(const QPoint &);
    void on_listWidgetFavorites_customContextMenuRequested(const QPoint &);
    void on_listWidgetPlayed_customContextMenuRequested(const QPoint &);
    void on_treeWidgetEmulators_customContextMenuRequested(const QPoint &);
    void on_tabWidgetGamelist_customContextMenuRequested(const QPoint &);
    void on_tabWidgetGameDetail_customContextMenuRequested(const QPoint &);
    void on_tabWidgetLogsAndEmulators_customContextMenuRequested(const QPoint &);
    void on_hSplitter_customContextMenuRequested(const QPoint &);
    void on_vSplitter_customContextMenuRequested(const QPoint &);

    // splitter flip/swap callbacks
    void on_menuHorizontalSplitter_FlipOrientation_activated();
    void on_menuHorizontalSplitter_SwapLayouts_activated();
    void on_menuVerticalSplitter_FlipOrientation_activated();
    void on_menuVerticalSplitter_SwapWidgets_activated();

    // joystick functions
#if QMC2_JOYSTICK == 1
    void on_joystickAxisValueChanged(int, int);
    void on_joystickButtonValueChanged(int, bool);
    void on_joystickHatValueChanged(int, int);
    void on_joystickTrackballValueChanged(int, int, int);
    void mapJoystickFunction(QString);
#endif

    // audio player functions
    void on_actionAudioPreviousTrack_triggered(bool checked = false);
    void on_toolButtonAudioPreviousTrack_resetButton();
    void on_actionAudioNextTrack_triggered(bool checked = false);
    void on_toolButtonAudioNextTrack_resetButton();
    void on_actionAudioFastBackward_triggered(bool checked = false);
    void on_toolButtonAudioFastBackward_clicked(bool checked = false);
    void on_toolButtonAudioFastBackward_resetButton();
    void on_actionAudioFastForward_triggered(bool checked = false);
    void on_toolButtonAudioFastForward_clicked(bool checked = false);
    void on_toolButtonAudioFastForward_resetButton();
    void on_actionAudioStopTrack_triggered(bool checked = false);
    void on_actionAudioPauseTrack_triggered(bool checked = false);
    void on_actionAudioPlayTrack_triggered(bool checked = false);
    void on_toolButtonAudioAddTracks_clicked();
    void on_toolButtonAudioAddURL_clicked();
    void on_toolButtonAudioRemoveTracks_clicked();
    void on_toolButtonAudioSetupEffects_clicked();
    void on_listWidgetAudioPlaylist_itemSelectionChanged();
    void on_dialAudioVolume_valueChanged(int);
    void on_actionAudioRaiseVolume_triggered(bool checked = false);
    void on_actionAudioLowerVolume_triggered(bool checked = false);
    void audioFinished();
    void audioTick(qint64);
    void audioTotalTimeChanged(qint64);
    void audioFade(int);
    void audioMetaDataChanged();
    void audioBufferStatus(int);
    void audioScrollToCurrentItem();

    // download manager widget
    void on_checkBoxRemoveFinishedDownloads_stateChanged(int);

    // ROM state filter toggles
    void on_romStateFilterC_toggled(bool);
    void on_romStateFilterM_toggled(bool);
    void on_romStateFilterI_toggled(bool);
    void on_romStateFilterN_toggled(bool);
    void on_romStateFilterU_toggled(bool);

    // arcade mode actions and functions
    void on_actionArcadeShowFPS_toggled(bool);
    void on_actionArcadeTakeScreenshot_triggered();
    void destroyArcadeView();

    // tab widget position callbacks
    void on_menuTabWidgetGamelist_North_activated();
    void on_menuTabWidgetGamelist_South_activated();
    void on_menuTabWidgetGamelist_West_activated();
    void on_menuTabWidgetGamelist_East_activated();
    void on_menuTabWidgetGameDetail_North_activated();
    void on_menuTabWidgetGameDetail_South_activated();
    void on_menuTabWidgetGameDetail_West_activated();
    void on_menuTabWidgetGameDetail_East_activated();
    void on_menuTabWidgetGameDetail_Setup_activated();
    void on_menuTabWidgetLogsAndEmulators_North_activated();
    void on_menuTabWidgetLogsAndEmulators_South_activated();
    void on_menuTabWidgetLogsAndEmulators_West_activated();
    void on_menuTabWidgetLogsAndEmulators_East_activated();

    // memory indicator
    void on_memoryUpdateTimer_timeout();

    // other
    void on_tabWidgetGameDetail_currentChanged(int);
    void on_tabWidgetGamelist_currentChanged(int);
    void on_tabWidgetLogsAndEmulators_currentChanged(int);
    void on_tabWidgetLogsAndEmulators_updateCurrent() { on_tabWidgetLogsAndEmulators_currentChanged(tabWidgetLogsAndEmulators->currentIndex()); };
    void on_treeWidgetGamelist_itemActivated(QTreeWidgetItem *, int);
    void on_treeWidgetGamelist_itemDoubleClicked(QTreeWidgetItem *, int); 
    void on_treeWidgetGamelist_itemExpanded(QTreeWidgetItem *);
    void on_treeWidgetGamelist_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    void on_treeWidgetGamelist_itemSelectionChanged();
    void on_treeWidgetGamelist_itemSelectionChanged_delayed();
    void on_treeWidgetHierarchy_itemActivated(QTreeWidgetItem *, int);
    void on_treeWidgetHierarchy_itemDoubleClicked(QTreeWidgetItem *, int);
    void on_treeWidgetHierarchy_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    void on_treeWidgetHierarchy_itemSelectionChanged();
    void on_stackedWidgetView_currentChanged(int);
    void on_pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName = QString());
    void on_pushButtonGlobalEmulatorOptionsSelectExportFile_clicked();
    void on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName = QString());
    void on_pushButtonGlobalEmulatorOptionsSelectImportFile_clicked();
    void on_pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName = QString());
    void on_pushButtonCurrentEmulatorOptionsSelectExportFile_clicked();
    void on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName = QString());
    void on_pushButtonCurrentEmulatorOptionsSelectImportFile_clicked();
#if defined(Q_WS_X11)
    void action_embedEmulator_triggered();
    void action_embedderScanPauseKey_triggered();
    void on_tabWidgetEmbeddedEmulators_tabCloseRequested(int);
    void on_embedderOptions_toggled(bool);
    void closeEmbeddedEmuTab();
    void on_toolButtonEmbedderMaximizeToggle_toggled(bool);
    void on_embedderOptionsMenu_KillEmulator_activated();
    void on_embedderOptionsMenu_TerminateEmulator_activated();
    void on_embedderOptionsMenu_ToFavorites_activated();
    void on_embedderOptionsMenu_CopyCommand_activated();
#endif
    void action_terminateEmulator_triggered();
    void action_killEmulator_triggered();
    void action_copyEmulatorCommand_triggered();
    void action_removeFromFavorites_triggered();
    void action_clearAllFavorites_triggered();
    void action_saveFavorites_triggered();
    void action_removeFromPlayed_triggered();
    void action_clearAllPlayed_triggered();
    void action_savePlayed_triggered();
    void scrollToCurrentItem();
    void checkCurrentSearchSelection();
    void checkCurrentFavoritesSelection();
    void checkCurrentPlayedSelection();
    void log(char, QString);
    void init();
    void setupStyle(QString);
    void setupStyleSheet(QString);
    void viewFullDetail();
    void viewParentClones();
    void loadGameInfoDB();
    void loadEmuInfoDB();
#if defined(QMC2_YOUTUBE_ENABLED)
    void loadYouTubeVideoInfoMap();
#endif
    void on_treeWidgetGamelist_itemEntered(QTreeWidgetItem *, int);
    void on_treeWidgetGamelist_itemPressed(QTreeWidgetItem *, int);
    void on_treeWidgetHierarchy_itemEntered(QTreeWidgetItem *, int);
    void on_treeWidgetHierarchy_itemPressed(QTreeWidgetItem *, int);
#if defined(QMC2_EMUTYPE_MAME)
    void on_treeWidgetCategoryView_itemEntered(QTreeWidgetItem *, int);
    void on_treeWidgetCategoryView_itemPressed(QTreeWidgetItem *, int);
    void on_treeWidgetVersionView_itemEntered(QTreeWidgetItem *, int);
    void on_treeWidgetVersionView_itemPressed(QTreeWidgetItem *, int);
    void mawsLoadStarted();
    void mawsLoadFinished(bool);
    void mawsQuickLinksSetVisible(bool);
    void mawsQuickLinksMenuHidden();
    void createMawsQuickLinksMenu();
    void setupMawsQuickLinks();
    void downloadMawsQuickLink();
    void storeMawsIcon();
    void startMawsAutoDownloads();
    void viewByCategory();
    void viewByVersion();
    void on_treeWidgetCategoryView_headerSectionClicked(int);
    void on_treeWidgetCategoryView_itemActivated(QTreeWidgetItem *, int);
    void on_treeWidgetCategoryView_itemDoubleClicked(QTreeWidgetItem *, int);
    void on_treeWidgetCategoryView_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    void on_treeWidgetCategoryView_itemSelectionChanged();
    void on_treeWidgetCategoryView_customContextMenuRequested(const QPoint &);
    void on_treeWidgetVersionView_headerSectionClicked(int);
    void on_treeWidgetVersionView_itemActivated(QTreeWidgetItem *, int);
    void on_treeWidgetVersionView_itemDoubleClicked(QTreeWidgetItem *, int);
    void on_treeWidgetVersionView_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    void on_treeWidgetVersionView_itemSelectionChanged();
    void on_treeWidgetVersionView_customContextMenuRequested(const QPoint &);
#endif
    void createFifo(bool logFifoCreation = true);
    void recreateFifo();
    void processFifoData();
    void on_hSplitter_splitterMoved(int, int);
    void on_comboBoxViewSelect_currentIndexChanged(int);
    void processEvents() { qApp->processEvents(); }
    void on_treeWidgetGamelist_headerSectionClicked(int);
    void on_treeWidgetHierarchy_headerSectionClicked(int);
#if defined(QMC2_EMUTYPE_MESS)
    QString &messWikiToHtml(QString &);
#endif
    void startDownload(QNetworkReply *, QString saveAsName = QString(), QString savePath = QString());
    void on_pushButtonClearFinishedDownloads_clicked();
    void on_pushButtonReloadSelectedDownloads_clicked();
    void on_pushButtonStopSelectedDownloads_clicked();
    void on_emuSelector_currentIndexChanged(const QString &);
    void checkActivity();
    void enableContextMenuPlayActions(bool);

    // callbacks for list view header context menu requests
    void treeWidgetGamelistHeader_customContextMenuRequested(const QPoint &);
    void actionGamelistHeader_triggered();
    void treeWidgetHierarchyHeader_customContextMenuRequested(const QPoint &);
    void actionHierarchyHeader_triggered();
#if defined(QMC2_EMUTYPE_MAME)
    void treeWidgetCategoryViewHeader_customContextMenuRequested(const QPoint &);
    void actionCategoryHeader_triggered();
    void treeWidgetVersionViewHeader_customContextMenuRequested(const QPoint &);
    void actionVersionHeader_triggered();
#endif
    void comboBoxToolbarSearch_textChanged(const QString &);
    void comboBoxToolbarSearch_activated(const QString &);

  protected:
    void closeEvent(QCloseEvent *);
};

#endif
