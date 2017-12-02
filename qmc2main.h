#ifndef QMC2MAIN_H
#define QMC2MAIN_H

#include <qglobal.h>
#include <QApplication>
#include <QCloseEvent>
#include <QSocketNotifier>
#include <QTimer>
#include <QTime>
#include <QNetworkReply>
#include <QWidgetAction>
#include <QProxyStyle>
#include <QMovie>
#include <QPoint>
#include <QList>
#include <QMap>

#include "settings.h"
#include "ui_qmc2main.h"
#include "ui_options.h"
#include "macros.h"
#if QMC2_USE_PHONON_API
#include "qmc2_phonon.h"
#endif
#if QMC2_MULTIMEDIA_ENABLED
#include <QMediaPlayer>
#endif
#include "imagewidget.h"
#include "rankitemwidget.h"
#include "swldbmgr.h"
#include "romstatefilter.h"
#include "machinelistviewer.h"

class ProxyStyle : public QProxyStyle
{
	public:
		int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const {
			if ( hint == QStyle::SH_ItemView_ActivateItemOnSingleClick )
				return 0;
			else
				return QProxyStyle::styleHint(hint, option, widget, returnData);
		}
};

class MainEventFilter : public QObject
{
	Q_OBJECT

	public:
		MainEventFilter(QObject *parent = 0) : QObject(parent) { ; }

	protected:
		bool eventFilter(QObject *, QEvent *);
};

class SearchBoxKeyEventFilter : public QObject
{
	Q_OBJECT

	public:
		SearchBoxKeyEventFilter(QObject *parent = 0) : QObject(parent) { ; }

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
};

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT

	public:
		bool isActiveState;
		QString defaultStyle;
		QTimer searchTimer;
		QTimer updateTimer;
		ProxyStyle *proxyStyle;
		ImagePixmap qmc2GhostImagePixmap;
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
		QMenu *menuTabWidgetMachineList;
		QMenu *menuTabWidgetMachineDetail;
		QMenu *menuTabWidgetLogsAndEmulators;
		QMenu *menuTabWidgetSoftwareDetail;
		QMenu *menuHorizontalSplitter;
		QMenu *menuVerticalSplitter;
		QMenu *menuMachineListHeader;
		QMenu *menuHierarchyHeader;
		QMenu *menuCategoryHeader;
		QAction *actionMenuMachineListHeaderCategory;
		QAction *actionMenuHierarchyHeaderCategory;
		QAction *actionMenuCategoryHeaderVersion;
		QAction *actionMenuVersionHeaderCategory;
		QMenu *menuVersionHeader;
		QAction *actionMenuMachineListHeaderVersion;
		QAction *actionMenuHierarchyHeaderVersion;
		QAction *actionCustomView;
		QWidgetAction *stateFilterAction;
		QList<QAction *> rebuildRomActions;
		QList<QAction *> contextMenuPlayActions;
		QStringList videoSnapAllowedFormatExtensions;
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
#if QMC2_MULTIMEDIA_ENABLED
		QMediaPlayer *mediaPlayer;
		bool audioFastForwarding;
		bool audioFastBackwarding;
		bool audioSkippingTracks;
		QMediaPlayer::State audioState;
#endif
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		QWidget *widgetEmbeddedEmus;
		QWidget *embedderCornerWidget;
		QHBoxLayout *embedderCornerLayout;
		QToolButton *toolButtonEmbedderMaximizeToggle;
#endif
#if defined(QMC2_OS_UNIX)
		QToolButton *toolButtonEmbedderAutoPause;
		QMenu *menuAutoPause;
