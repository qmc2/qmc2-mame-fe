#ifndef _QMC2_MAIN_H_
#define _QMC2_MAIN_H_

#include <QApplication>
#include <QCloseEvent>
#include <QSocketNotifier>
#include <QTimer>
#include <QTime>
#include <QNetworkReply>
#include "ui_qmc2main.h"
#include "ui_options.h"
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

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
class AutoPopupToolButton : public QToolButton
{
  Q_OBJECT

  public:
    AutoPopupToolButton(QWidget *parent = 0) : QToolButton(parent) { ; }

  public slots:
    void hideMenu()
    {
      if ( menu() )
        if ( menu()->activeAction() == NULL )
          QTimer::singleShot(0, menu(), SLOT(hide()));
    }

  protected:
    void enterEvent(QEvent *e)
    {
      if ( menu() )
        QTimer::singleShot(0, this, SLOT(showMenu()));
      QToolButton::enterEvent(e);
    }
    void leaveEvent(QEvent *e)
    {
      QTimer::singleShot(1000, this, SLOT(hideMenu()));
      QToolButton::leaveEvent(e);
    }
};
#endif

class MainWindow : public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT

  public:
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
    QMenu *menuRomStatusFilter;
    QMenu *menuTabWidgetGamelist;
    QMenu *menuTabWidgetGameDetail;
    QMenu *menuTabWidgetLogsAndEmulators;
    QAction *actionRomStatusFilterC;
    QAction *actionRomStatusFilterM;
    QAction *actionRomStatusFilterI;
    QAction *actionRomStatusFilterN;
    QAction *actionRomStatusFilterU;
#if QMC2_JOYSTICK == 1
    int joyIndex;
#endif
#if QMC2_USE_PHONON_API
    Phonon::MediaObject *phononAudioPlayer;
    Phonon::AudioOutput *phononAudioOutput;
    bool audioFastForwarding;
    bool audioFastBackwarding;
    bool audioSkippingTracks;
#endif
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    AutoPopupToolButton *toolButtonMAWSQuickLinks;
    QMenu *menuMAWSQuickLinks;
#endif

    static QColor qmc2StatusColorGreen;
    static QColor qmc2StatusColorYellowGreen;
    static QColor qmc2StatusColorRed;
    static QColor qmc2StatusColorBlue;
    static QColor qmc2StatusColorGrey;

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

  public slots:
    // game menu
    void on_actionPlay_activated();
    void on_actionToFavorites_activated();
    void on_actionReload_activated();
    void on_actionExitStop_activated();
    void on_actionCheckCurrentROM_activated();
    void on_actionAnalyseCurrentROM_activated();

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
    void on_actionClearImageCache_activated();
    void on_actionClearIconCache_activated();
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    void on_actionClearMAWSCache_activated();
#endif
    void on_actionRecreateTemplateMap_activated();
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
    void on_comboBoxSearch_textChanged(QString);
    void on_comboBoxSearch_textChanged_delayed();
    void on_comboBoxSearch_activated(QString);
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

    // joystick functions
#if QMC2_JOYSTICK == 1
    void on_joystickAxisValueChanged(int, int);
    void on_joystickButtonValueChanged(int, bool);
    void on_joystickHatValueChanged(int, int);
    void on_joystickTrackballValueChanged(int, int, int);
    void mapJoystickFunction(QString);
#endif

    // audio player functions
    void on_actionAudioPreviousTrack_triggered(bool checked = FALSE);
    void on_toolButtonAudioPreviousTrack_resetButton();
    void on_actionAudioNextTrack_triggered(bool checked = FALSE);
    void on_toolButtonAudioNextTrack_resetButton();
    void on_actionAudioFastBackward_triggered(bool checked = FALSE);
    void on_toolButtonAudioFastBackward_clicked(bool checked = FALSE);
    void on_toolButtonAudioFastBackward_resetButton();
    void on_actionAudioFastForward_triggered(bool checked = FALSE);
    void on_toolButtonAudioFastForward_clicked(bool checked = FALSE);
    void on_toolButtonAudioFastForward_resetButton();
    void on_actionAudioStopTrack_triggered(bool checked = FALSE);
    void on_actionAudioPauseTrack_triggered(bool checked = FALSE);
    void on_actionAudioPlayTrack_triggered(bool checked = FALSE);
    void on_toolButtonAudioAddTracks_clicked();
    void on_toolButtonAudioRemoveTracks_clicked();
    void on_listWidgetAudioPlaylist_itemSelectionChanged();
    void on_sliderAudioVolume_valueChanged(int);
    void on_actionAudioRaiseVolume_triggered(bool checked = FALSE);
    void on_actionAudioLowerVolume_triggered(bool checked = FALSE);
    void audioFinished();
    void audioTick(qint64);
    void audioTotalTimeChanged(qint64);
    void audioFade(int);
    void audioMetaDataChanged();

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

    // other
    void on_tabWidgetGameDetail_currentChanged(int);
    void on_tabWidgetGamelist_currentChanged(int);
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
    void on_textBrowserFrontendLog_textChanged();
    void on_textBrowserEmulatorLog_textChanged();
    void on_stackedWidgetView_currentChanged(int);
    void on_pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName = QString());
    void on_pushButtonGlobalEmulatorOptionsSelectExportFile_clicked();
    void on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName = QString());
    void on_pushButtonGlobalEmulatorOptionsSelectImportFile_clicked();
    void on_pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName = QString());
    void on_pushButtonCurrentEmulatorOptionsSelectExportFile_clicked();
    void on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName = QString());
    void on_pushButtonCurrentEmulatorOptionsSelectImportFile_clicked();
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
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    void loadEmuInfoDB();
    void mawsLoadStarted();
    void mawsLoadFinished(bool);
    void mawsQuickLinksSetVisible(bool);
    void createMawsQuickLinksMenu();
    void setupMawsQuickLinks();
    void downloadMawsQuickLink();
#endif
    void createFifo(bool logFifoCreation = TRUE);
    void recreateFifo();
    void processFifoData();
    void on_hSplitter_splitterMoved(int, int);
    void on_comboBoxViewSelect_currentIndexChanged(int);
    void processEvents() { qApp->processEvents(); }
    void on_treeWidgetGamelist_headerSectionClicked(int);
    void on_treeWidgetHierarchy_headerSectionClicked(int);
    void startDownload(QNetworkReply *, QString saveAsName = QString());
    void on_pushButtonClearFinishedDownloads_clicked();
    void on_pushButtonReloadSelectedDownloads_clicked();
    void on_pushButtonStopSelectedDownloads_clicked();

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
};

#endif