#endif
		QTimer messDevCfgTimer;
		QList<int> hSplitterSizes;
		QList<int> vSplitterSizes;
		QList<int> vSplitterSizesSoftwareDetail;
		QRect desktopGeometry;
		QWidget *hSplitterWidget0;
		QWidget *vSplitterWidget0;
		QTimer activityCheckTimer;
		int retry_tabWidgetMachineDetail_currentIndex;
		QString urlSectionRegExp;
		QString foreignEmuName;
		QString foreignID;
		QString foreignDescription;
		QWidgetAction *widgetActionToolbarSearch;
		QComboBox *comboBoxToolbarSearch;
		QMenu *menuSearchOptions;
		QAction *actionNegateSearch;
		QAction *actionSearchIncludeDeviceSets;
		QAction *actionSearchIncludeBiosSets;
		QList<QAction *> criticalActions;
		QMovie *loadAnimMovie;
		QMovie *nullMovie;
		bool activityState;
		bool isCreatingSoftList;
		bool negatedMatch;
		bool launchForeignID;
		bool searchActive;
		bool stopSearch;
		bool lastPageSoftware;

		static QColor qmc2StatusColorGreen;
		static QColor qmc2StatusColorYellowGreen;
		static QColor qmc2StatusColorRed;
		static QColor qmc2StatusColorBlue;
		static QColor qmc2StatusColorGrey;
		static QList<MachineListViewer *> machineListViewers;

		QToolButton *floatToggleButtonSoftwareDetail;

		int sortCriteriaLogicalIndex();
		QPoint adjustedWidgetPosition(QPoint, QWidget *);
		QStringList &getXmlChoices(const QString &, const QString &, const QString &optionAttribute = QString(), QString *defaultChoice = 0);
		static bool qStringListLessThan(const QString &, const QString &);
		SoftwareListXmlDatabaseManager *swlDb;
		RomStateFilter *romStateFilter;
		QList<RankItemWidget *> &rankItemWidgets() { return m_rankItemWidgets; }
		MachineListViewer *attachedViewer() { return m_attachedViewer; }

		MainWindow(QWidget *parent = 0);
		~MainWindow();

		void commonWebSearch(QString, QTreeWidgetItem *);
		void processGlobalEmuConfig();
		void prepareShortcuts();
		void clearSortedItemMap() { m_sortedItemMap.clear(); }

	public slots:
		// machine menu
		void on_actionPlay_triggered(bool checked = false);
		void on_actionPlayEmbedded_triggered(bool checked = false);
		void on_actionPlayTagged_triggered(bool checked = false);
		void on_actionPlayEmbeddedTagged_triggered(bool checked = false);
		void on_actionToFavorites_triggered(bool checked = false);
		void on_actionToFavoritesTagged_triggered(bool checked = false);
		void on_actionReload_triggered(bool checked = false);
		void on_actionExitStop_triggered(bool checked = false);
		void on_actionCheckCurrentROM_triggered(bool checked = false);
		void on_actionCheckROMStateTagged_triggered(bool checked = false);
		void on_actionAnalyseCurrentROM_triggered(bool checked = false);
		void on_actionAnalyseROMTagged_triggered(bool checked = false);
		void on_actionRunRomTool_triggered(bool checked = false);
		void on_actionRunRomToolTagged_triggered(bool checked = false);
		void on_actionSetTag_triggered(bool checked = false);
		void on_actionUnsetTag_triggered(bool checked = false);
		void on_actionToggleTag_triggered(bool checked = false);
		void on_actionToggleTagCursorDown_triggered(bool checked = false);
		void on_actionToggleTagCursorUp_triggered(bool checked = false);
		void on_actionTagAll_triggered(bool checked = false);
		void on_actionUntagAll_triggered(bool checked = false);
		void on_actionInvertTags_triggered(bool checked = false);
		void on_actionTagVisible_triggered(bool checked = false);
		void on_actionUntagVisible_triggered(bool checked = false);
		void on_actionInvertVisibleTags_triggered(bool checked = false);
		void on_actionSearchDuckDuckGo_triggered(bool checked = false);
		void on_actionSearchGoogle_triggered(bool checked = false);
		void on_actionSearchWikipedia_triggered(bool checked = false);
		void on_actionSearchYandex_triggered(bool checked = false);
		void on_actionSearchInternalBrowser_triggered(bool checked = false);
		void on_actionRebuildROM_triggered(bool) { actionRebuildRom_triggered(); }
		void on_actionRebuildROMTagged_triggered(bool checked = false);
		void on_actionManualOpenInViewer_triggered(bool checked = false);
		void on_actionManualInternalViewer_triggered(bool checked = false);

		// arcade menu
		void on_actionLaunchArcade_triggered(bool checked = false);
		void on_actionArcadeSetup_triggered(bool checked = false);

		// tools menu
		void on_actionCheckROMs_triggered(bool checked = false);
		void on_actionCheckSamples_triggered(bool checked = false);
		void on_actionCheckImagesAndIcons_triggered(bool checked = false);
		void on_actionSystemROMAlyzer_triggered(bool checked = false);
		void on_actionSoftwareROMAlyzer_triggered(bool checked = false);
		void on_actionExportROMStatus_triggered(bool checked = false);
		void on_actionDemoMode_triggered(bool checked = false);
		void on_actionNewBrowserWindow_triggered(bool checked = false);
		void on_actionNewPdfViewer_triggered(bool checked = false);
		void on_actionNewFilteredView_triggered(bool checked = false);
		void on_actionClearImageCache_triggered(bool checked = false);
		void on_actionClearIconCache_triggered(bool checked = false);
		void on_actionClearProjectMESSCache_triggered(bool checked = false);
#if defined(QMC2_YOUTUBE_ENABLED)
		void on_actionClearYouTubeCache_triggered(bool checked = false);
#endif
		void on_actionRecreateTemplateMap_triggered(bool checked = false);
		void on_actionCheckTemplateMap_triggered(bool checked = false);
		void on_actionClearROMStateCache_triggered(bool checked = false);
		void on_actionClearMachineListCache_triggered(bool checked = false);
		void on_actionClearXMLCache_triggered(bool checked = false);
		void on_actionClearSlotInfoCache_triggered(bool checked = false);
		void on_actionClearSoftwareListCache_triggered(bool checked = false);
		void on_actionClearAllEmulatorCaches_triggered(bool complete = false);
		void on_actionOptions_triggered(bool checked = false);
		void on_actionRelaunchSetupWizard_triggered(bool checked = false);

		// display menu
		void on_actionFullscreenToggle_triggered(bool checked = false);

		// help menu
		void on_actionDocumentation_triggered(bool checked = false);
		void on_actionAbout_triggered(bool checked = false);
		void on_actionHomepage_triggered(bool checked = false);
		void on_actionWiki_triggered(bool checked = false);
		void on_actionForum_triggered(bool checked = false);
		void on_actionBugTracker_triggered(bool checked = false);
		void on_actionAboutQt_triggered(bool checked = false);

		// search widget
		void on_comboBoxSearch_editTextChanged(const QString &);
		void comboBoxSearch_editTextChanged_delayed();
		void on_comboBoxSearch_activated(const QString &);
		void on_listWidgetSearch_itemActivated(QListWidgetItem *);
		void on_listWidgetSearch_currentItemChanged(QListWidgetItem *, QListWidgetItem *);
		void on_listWidgetSearch_itemPressed(QListWidgetItem *);
		void on_listWidgetSearch_itemSelectionChanged();

		// favorites & played widgets
		void on_listWidgetFavorites_itemSelectionChanged();
		void on_listWidgetFavorites_itemActivated(QListWidgetItem *);
		void on_listWidgetPlayed_itemSelectionChanged();
		void on_listWidgetPlayed_itemActivated(QListWidgetItem *);

		// context menus
		void on_treeWidgetMachineList_customContextMenuRequested(const QPoint &);
		void on_treeWidgetHierarchy_customContextMenuRequested(const QPoint &);
		void on_listWidgetSearch_customContextMenuRequested(const QPoint &);
		void on_listWidgetFavorites_customContextMenuRequested(const QPoint &);
		void on_listWidgetPlayed_customContextMenuRequested(const QPoint &);
		void on_treeWidgetEmulators_customContextMenuRequested(const QPoint &);
		void on_tabWidgetMachineList_customContextMenuRequested(const QPoint &);
		void tabWidgetMachineList_actionSwitchTab_triggered(bool checked = false);
		void on_tabWidgetMachineDetail_customContextMenuRequested(const QPoint &);
		void tabWidgetMachineDetail_actionSwitchTab_triggered(bool checked = false);
		void on_tabWidgetLogsAndEmulators_customContextMenuRequested(const QPoint &);
		void tabWidgetLogsAndEmulators_actionSwitchTab_triggered(bool checked = false);
		void on_tabWidgetSoftwareDetail_customContextMenuRequested(const QPoint &p);
		void tabWidgetSoftwareDetail_actionSwitchTab_triggered(bool checked = false);

		// joystick functions
#if QMC2_JOYSTICK == 1
		void joystickAxisValueChanged(int, int);
		void joystickButtonValueChanged(int, bool);
		void joystickHatValueChanged(int, int);
		void joystickTrackballValueChanged(int, int, int);
		void mapJoystickFunction(QString);
#endif

		// audio player functions
		void on_actionAudioPreviousTrack_triggered(bool checked = false);
		void toolButtonAudioPreviousTrack_resetButton();
		void on_actionAudioNextTrack_triggered(bool checked = false);
		void toolButtonAudioNextTrack_resetButton();
		void on_actionAudioFastBackward_triggered(bool checked = false);
		void on_toolButtonAudioFastBackward_clicked(bool checked = false);
		void toolButtonAudioFastBackward_resetButton();
		void on_actionAudioFastForward_triggered(bool checked = false);
		void on_toolButtonAudioFastForward_clicked(bool checked = false);
		void toolButtonAudioFastForward_resetButton();
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
#if QMC2_MULTIMEDIA_ENABLED
		void audioStateChanged(QMediaPlayer::MediaStatus);
#endif
		void audioTick(qint64);
		void audioTotalTimeChanged(qint64);
		void audioFade(int);
		void audioMetaDataChanged();
		void audioBufferStatus(int);
		void audioScrollToCurrentItem();

		// download manager widget
		void on_checkBoxRemoveFinishedDownloads_stateChanged(int);

		// tab widget position callbacks
		void menuTabWidgetMachineList_North_activated();
		void menuTabWidgetMachineList_South_activated();
		void menuTabWidgetMachineList_West_activated();
		void menuTabWidgetMachineList_East_activated();
		void menuTabWidgetMachineList_Setup_activated();
		void menuTabWidgetMachineDetail_North_activated();
		void menuTabWidgetMachineDetail_South_activated();
		void menuTabWidgetMachineDetail_West_activated();
		void menuTabWidgetMachineDetail_East_activated();
		void menuTabWidgetMachineDetail_Setup_activated();
		void menuTabWidgetLogsAndEmulators_North_activated();
		void menuTabWidgetLogsAndEmulators_South_activated();
		void menuTabWidgetLogsAndEmulators_West_activated();
		void menuTabWidgetLogsAndEmulators_East_activated();
		void menuTabWidgetLogsAndEmulators_Setup_activated();
		void menuTabWidgetSoftwareDetail_North_activated();
		void menuTabWidgetSoftwareDetail_South_activated();
		void menuTabWidgetSoftwareDetail_West_activated();
		void menuTabWidgetSoftwareDetail_East_activated();
		void menuTabWidgetSoftwareDetail_Setup_activated();

		// other
		void on_tabWidgetMachineDetail_currentChanged(int);
		void retry_tabWidgetMachineDetail_currentChanged() { on_tabWidgetMachineDetail_currentChanged(retry_tabWidgetMachineDetail_currentIndex); };
		void tabWidgetMachineList_tabMoved(int, int);
		void tabWidgetMachineDetail_tabMoved(int, int);
		void tabWidgetLogsAndEmulators_tabMoved(int, int);
		void tabWidgetSoftwareDetail_tabMoved(int, int);
		void on_tabWidgetMachineList_currentChanged(int);
		void on_tabWidgetLogsAndEmulators_currentChanged(int);
		void tabWidgetLogsAndEmulators_updateCurrent() { on_tabWidgetLogsAndEmulators_currentChanged(tabWidgetLogsAndEmulators->currentIndex()); };
		void on_tabWidgetSoftwareDetail_currentChanged(int);
		void tabWidgetSoftwareDetail_updateCurrent() { on_tabWidgetSoftwareDetail_currentChanged(tabWidgetSoftwareDetail->currentIndex()); };
		void on_treeWidgetMachineList_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetMachineList_itemDoubleClicked(QTreeWidgetItem *, int); 
		void on_treeWidgetMachineList_itemExpanded(QTreeWidgetItem *);
		void on_treeWidgetMachineList_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
		void on_treeWidgetMachineList_itemSelectionChanged();
		void treeWidgetMachineList_itemSelectionChanged_delayed();
		void on_treeWidgetHierarchy_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetHierarchy_itemDoubleClicked(QTreeWidgetItem *, int);
		void on_treeWidgetHierarchy_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
		void on_treeWidgetHierarchy_itemSelectionChanged();
		void on_stackedWidgetView_currentChanged(int);
		void pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName = QString());
		void pushButtonGlobalEmulatorOptionsSelectExportFile_clicked();
		void pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName = QString());
		void pushButtonGlobalEmulatorOptionsSelectImportFile_clicked();
		void pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName = QString());
		void pushButtonCurrentEmulatorOptionsSelectExportFile_clicked();
		void pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName = QString());
		void pushButtonCurrentEmulatorOptionsSelectImportFile_clicked();
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		void action_embedEmulator_triggered();
		void on_tabWidgetEmbeddedEmulators_tabCloseRequested(int);
		void embedderOptions_toggled(bool);
		void closeEmbeddedEmuTab();
		void toolButtonEmbedderMaximizeToggle_toggled(bool);
		void embedderOptionsMenu_KillEmulator_activated();
		void embedderOptionsMenu_ToFavorites_activated();
		void embedderOptionsMenu_CopyCommand_activated();
#if defined(QMC2_OS_UNIX)
		void action_embedderScanPauseKey_triggered();
#endif
#endif
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
		void log(int, const QString &);
		void logFE(const QString &message) { log(QMC2_LOG_FRONTEND, message); }
		void logEMU(const QString &message) { log(QMC2_LOG_EMULATOR, message); }
		void logScrollToEnd(int);
		void init();
		void setupStyle(QString);
		void setupStyleSheet(QString);
		void setupPalette(QString);
		void viewFullDetail();
		void viewParentClones();
		void loadMachineInfoDB();
		void loadEmuInfoDB();
		void loadSoftwareInfoDB();
#if defined(QMC2_YOUTUBE_ENABLED)
		void loadYouTubeVideoInfoMap();
#endif
		void on_treeWidgetMachineList_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetMachineList_itemPressed(QTreeWidgetItem *, int);
		void on_treeWidgetHierarchy_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetHierarchy_itemPressed(QTreeWidgetItem *, int);
		void on_treeWidgetCategoryView_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetCategoryView_itemPressed(QTreeWidgetItem *, int);
		void treeWidgetCategoryView_headerSectionClicked(int);
		void on_treeWidgetCategoryView_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetCategoryView_itemDoubleClicked(QTreeWidgetItem *, int);
		void on_treeWidgetCategoryView_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
		void on_treeWidgetCategoryView_itemSelectionChanged();
		void on_treeWidgetCategoryView_customContextMenuRequested(const QPoint &);
		void viewByCategory();
		void on_treeWidgetVersionView_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetVersionView_itemPressed(QTreeWidgetItem *, int);
		void treeWidgetVersionView_headerSectionClicked(int);
		void on_treeWidgetVersionView_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetVersionView_itemDoubleClicked(QTreeWidgetItem *, int);
		void on_treeWidgetVersionView_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
		void on_treeWidgetVersionView_itemSelectionChanged();
		void on_treeWidgetVersionView_customContextMenuRequested(const QPoint &);
		void viewByVersion();
		void actionCustomView_triggered();
		void projectMessSystemLoadStarted();
		void projectMessSystemLoadFinished(bool);
		void projectMessLoadStarted();
		void projectMessLoadFinished(bool);
		void processOutputNotifier(int, const QString &, const QString &);
		void on_hSplitter_splitterMoved(int, int);
		void on_vSplitter_splitterMoved(int, int);
		void on_comboBoxViewSelect_currentIndexChanged(int);
		void processEvents() { qApp->processEvents(); }
		void treeWidgetMachineList_headerSectionClicked(int);
		void treeWidgetHierarchy_headerSectionClicked(int);
		QString &messWikiToHtml(QString &);
		void startDownload(QWidget *, QNetworkReply *, QString saveAsName = QString(), QString savePath = QString());
		void on_pushButtonClearFinishedDownloads_clicked();
		void on_pushButtonReloadSelectedDownloads_clicked();
		void on_pushButtonStopSelectedDownloads_clicked();
		void emuSelector_currentIndexChanged(const QString &);
		void checkActivity();
		void enableContextMenuPlayActions(bool);
		void softwareLoadInterrupted();
		void checkRomPath();
		void negateSearchTriggered(bool);
		void searchIncludeBiosSetsTriggered(bool);
		void searchIncludeDeviceSetsTriggered(bool);
		void updateTabWidgets();
		void viewPdf(QString filePath = QString());
		void viewHtml(QString filePath = QString());
		void showLoadAnim(QString text, bool enable = true);
		void hideLoadAnim() { showLoadAnim(QString(), false); }
		void actionRebuildRom_triggered(bool checked = false);
		void update_rebuildRomActions_visibility();

		// float toggle button callbacks for 'special' widgets
		void floatToggleButtonSoftwareDetail_toggled(bool);
		void adjustSplitter(QSplitter *, QTabWidget *, QList<int> &, bool);
		void stackedWidgetSpecial_setCurrentIndex(int);

		// callbacks for list view header context menu requests
		void treeWidgetMachineListHeader_customContextMenuRequested(const QPoint &);
		void actionMachineListHeader_triggered();
		void treeWidgetHierarchyHeader_customContextMenuRequested(const QPoint &);
		void actionHierarchyHeader_triggered();
		void treeWidgetCategoryViewHeader_customContextMenuRequested(const QPoint &);
		void actionCategoryHeader_triggered();
		void treeWidgetVersionViewHeader_customContextMenuRequested(const QPoint &);
		void actionVersionHeader_triggered();
		void comboBoxToolbarSearch_editTextChanged(const QString &);
		void comboBoxToolbarSearch_activated(const QString &);
		void comboBoxToolbarSearch_activated() { comboBoxToolbarSearch_activated(comboBoxToolbarSearch->currentText()); }

		void signalStyleSetupRequested(QString style) { emit styleSetupRequested(style); }
		void signalStyleSheetSetupRequested(QString styleSheet) { emit styleSheetSetupRequested(styleSheet); }
		void signalPaletteSetupRequested(QString style) { emit paletteSetupRequested(style); }

		// callbacks for dynamically updated list contents (ranks and comments)
		void treeWidgetMachineList_verticalScrollChanged(int value = -1);
		void treeWidgetMachineList_updateRanks();
		void treeWidgetHierarchy_verticalScrollChanged(int value = -1);
		void treeWidgetHierarchy_updateRanks();
		void on_treeWidgetHierarchy_itemExpanded(QTreeWidgetItem *);
		void treeWidgetCategoryView_verticalScrollChanged(int value = -1);
		void treeWidgetCategoryView_updateRanks();
		void on_treeWidgetCategoryView_itemExpanded(QTreeWidgetItem *);
		void treeWidgetVersionView_verticalScrollChanged(int value = -1);
		void treeWidgetVersionView_updateRanks();
		void on_treeWidgetVersionView_itemExpanded(QTreeWidgetItem *);
		void updateUserData();
		void on_actionIncreaseRank_triggered(bool);
		void on_actionDecreaseRank_triggered(bool);
		void on_actionRankImageGradient_triggered(bool checked = false);
		void on_actionRankImageFlat_triggered(bool checked = false);
		void on_actionRankImagePlain_triggered(bool checked = false);
		void on_actionRankImageColor_triggered(bool);
		void on_actionSetRank0_triggered(bool);
		void on_actionSetRank1_triggered(bool);
		void on_actionSetRank2_triggered(bool);
		void on_actionSetRank3_triggered(bool);
		void on_actionSetRank4_triggered(bool);
		void on_actionSetRank5_triggered(bool);
		void on_actionTaggedIncreaseRank_triggered(bool);
		void on_actionTaggedDecreaseRank_triggered(bool);
		void on_actionTaggedSetRank0_triggered(bool);
		void on_actionTaggedSetRank1_triggered(bool);
		void on_actionTaggedSetRank2_triggered(bool);
		void on_actionTaggedSetRank3_triggered(bool);
		void on_actionTaggedSetRank4_triggered(bool);
		void on_actionTaggedSetRank5_triggered(bool);
		void on_actionLockRanks_triggered(bool);
		void menuRank_enableActions(bool);
		void menuRank_aboutToShow();
		RankItemWidget *getCurrentRankItemWidget();
		QList<RankItemWidget *> *getTaggedRankItemWidgets();

		// foreign IDs tree-widget
		void on_treeWidgetForeignIDs_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetForeignIDs_itemDoubleClicked(QTreeWidgetItem *, int); 
		void on_treeWidgetForeignIDs_customContextMenuRequested(const QPoint &);

		// selection and tag changes done by filtering machine list viewers
		void machineListViewer_selectionChanged(const QString &id);
		void machineListViewer_tagChanged(const QString &id, bool tagged);

		// related to system manuals
		void checkSystemManualAvailability();

		// Qt bug workarounds
		void detailTabBarUpdate(int currentIndex);

		// related to attached views
		void showAttachedView(const QString &);
		void attachedViewAction_triggered(bool);

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void resizeEvent(QResizeEvent *);

	signals:
		void styleSetupRequested(QString);
		void styleSheetSetupRequested(QString);
		void paletteSetupRequested(QString);

		// inform about selection changes done by ourself
		void selectionChanged(const QString &);
		void updateDetailTabBar(int);

	private:
		QTimer m_mlRankUpdateTimer;
		QTimer m_hlRankUpdateTimer;
		QTimer m_clRankUpdateTimer;
		QTimer m_vlRankUpdateTimer;
		bool m_ignoreSelectionChange;
		bool m_ignoreDetailTabChange;
#if defined(QMC2_YOUTUBE_ENABLED)
		bool m_videoInfoMapLoaded;
#endif
		bool m_focusSearchResults;
		MachineListViewer *m_lastMlvSender;
		QList<RankItemWidget *> m_rankItemWidgets;
		MachineListViewer *m_attachedViewer;
		SearchBoxKeyEventFilter *m_searchBoxKeyEventFilter;
		QMap<QString, QTreeWidgetItem *> m_sortedItemMap;
};

#endif
