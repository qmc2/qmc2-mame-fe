#include <QtGui>
#include <QTest>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QCache>
#include <QInputDialog>
#include <QWidgetAction>
#include <QLocale>
#include <QPainterPath>
#include <QAbstractButton>
#include <QHash>

#include "softwarelist.h"
#include "gamelist.h"
#include "qmc2main.h"
#include "options.h"
#include "iconlineedit.h"
#include "romalyzer.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;
extern Gamelist *qmc2Gamelist;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern bool qmc2UseSoftwareSnapFile;
extern SoftwareList *qmc2SoftwareList;
extern SoftwareSnap *qmc2SoftwareSnap;
extern SoftwareSnapshot *qmc2SoftwareSnapshot;
extern int qmc2SoftwareSnapPosition;
extern bool qmc2IgnoreItemActivation;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ShowGameName;
extern int qmc2UpdateDelay;
extern int qmc2DefaultLaunchMode;
extern bool qmc2StopParser;
extern bool qmc2CriticalSection;
extern bool qmc2UseDefaultEmulator;
extern bool qmc2TemplateCheck;
extern bool qmc2VerifyActive;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;
extern QHash<QString, QPair<QString, QAction *> > qmc2ShortcutHash;
extern QHash<QString, QString> qmc2CustomShortcutHash;
extern ROMAlyzer *qmc2SoftwareROMAlyzer;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;

QHash<QString, QStringList> systemSoftwareListHash;
QHash<QString, QStringList> systemSoftwareFilterHash;
QHash<QString, QHash<QString, char> > softwareListStateHash;
QHash<QString, SoftwareItem *> softwareItemHash;
QHash<QString, SoftwareItem *> softwareHierarchyItemHash;
QHash<QString, QString> softwareParentHash;
bool SoftwareList::isInitialLoad = true;
bool SoftwareList::swlSupported = true;
QString SoftwareList::swStatesLastLine;

#define swlDb	qmc2MainWindow->swlDb

SoftwareList::SoftwareList(QString sysName, QWidget *parent)
	: QWidget(parent)
{
	if ( !swlDb ) {
		swlDb = new SoftwareListXmlDatabaseManager(qmc2MainWindow);
		swlDb->setSyncMode(QMC2_DB_SYNC_MODE_OFF);
		swlDb->setJournalMode(QMC2_DB_JOURNAL_MODE_MEMORY);
	}

	setupUi(this);

	progressBar->setVisible(false);
	progressBarSearch->setVisible(false);

	// hide snapname device selection initially
	comboBoxSnapnameDevice->hide();

	if ( !qmc2SoftwareSnap )
		qmc2SoftwareSnap = new SoftwareSnap(0);

	qmc2SoftwareSnap->hide();
	snapTimer.setSingleShot(true);
	connect(&snapTimer, SIGNAL(timeout()), qmc2SoftwareSnap, SLOT(loadSnapshot()));

	systemName = sysName;
	loadProc = verifyProc = NULL;
	exporter = NULL;
	currentItem = enteredItem = NULL;
	snapForced = autoSelectSearchItem = interruptLoad = isLoading = isReady = fullyLoaded = updatingMountDevices = negatedMatch = false;
	validData = autoMounted = true;
	searchActive = stopSearch = false;
	uncommittedSwlDbRows = 0;

#if defined(QMC2_EMUTYPE_MAME)
	comboBoxDeviceConfiguration->setVisible(false);
	QString altText = tr("Add the currently selected software to the favorites list");
	toolButtonAddToFavorites->setToolTip(altText); toolButtonAddToFavorites->setStatusTip(altText);
	treeWidgetFavoriteSoftware->setColumnCount(QMC2_SWLIST_COLUMN_DEVICECFG);
#elif defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
	horizontalLayout->removeItem(horizontalSpacer);
#endif

	oldMin = 0;
	oldMax = 1;
	oldFmt = qmc2MainWindow->progressBarGamelist->format();

	comboBoxSearch->setLineEdit(new IconLineEdit(QIcon(QString::fromUtf8(":/data/img/find.png")), QMC2_ALIGN_LEFT, comboBoxSearch));
	comboBoxSearch->lineEdit()->setPlaceholderText(tr("Enter search string"));

	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeMiddle = iconSize + QSize(2, 2);
	treeWidgetKnownSoftware->setIconSize(iconSizeMiddle);
	treeWidgetKnownSoftwareTree->setIconSize(iconSizeMiddle);
	treeWidgetFavoriteSoftware->setIconSize(iconSizeMiddle);
	treeWidgetSearchResults->setIconSize(iconSizeMiddle);
	toolButtonAddToFavorites->setIconSize(iconSize);
	toolButtonRemoveFromFavorites->setIconSize(iconSize);
	toolButtonFavoritesOptions->setIconSize(iconSize);
	toolButtonPlay->setIconSize(iconSize);
	toolButtonReload->setIconSize(iconSize);
	toolButtonExport->setIconSize(iconSize);
	toolButtonToggleSoftwareInfo->setIconSize(iconSize);
	toolButtonCompatFilterToggle->setIconSize(iconSize);
	toolButtonToggleSnapnameAdjustment->setIconSize(iconSize);
	toolButtonSoftwareStates->setIconSize(iconSize);
	toolButtonAnalyzeSoftware->setIconSize(iconSize);
	toolButtonRebuildSoftware->setIconSize(iconSize);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	toolButtonPlayEmbedded->setIconSize(iconSize);
#else
	toolButtonPlayEmbedded->setVisible(false);
#endif
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/view_detail.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_FAVORITES_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/favorites.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_SEARCH_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/hint.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

	toolBoxSoftwareList->setEnabled(false);
	toolButtonAddToFavorites->setEnabled(false);
	toolButtonRemoveFromFavorites->setEnabled(false);
	toolButtonFavoritesOptions->setEnabled(false);
	toolButtonPlay->setEnabled(false);
	toolButtonPlayEmbedded->setEnabled(false);
	toolButtonExport->setEnabled(false);
	toolButtonToggleSoftwareInfo->setEnabled(false);
	toolButtonCompatFilterToggle->setEnabled(false);
	toolButtonToggleSnapnameAdjustment->setEnabled(false);
	toolButtonSoftwareStates->setEnabled(false);
	toolButtonAnalyzeSoftware->setEnabled(false);
	toolButtonRebuildSoftware->setEnabled(false);
	comboBoxDeviceConfiguration->setEnabled(false);

	// software list context menu
	softwareListMenu = new QMenu(this);
	QString s = tr("Play selected software");
	QAction *action = softwareListMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playActivated()));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Play selected software (embedded)");
	action = softwareListMenu->addAction(tr("Play &embedded"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playEmbeddedActivated()));
#endif
	softwareListMenu->addSeparator();
	s = tr("Add to favorite software list");
	actionAddToFavorites = softwareListMenu->addAction(tr("&Add to favorites"));
	actionAddToFavorites->setToolTip(s); actionAddToFavorites->setStatusTip(s);
	actionAddToFavorites->setIcon(QIcon(QString::fromUtf8(":/data/img/add_to_favorites.png")));
	connect(actionAddToFavorites, SIGNAL(triggered()), this, SLOT(addToFavorites()));
	s = tr("Remove from favorite software list");
	actionRemoveFromFavorites = softwareListMenu->addAction(tr("&Remove from favorites"));
	actionRemoveFromFavorites->setToolTip(s); actionRemoveFromFavorites->setStatusTip(s);
	actionRemoveFromFavorites->setIcon(QIcon(QString::fromUtf8(":/data/img/remove_from_favorites.png")));
	connect(actionRemoveFromFavorites, SIGNAL(triggered()), this, SLOT(removeFromFavorites()));
	softwareListMenu->addSeparator();

	// analyze sub-menu
	QMenu *analyzeMenu = new QMenu(this);
	analyzeMenuAction = softwareListMenu->addMenu(analyzeMenu);
	analyzeMenuAction->setText(tr("Analy&ze"));
	analyzeMenuAction->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer_sw.png")));
	s = tr("Analyze the currently selected software with the ROMAlyzer");
	actionAnalyzeSoftware = analyzeMenu->addAction(tr("Current &software..."));
	actionAnalyzeSoftware->setToolTip(s); actionAnalyzeSoftware->setStatusTip(s);
	actionAnalyzeSoftware->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer_sw.png")));
	connect(actionAnalyzeSoftware, SIGNAL(triggered()), this, SLOT(analyzeSoftware()));
	s = tr("Analyze the currently selected software's list with the ROMAlyzer");
	actionAnalyzeSoftwareList = analyzeMenu->addAction(tr("Current software-lis&t..."));
	actionAnalyzeSoftwareList->setToolTip(s); actionAnalyzeSoftwareList->setStatusTip(s);
	actionAnalyzeSoftwareList->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer_sw.png")));
	connect(actionAnalyzeSoftwareList, SIGNAL(triggered()), this, SLOT(analyzeSoftwareList()));
	s = tr("Analyze all relevant software-lists for the current system with the ROMAlyzer");
	actionAnalyzeSoftwareLists = analyzeMenu->addAction(tr("All supported software-&lists..."));
	actionAnalyzeSoftwareLists->setToolTip(s); actionAnalyzeSoftwareLists->setStatusTip(s);
	actionAnalyzeSoftwareLists->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer_sw.png")));
	connect(actionAnalyzeSoftwareLists, SIGNAL(triggered()), this, SLOT(analyzeSoftwareLists()));
	toolButtonAnalyzeSoftware->setMenu(analyzeMenu);
	connect(analyzeMenu, SIGNAL(aboutToShow()), this, SLOT(analyzeSoftwareMenu_aboutToShow()));

	// rebuild sub-menu
	QMenu *rebuildMenu = new QMenu(this);
	rebuildMenuAction = softwareListMenu->addMenu(rebuildMenu);
	rebuildMenuAction->setText(tr("&Rebuild"));
	rebuildMenuAction->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	s = tr("Rebuild the currently selected software with the ROMAlyzer");
	actionRebuildSoftware = rebuildMenu->addAction(tr("Current &software..."));
	actionRebuildSoftware->setToolTip(s); actionRebuildSoftware->setStatusTip(s);
	actionRebuildSoftware->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	connect(actionRebuildSoftware, SIGNAL(triggered()), this, SLOT(rebuildSoftware()));
	s = tr("Rebuild the currently selected software's list with the ROMAlyzer");
	actionRebuildSoftwareList = rebuildMenu->addAction(tr("Current software-lis&t..."));
	actionRebuildSoftwareList->setToolTip(s); actionRebuildSoftwareList->setStatusTip(s);
	actionRebuildSoftwareList->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	connect(actionRebuildSoftwareList, SIGNAL(triggered()), this, SLOT(rebuildSoftwareList()));
	s = tr("Rebuild all relevant software-lists for the current system with the ROMAlyzer");
	actionRebuildSoftwareLists = rebuildMenu->addAction(tr("All supported software-&lists..."));
	actionRebuildSoftwareLists->setToolTip(s); actionRebuildSoftwareLists->setStatusTip(s);
	actionRebuildSoftwareLists->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	connect(actionRebuildSoftwareLists, SIGNAL(triggered()), this, SLOT(rebuildSoftwareLists()));
	toolButtonRebuildSoftware->setMenu(rebuildMenu);
	connect(rebuildMenu, SIGNAL(aboutToShow()), this, SLOT(rebuildSoftwareMenu_aboutToShow()));

	updateRebuildSoftwareMenuVisibility();

	// clear selection item
	softwareListMenu->addSeparator();
	s = tr("Clear software selection");
	actionClearSelection = softwareListMenu->addAction(tr("&Clear selection"));
	actionClearSelection->setToolTip(s); actionClearSelection->setStatusTip(s);
	actionClearSelection->setIcon(QIcon(QString::fromUtf8(":/data/img/broom.png")));
	connect(actionClearSelection, SIGNAL(triggered()), this, SLOT(clearSoftwareSelection()));

	// favorites options menu
	favoritesOptionsMenu = new QMenu(this);
	s = tr("Load favorites from a file...");
	action = favoritesOptionsMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(loadFavoritesFromFile()));
	s = tr("Save favorites to a file...");
	action = favoritesOptionsMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
	actionSaveFavoritesToFile = action;
	connect(action, SIGNAL(triggered()), this, SLOT(saveFavoritesToFile()));
	toolButtonFavoritesOptions->setMenu(favoritesOptionsMenu);

	// snapname adjustment menu
	menuSnapnameAdjustment = new QMenu(this);
	s = tr("Adjust pattern...");
	action = menuSnapnameAdjustment->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/configure.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(adjustSnapnamePattern()));
	toolButtonToggleSnapnameAdjustment->setMenu(menuSnapnameAdjustment);

	// software-states menu
	menuSoftwareStates = new QMenu(this);
	s = tr("Check software-states");
	action = menuSoftwareStates->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/update.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(checkSoftwareStates()));
	actionCheckSoftwareStates = action;
	actionCheckSoftwareStates->setShortcut(QKeySequence(qmc2CustomShortcutHash["F10"]));
	actionCheckSoftwareStates->setShortcutContext(Qt::ApplicationShortcut);
	qmc2ShortcutHash["F10"].second = actionCheckSoftwareStates;
	menuSoftwareStates->addSeparator();
	stateFilter = new SoftwareStateFilter(menuSoftwareStates);
	QWidgetAction *stateFilterAction = new QWidgetAction(menuSoftwareStates);
	stateFilterAction->setDefaultWidget(stateFilter);
	menuSoftwareStates->addAction(stateFilterAction);

	// search options menu
	menuSearchOptions = new QMenu(this);
	s = tr("Negate search");
	actionNegateSearch = menuSearchOptions->addAction(s);
	actionNegateSearch->setToolTip(s); actionNegateSearch->setStatusTip(s);
	actionNegateSearch->setIcon(QIcon(QString::fromUtf8(":/data/img/find_negate.png")));
	actionNegateSearch->setCheckable(true);
	bool negated = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/NegateSearch", false).toBool();
	actionNegateSearch->setChecked(negated);
	negateSearchTriggered(negated);
	connect(actionNegateSearch, SIGNAL(triggered(bool)), this, SLOT(negateSearchTriggered(bool)));
	IconLineEdit *ile = ((IconLineEdit *)comboBoxSearch->lineEdit());
	connect(ile, SIGNAL(returnPressed()), this, SLOT(comboBoxSearch_editTextChanged_delayed()));
	ile->button()->setPopupMode(QToolButton::InstantPopup);
	ile->button()->setMenu(menuSearchOptions);

	// restore widget states
	treeWidgetKnownSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState").toByteArray());
	treeWidgetKnownSoftwareTree->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareTreeHeaderState").toByteArray());
	treeWidgetFavoriteSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState").toByteArray());
	treeWidgetSearchResults->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState").toByteArray());
	toolBoxSoftwareList->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex").toInt());
	toolButtonToggleSoftwareInfo->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareInfo", false).toBool());
	toolButtonCompatFilterToggle->blockSignals(true);
	toolButtonCompatFilterToggle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/CompatFilter", true).toBool());
	toolButtonCompatFilterToggle->blockSignals(false);
	toolButtonToggleSnapnameAdjustment->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AdjustSnapname", false).toBool());
	toolButtonSoftwareStates->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareStates", false).toBool());
	if ( toolButtonSoftwareStates->isChecked() )
		toolButtonSoftwareStates->setMenu(menuSoftwareStates);

	connect(treeWidgetKnownSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetKnownSoftware_headerSectionClicked(int)));
	connect(treeWidgetKnownSoftwareTree->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetKnownSoftwareTree_headerSectionClicked(int)));
	connect(treeWidgetFavoriteSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetFavoriteSoftware_headerSectionClicked(int)));
	connect(treeWidgetSearchResults->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetSearchResults_headerSectionClicked(int)));
	connect(&searchTimer, SIGNAL(timeout()), this, SLOT(comboBoxSearch_editTextChanged_delayed()));

	QHeaderView *header;

	// header context menus
	menuKnownSoftwareHeader = new QMenu(this);
	header = treeWidgetKnownSoftware->header();
	action = menuKnownSoftwareHeader->addAction(tr("Title"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_TITLE);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_TITLE));
	action = menuKnownSoftwareHeader->addAction(tr("Name"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_NAME);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_NAME));
	action = menuKnownSoftwareHeader->addAction(tr("Publisher"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PUBLISHER);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_PUBLISHER));
	action = menuKnownSoftwareHeader->addAction(tr("Year"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_YEAR));
	action = menuKnownSoftwareHeader->addAction(tr("Part"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PART);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_PART));
	action = menuKnownSoftwareHeader->addAction(tr("Interface"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_INTERFACE);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_INTERFACE));
	action = menuKnownSoftwareHeader->addAction(tr("List"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_LIST);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
	action = menuKnownSoftwareHeader->addAction(tr("Supported"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_SUPPORTED);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_SUPPORTED));
	menuKnownSoftwareHeader->addSeparator();
	action = menuKnownSoftwareHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setData(QMC2_SWLIST_RESET);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &)));
	header = treeWidgetKnownSoftwareTree->header();
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetKnownSoftwareTreeHeader_customContextMenuRequested(const QPoint &)));

	menuFavoriteSoftwareHeader = new QMenu(this);
	header = treeWidgetFavoriteSoftware->header();
	action = menuFavoriteSoftwareHeader->addAction(tr("Title"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_TITLE);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_TITLE));
	action = menuFavoriteSoftwareHeader->addAction(tr("Name"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_NAME);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_NAME));
	action = menuFavoriteSoftwareHeader->addAction(tr("Publisher"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PUBLISHER);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_PUBLISHER));
	action = menuFavoriteSoftwareHeader->addAction(tr("Year"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_YEAR));
	action = menuFavoriteSoftwareHeader->addAction(tr("Part"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PART);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_PART));
	action = menuFavoriteSoftwareHeader->addAction(tr("Interface"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_INTERFACE);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_INTERFACE));
	action = menuFavoriteSoftwareHeader->addAction(tr("List"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_LIST);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
	action = menuFavoriteSoftwareHeader->addAction(tr("Supported"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_SUPPORTED);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
#if defined(QMC2_EMUTYPE_MESS) | defined(QMC2_EMUTYPE_UME)
	action = menuFavoriteSoftwareHeader->addAction(tr("Device configuration"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_DEVICECFG);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_DEVICECFG));
#endif
	menuFavoriteSoftwareHeader->addSeparator();
	action = menuFavoriteSoftwareHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setData(QMC2_SWLIST_RESET);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &)));

	menuSearchResultsHeader = new QMenu(this);
	header = treeWidgetSearchResults->header();
	action = menuSearchResultsHeader->addAction(tr("Title"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_TITLE);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_TITLE));
	action = menuSearchResultsHeader->addAction(tr("Name"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_NAME);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_NAME));
	action = menuSearchResultsHeader->addAction(tr("Publisher"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PUBLISHER);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_PUBLISHER));
	action = menuSearchResultsHeader->addAction(tr("Year"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_YEAR));
	action = menuSearchResultsHeader->addAction(tr("Part"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PART);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_PART));
	action = menuSearchResultsHeader->addAction(tr("Interface"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_INTERFACE);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_INTERFACE));
	action = menuSearchResultsHeader->addAction(tr("List"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_LIST);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
	action = menuSearchResultsHeader->addAction(tr("Supported"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_SUPPORTED);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
	menuSearchResultsHeader->addSeparator();
	action = menuSearchResultsHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionSearchResultsHeader_triggered())); action->setData(QMC2_SWLIST_RESET);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &)));

	// detail update timer
	connect(&detailUpdateTimer, SIGNAL(timeout()), this, SLOT(updateDetail()));

	// tool box menu hack (for "known software" page)
	toolBoxButtonKnownSoftware = 0;
	foreach(QWidget *w, toolBoxSoftwareList->findChildren<QWidget*>())
	{
		if( w->inherits("QToolBoxButton") ) {
			QAbstractButton *button = qobject_cast<QAbstractButton*>(w);
			if ( button->text() == tr("Known software") ) {
				toolBoxButtonKnownSoftware = new QToolButton(w);
				toolBoxButtonKnownSoftware->setToolButtonStyle(Qt::ToolButtonIconOnly);
				toolBoxButtonKnownSoftware->setPopupMode(QToolButton::InstantPopup);
				toolBoxButtonKnownSoftware->setIconSize(iconSize);
				toolBoxButtonKnownSoftware->setAutoRaise(true);
				QPalette pal = toolBoxButtonKnownSoftware->palette();
				pal.setBrush(QPalette::Window, Qt::transparent);
				pal.setBrush(QPalette::WindowText, Qt::transparent);
				pal.setBrush(QPalette::Text, Qt::transparent);
				pal.setBrush(QPalette::Button, Qt::transparent);
				pal.setBrush(QPalette::Base, Qt::transparent);
				pal.setBrush(QPalette::AlternateBase, Qt::transparent);
				pal.setBrush(QPalette::Light, Qt::transparent);
				pal.setBrush(QPalette::Midlight, Qt::transparent);
				pal.setBrush(QPalette::Dark, Qt::transparent);
				pal.setBrush(QPalette::Mid, Qt::transparent);
				pal.setBrush(QPalette::Shadow, Qt::transparent);
				pal.setBrush(QPalette::Highlight, Qt::transparent);
				pal.setBrush(QPalette::HighlightedText, Qt::transparent);
				pal.setBrush(QPalette::Link, Qt::transparent);
				pal.setBrush(QPalette::LinkVisited, Qt::transparent);
				pal.setBrush(QPalette::NoRole, Qt::transparent);
				toolBoxButtonKnownSoftware->setPalette(pal);
				toolBoxButtonKnownSoftware->setToolTip(tr("Click to open the view menu"));
				break;
			}
		}
	}
	if ( toolBoxButtonKnownSoftware ) {
		QMenu *knownSoftwareMenu = new QMenu(this);
		viewFlatAction = knownSoftwareMenu->addAction(QIcon(":data/img/view_detail.png"), tr("View flat"), this, SLOT(actionViewFlat_triggered()));
		s = tr("View known software as a flat list");
		viewFlatAction->setToolTip(s); viewFlatAction->setStatusTip(s);
		viewFlatAction->setCheckable(true);
		viewTreeAction = knownSoftwareMenu->addAction(QIcon(":data/img/view_tree.png"), tr("View tree"), this, SLOT(actionViewTree_triggered()));
		s = tr("View known software as a parent/clone tree");
		viewTreeAction->setToolTip(s); viewTreeAction->setStatusTip(s);
		viewTreeAction->setCheckable(true);
		toolBoxButtonKnownSoftware->setMenu(knownSoftwareMenu);
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ViewTree", false).toBool() )
			actionViewTree_triggered();
		else
			actionViewFlat_triggered();
	}

	isReady = true;
}

SoftwareList::~SoftwareList()
{
	if ( exporter )
		exporter->close();

	// save widget states
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState", treeWidgetKnownSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareTreeHeaderState", treeWidgetKnownSoftwareTree->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState", treeWidgetFavoriteSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState", treeWidgetSearchResults->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex", toolBoxSoftwareList->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareInfo", toolButtonToggleSoftwareInfo->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/CompatFilter", toolButtonCompatFilterToggle->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AdjustSnapname", toolButtonToggleSnapnameAdjustment->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareStates", toolButtonSoftwareStates->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/NegateSearch", actionNegateSearch->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ViewTree", viewTree());
}

void SoftwareList::adjustSnapnamePattern()
{
	bool ok;
	QStringList items;
	items << "soft-lists/$SOFTWARE_LIST$/$SOFTWARE_NAME$" << "soft-lists/$SOFTWARE_LIST$/$SOFTWARE_NAME$/%i";
	QString storedPattern = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SnapnamePattern", "soft-lists/$SOFTWARE_LIST$/$SOFTWARE_NAME$").toString();
	int index = items.indexOf(storedPattern);
	if ( index < 0 ) {
		items << storedPattern;
		index = 2;
	}
	QString pattern = QInputDialog::getItem(this,
						tr("Snapname adjustment pattern"),
						tr("Enter the pattern used for snapname adjustment:\n(Allowed macros: $SOFTWARE_LIST$, $SOFTWARE_NAME$)"),
						items, index, true, &ok);
	if ( ok )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SnapnamePattern", pattern);
}

void SoftwareList::clearSoftwareSelection()
{
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			treeWidgetKnownSoftware->clearSelection();
			treeWidgetKnownSoftwareTree->clearSelection();
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidgetFavoriteSoftware->clearSelection();
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidgetSearchResults->clearSelection();
			break;
	}
}

void SoftwareList::negateSearchTriggered(bool negate)
{
	IconLineEdit *ile = ((IconLineEdit *)comboBoxSearch->lineEdit());
	if ( negate )
		ile->button()->setIcon(QIcon(QString::fromUtf8(":/data/img/find_negate.png")));
	else
		ile->button()->setIcon(QIcon(QString::fromUtf8(":/data/img/find.png")));
	negatedMatch = negate;

	searchTimer.start(QMC2_SEARCH_DELAY);
}

void SoftwareList::rebuildSoftware()
{
	if ( qmc2SoftwareROMAlyzer && qmc2SoftwareROMAlyzer->rebuilderActive() ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current rebuild and try again"));
		return;
	}

	bool initial = false;
	if ( !qmc2SoftwareROMAlyzer ) {
		qmc2SoftwareROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SOFTWARE);
		initial = true;
	}

	if ( qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabCollectionRebuilder )
		qmc2SoftwareROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SoftwareROMAlyzer->tabCollectionRebuilder);

	if ( qmc2SoftwareROMAlyzer->isHidden() )
		qmc2SoftwareROMAlyzer->show();
	else if ( qmc2SoftwareROMAlyzer->isMinimized() )
		qmc2SoftwareROMAlyzer->showNormal();

	CollectionRebuilder *cr = qmc2SoftwareROMAlyzer->collectionRebuilder();
	if ( cr ) {
		QTreeWidget *treeWidget = 0;
		switch ( toolBoxSoftwareList->currentIndex() ) {
			case QMC2_SWLIST_KNOWN_SW_PAGE:
				if ( stackedWidgetKnownSoftware->currentIndex() == QMC2_SWLIST_KNOWN_SW_PAGE_FLAT )
					treeWidget = treeWidgetKnownSoftware;
				else
					treeWidget = treeWidgetKnownSoftwareTree;
				break;
			case QMC2_SWLIST_FAVORITES_PAGE:
				treeWidget = treeWidgetFavoriteSoftware;
				break;
			case QMC2_SWLIST_SEARCH_PAGE:
				treeWidget = treeWidgetSearchResults;
				break;
		}
		if ( treeWidget ) {
			QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();
			if ( !selectedItems.isEmpty() ) {
				QTreeWidgetItem *item = selectedItems[0];
				if ( viewTree() ) {
					while ( item->whatsThis(QMC2_SWLIST_COLUMN_NAME).isEmpty() && item->parent() )
						item = item->parent();
				} else {
					while ( item->parent() )
						item = item->parent();
				}
				cr->comboBoxXmlSource->setCurrentIndex(0);
				cr->setIgnoreCheckpoint(true);
				cr->checkBoxFilterExpression->setChecked(true);
				cr->comboBoxFilterSyntax->setCurrentIndex(4);
				cr->comboBoxFilterType->setCurrentIndex(0);
				cr->toolButtonExactMatch->setChecked(true);
				cr->checkBoxFilterStates->setChecked(false);
				cr->lineEditFilterExpression->setText(item->text(QMC2_SWLIST_COLUMN_NAME));
				cr->checkBoxFilterExpressionSoftwareLists->setChecked(true);
				cr->comboBoxFilterSyntaxSoftwareLists->setCurrentIndex(4);
				cr->comboBoxFilterTypeSoftwareLists->setCurrentIndex(0);
				cr->toolButtonExactMatchSoftwareLists->setChecked(true);
				cr->lineEditFilterExpressionSoftwareLists->setText(item->text(QMC2_SWLIST_COLUMN_LIST));
				if ( !initial )
					cr->plainTextEditLog->clear();
				QTimer::singleShot(0, cr->pushButtonStartStop, SLOT(animateClick()));
			}
		}
	}

	QTimer::singleShot(0, qmc2SoftwareROMAlyzer, SLOT(raise()));
}

void SoftwareList::rebuildSoftwareList()
{
	if ( qmc2SoftwareROMAlyzer && qmc2SoftwareROMAlyzer->rebuilderActive() ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current rebuild and try again"));
		return;
	}

	bool initial = false;
	if ( !qmc2SoftwareROMAlyzer ) {
		qmc2SoftwareROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SOFTWARE);
		initial = true;
	}

	if ( qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabCollectionRebuilder )
		qmc2SoftwareROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SoftwareROMAlyzer->tabCollectionRebuilder);

	if ( qmc2SoftwareROMAlyzer->isHidden() )
		qmc2SoftwareROMAlyzer->show();
	else if ( qmc2SoftwareROMAlyzer->isMinimized() )
		qmc2SoftwareROMAlyzer->showNormal();

	CollectionRebuilder *cr = qmc2SoftwareROMAlyzer->collectionRebuilder();
	if ( cr ) {
		QTreeWidget *treeWidget = 0;
		switch ( toolBoxSoftwareList->currentIndex() ) {
			case QMC2_SWLIST_KNOWN_SW_PAGE:
				if ( stackedWidgetKnownSoftware->currentIndex() == QMC2_SWLIST_KNOWN_SW_PAGE_FLAT )
					treeWidget = treeWidgetKnownSoftware;
				else
					treeWidget = treeWidgetKnownSoftwareTree;
				break;
			case QMC2_SWLIST_FAVORITES_PAGE:
				treeWidget = treeWidgetFavoriteSoftware;
				break;
			case QMC2_SWLIST_SEARCH_PAGE:
				treeWidget = treeWidgetSearchResults;
				break;
		}
		if ( treeWidget ) {
			QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();
			if ( !selectedItems.isEmpty() ) {
				QTreeWidgetItem *item = selectedItems[0];
				if ( viewTree() ) {
					while ( item->whatsThis(QMC2_SWLIST_COLUMN_NAME).isEmpty() && item->parent() )
						item = item->parent();
				} else {
					while ( item->parent() )
						item = item->parent();
				}
				cr->comboBoxXmlSource->setCurrentIndex(0);
				cr->setIgnoreCheckpoint(true);
				cr->checkBoxFilterExpression->setChecked(false);
				cr->checkBoxFilterExpressionSoftwareLists->setChecked(true);
				cr->comboBoxFilterSyntaxSoftwareLists->setCurrentIndex(4);
				cr->comboBoxFilterTypeSoftwareLists->setCurrentIndex(0);
				cr->toolButtonExactMatchSoftwareLists->setChecked(true);
				cr->lineEditFilterExpressionSoftwareLists->setText(item->text(QMC2_SWLIST_COLUMN_LIST));
				if ( !initial )
					cr->plainTextEditLog->clear();
				QTimer::singleShot(0, cr->pushButtonStartStop, SLOT(animateClick()));
			}
		}
	}

	QTimer::singleShot(0, qmc2SoftwareROMAlyzer, SLOT(raise()));
}

void SoftwareList::rebuildSoftwareLists()
{
	if ( qmc2SoftwareROMAlyzer && qmc2SoftwareROMAlyzer->rebuilderActive() ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current rebuild and try again"));
		return;
	}

	bool initial = false;
	if ( !qmc2SoftwareROMAlyzer ) {
		qmc2SoftwareROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SOFTWARE);
		initial = true;
	}

	if ( qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabCollectionRebuilder )
		qmc2SoftwareROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SoftwareROMAlyzer->tabCollectionRebuilder);

	if ( qmc2SoftwareROMAlyzer->isHidden() )
		qmc2SoftwareROMAlyzer->show();
	else if ( qmc2SoftwareROMAlyzer->isMinimized() )
		qmc2SoftwareROMAlyzer->showNormal();

	CollectionRebuilder *cr = qmc2SoftwareROMAlyzer->collectionRebuilder();
	if ( cr ) {
		cr->comboBoxXmlSource->setCurrentIndex(0);
		cr->setIgnoreCheckpoint(true);
		cr->checkBoxFilterExpression->setChecked(false);
		cr->checkBoxFilterExpressionSoftwareLists->setChecked(true);
		cr->comboBoxFilterSyntaxSoftwareLists->setCurrentIndex(0);
		cr->comboBoxFilterTypeSoftwareLists->setCurrentIndex(0);
		cr->toolButtonExactMatchSoftwareLists->setChecked(true);
		cr->lineEditFilterExpressionSoftwareLists->setText(systemSoftwareListHash[systemName].join("|"));
		if ( !initial )
			cr->plainTextEditLog->clear();
		QTimer::singleShot(0, cr->pushButtonStartStop, SLOT(animateClick()));
	}

	QTimer::singleShot(0, qmc2SoftwareROMAlyzer, SLOT(raise()));
}

void SoftwareList::updateRebuildSoftwareMenuVisibility()
{
	bool enable = false;
	if ( qmc2SoftwareROMAlyzer )
		enable = (qmc2SoftwareROMAlyzer->groupBoxCheckSumDatabase->isChecked() && qmc2SoftwareROMAlyzer->groupBoxSetRewriter->isChecked());
	else
		enable = (qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareROMAlyzer/EnableCheckSumDb", false).toBool() && qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareROMAlyzer/EnableSetRewriter", false).toBool());
	rebuildMenuAction->setVisible(enable);
	toolButtonRebuildSoftware->setVisible(enable);
}

void SoftwareList::analyzeSoftwareMenu_aboutToShow()
{
	QTreeWidget *treeWidget = 0;
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
		case QMC2_SWLIST_KNOWN_SW_PAGE:
		default:
			if ( stackedWidgetKnownSoftware->currentIndex() == QMC2_SWLIST_KNOWN_SW_PAGE_FLAT )
				treeWidget = treeWidgetKnownSoftware;
			else
				treeWidget = treeWidgetKnownSoftwareTree;
			break;
	}
	bool enable = qmc2SoftwareROMAlyzer ? !qmc2SoftwareROMAlyzer->active() : true;
	actionAnalyzeSoftware->setVisible(!treeWidget->selectedItems().isEmpty());
	actionAnalyzeSoftware->setEnabled(enable);
	actionAnalyzeSoftwareList->setVisible(!treeWidget->selectedItems().isEmpty());
	actionAnalyzeSoftwareList->setEnabled(enable);
	actionAnalyzeSoftwareLists->setVisible(actionAnalyzeSoftwareList->isVisible() ? (systemSoftwareListHash[systemName].count() > 1) : true);
	actionAnalyzeSoftwareLists->setEnabled(enable);
}

void SoftwareList::rebuildSoftwareMenu_aboutToShow()
{
	QTreeWidget *treeWidget = 0;
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
		case QMC2_SWLIST_KNOWN_SW_PAGE:
		default:
			if ( stackedWidgetKnownSoftware->currentIndex() == QMC2_SWLIST_KNOWN_SW_PAGE_FLAT )
				treeWidget = treeWidgetKnownSoftware;
			else
				treeWidget = treeWidgetKnownSoftwareTree;
			break;
	}
	bool enable = qmc2SoftwareROMAlyzer ? !qmc2SoftwareROMAlyzer->rebuilderActive() : true;
	actionRebuildSoftware->setVisible(!treeWidget->selectedItems().isEmpty());
	actionRebuildSoftware->setEnabled(enable);
	actionRebuildSoftwareList->setVisible(!treeWidget->selectedItems().isEmpty());
	actionRebuildSoftwareList->setEnabled(enable);
	actionRebuildSoftwareLists->setVisible(actionRebuildSoftwareList->isVisible() ? (systemSoftwareListHash[systemName].count() > 1) : true);
	actionRebuildSoftwareLists->setEnabled(enable);
}

void SoftwareList::actionViewFlat_triggered()
{
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/view_detail.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	viewFlatAction->setChecked(true);
	viewTreeAction->setChecked(false);
	toolButtonCompatFilterToggle->setEnabled(true);
	stateFilter->setEnabled(true);
	setViewTree(false);
	stackedWidgetKnownSoftware->setCurrentIndex(QMC2_SWLIST_KNOWN_SW_PAGE_FLAT);
}

void SoftwareList::actionViewTree_triggered()
{
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/view_tree.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	viewFlatAction->setChecked(false);
	viewTreeAction->setChecked(true);
	toolButtonCompatFilterToggle->setEnabled(false);
	stateFilter->setEnabled(false);
	setViewTree(true);
	stackedWidgetKnownSoftware->setCurrentIndex(QMC2_SWLIST_KNOWN_SW_PAGE_TREE);
}

QString &SoftwareList::getSoftwareListXmlData(QString listName)
{
	static QString softwareListBuffer;
	softwareListBuffer = swlDb->allXml(listName);
	return softwareListBuffer;
}

QString &SoftwareList::getXmlDataWithEnabledSlots(QStringList swlArgs)
{
	static QString xmlBuffer;

	xmlBuffer.clear();

	qmc2CriticalSection = true;

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	QProcess commandProc;
#if defined(QMC2_SDLMESS)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLMAME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLUME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlume.tmp").toString());
#elif defined(QMC2_UME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-ume.tmp").toString());
#endif
#if !defined(QMC2_OS_WIN)
	commandProc.setStandardErrorFile("/dev/null");
#endif

	QStringList args;
	args << systemName << swlArgs << "-listxml";

#ifdef QMC2_DEBUG
	printf("SoftwareList::getXmlDataWithEnabledSlots(): args = %s\n", (const char *)args.join(" ").toLocal8Bit());
#endif

	bool commandProcStarted = false;
	int retries = 0;
	commandProc.start(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString(), args);
	bool started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
	while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
		started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
		qApp->processEvents();
	}

	if ( started ) {
		commandProcStarted = true;
		bool commandProcRunning = (commandProc.state() == QProcess::Running);
		while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
			qApp->processEvents();
			commandProcRunning = (commandProc.state() == QProcess::Running);
		}
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start emulator executable within a reasonable time frame, giving up"));
		qmc2CriticalSection = false;
		return xmlBuffer;
	}

#if defined(QMC2_SDLMESS)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLMAME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLUME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlume.tmp").toString());
#elif defined(QMC2_UME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-ume.tmp").toString());
#endif

	if ( commandProcStarted && qmc2TempXml.open(QFile::ReadOnly) ) {
		QTextStream ts(&qmc2TempXml);
		ts.setCodec(QTextCodec::codecForName("UTF-8"));
		xmlBuffer = ts.readAll();
#if defined(QMC2_OS_WIN)
		xmlBuffer.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
		qmc2TempXml.close();
		qmc2TempXml.remove();
		if ( !xmlBuffer.isEmpty() ) {
			QStringList xmlLines = xmlBuffer.split("\n");
			qApp->processEvents();
			xmlBuffer.clear();
			if ( !xmlLines.isEmpty() ) {
				int i = 0;
#if defined(QMC2_EMUTYPE_MESS)
				QString s = "<machine name=\"" + systemName + "\"";
#elif defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
				QString s = "<game name=\"" + systemName + "\"";
#endif
				while ( i < xmlLines.count() && !xmlLines[i].contains(s) ) i++;
				xmlBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
				if ( i < xmlLines.count() ) {
#if defined(QMC2_EMUTYPE_MESS)
					while ( i < xmlLines.count() && !xmlLines[i].contains("</machine>") )
						xmlBuffer += xmlLines[i++].simplified() + "\n";
					if ( i == xmlLines.count() && !xmlLines[i - 1].contains("</machine>") ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: invalid XML data retrieved for '%1'").arg(systemName));
						xmlBuffer.clear();
					} else
						xmlBuffer += "</machine>\n";
#elif defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
					while ( i < xmlLines.count() && !xmlLines[i].contains("</game>") )
						xmlBuffer += xmlLines[i++].simplified() + "\n";
					if ( i == xmlLines.count() && !xmlLines[i - 1].contains("</game>") ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: invalid XML data retrieved for '%1'").arg(systemName));
						xmlBuffer.clear();
					} else
						xmlBuffer += "</game>\n";
#endif
				} else {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: invalid XML data retrieved for '%1'").arg(systemName));
					xmlBuffer.clear();
				}
			}
		}
	}

	qmc2CriticalSection = false;
	return xmlBuffer;
}

void SoftwareList::on_comboBoxDeviceConfiguration_currentIndexChanged(int index)
{
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
	QTimer::singleShot(0, this, SLOT(updateMountDevices()));
#endif
}

QString &SoftwareList::lookupMountDevice(QString device, QString deviceInterface, QStringList *mountList)
{
	static QString softwareListDeviceName;

	QMap<QString, QStringList> deviceInstanceMap;
	softwareListDeviceName.clear();

	QStringList xmlLines = qmc2Gamelist->xmlDb()->xml(systemName).split("\n", QString::SkipEmptyParts);
	QStringList *xmlData = &xmlLines;
	QStringList dynamicXmlData;
	if ( comboBoxDeviceConfiguration->currentIndex() > 0 ) {
		qmc2Config->beginGroup(QMC2_EMULATOR_PREFIX + QString("Configuration/Devices/%1/%2").arg(systemName).arg(comboBoxDeviceConfiguration->currentText()));
		QStringList instances = qmc2Config->value("Instances").toStringList();
		QStringList files = qmc2Config->value("Files").toStringList();
		QStringList slotNames = qmc2Config->value("Slots").toStringList();
		QStringList slotOptions = qmc2Config->value("SlotOptions").toStringList();
		QStringList slotBIOSs = qmc2Config->value("SlotBIOSs").toStringList();
		qmc2Config->endGroup();
		QStringList swlArgs;
		for (int j = 0; j < slotNames.count(); j++) {
			if ( !slotOptions[j].isEmpty() ) {
				QString slotOpt = slotOptions[j];
				if ( !slotBIOSs[j].isEmpty() )
					slotOpt += ",bios=" + slotBIOSs[j];
				swlArgs << QString("-%1").arg(slotNames[j]) << slotOpt;
			}
		}
		for (int j = 0; j < instances.count(); j++) {
#if defined(QMC2_OS_WIN)
			swlArgs << QString("-%1").arg(instances[j]) << files[j].replace('/', '\\');
#else
			swlArgs << QString("-%1").arg(instances[j]) << files[j].replace("~", "$HOME");
#endif
		}
		foreach (QString line, getXmlDataWithEnabledSlots(swlArgs).split("\n", QString::SkipEmptyParts))
			dynamicXmlData << line.trimmed();
		xmlData = &dynamicXmlData;
#ifdef QMC2_DEBUG
		printf("SoftwareList::getXmlDataWithEnabledSlots(): XML data start\n");
		foreach (QString line, dynamicXmlData)
			printf("%s\n", (const char *)line.toLocal8Bit());
		printf("SoftwareList::getXmlDataWithEnabledSlots(): XML data end\n");
#endif
	}

	int i = 0;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	QString s = "<game name=\"" + systemName + "\"";
	while ( i < xmlData->count() && !(*xmlData)[i].contains(s) ) i++;
	while ( i < xmlData->count() && !(*xmlData)[i].contains("</game>") ) {
		QString line = (*xmlData)[i++].simplified();
		if ( line.startsWith("<device type=\"") ) {
			int startIndex = line.indexOf("interface=\"");
			int endIndex;
			if ( startIndex >= 0 ) {
				startIndex += 11;
				int endIndex = line.indexOf("\"", startIndex);
				QStringList devInterfaces = line.mid(startIndex, endIndex - startIndex).split(",", QString::SkipEmptyParts);
				line = xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					foreach (QString devIf, devInterfaces)
						deviceInstanceMap[devIf] << devName;
			} else {
				line = (*xmlData)[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devName] << devName;
			}
		}
	}
#elif defined(QMC2_EMUTYPE_MESS)
	QString s = "<machine name=\"" + systemName + "\"";
	while ( i < xmlData->count() && !(*xmlData)[i].contains(s) ) i++;
	while ( i < xmlData->count() && !(*xmlData)[i].contains("</machine>") ) {
		QString line = (*xmlData)[i++].simplified();
		if ( line.startsWith("<device type=\"") ) {
			int startIndex = line.indexOf("interface=\"");
			int endIndex;
			if ( startIndex >= 0 ) {
				startIndex += 11;
				int endIndex = line.indexOf("\"", startIndex);
				QStringList devInterfaces = line.mid(startIndex, endIndex - startIndex).split(",", QString::SkipEmptyParts);
				line = (*xmlData)[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					foreach (QString devIf, devInterfaces)
						deviceInstanceMap[devIf] << devName;
			} else {
				line = (*xmlData)[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devName] << devName;
			}
		}
	}
#endif

	QStringList briefNames = deviceInstanceMap[deviceInterface];
	briefNames.sort();

	if ( briefNames.contains(device) )
		softwareListDeviceName = device;
	else for (int i = 0; i < briefNames.count() && softwareListDeviceName.isEmpty(); i++) {
			softwareListDeviceName = briefNames[i];
			if ( successfulLookups.contains(softwareListDeviceName) )
				softwareListDeviceName.clear();
	}

	if ( successfulLookups.contains(softwareListDeviceName) )
		softwareListDeviceName.clear();

	if ( mountList != NULL )
		*mountList = briefNames;

	if ( !softwareListDeviceName.isEmpty() )
		successfulLookups << softwareListDeviceName;

	return softwareListDeviceName;
}

void SoftwareList::getXmlData()
{
	QStringList softwareList = systemSoftwareListHash[systemName];
	if ( softwareList.isEmpty() || softwareList.contains("NO_SOFTWARE_LIST") ) {
		softwareList.clear();
		int i = 0;
		QString filter;
		QStringList xmlLines = qmc2Gamelist->xmlDb()->xml(systemName).split("\n", QString::SkipEmptyParts);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		while ( !interruptLoad && i < xmlLines.count() && !xmlLines[i].contains("</game>") ) {
			QString line = xmlLines[i++];
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				softwareList << line.mid(startIndex, endIndex - startIndex); 
				startIndex = line.indexOf(" filter=\"");
				if ( startIndex >= 0 ) {
					startIndex += 9;
					endIndex = line.indexOf("\"", startIndex);
					filter = line.mid(startIndex, endIndex - startIndex);
				}
			}
		}
#elif defined(QMC2_EMUTYPE_MESS)
		while ( !interruptLoad && i < xmlLines.count() && !xmlLines[i].contains("</machine>") ) {
			QString line = xmlLines[i++];
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				softwareList << line.mid(startIndex, endIndex - startIndex); 
				startIndex = line.indexOf(" filter=\"");
				if ( startIndex >= 0 ) {
					startIndex += 9;
					endIndex = line.indexOf("\"", startIndex);
					filter = line.mid(startIndex, endIndex - startIndex);
				}
			}
		}
#endif

		if ( softwareList.isEmpty() )
			softwareList << "NO_SOFTWARE_LIST";
		else
			softwareList.sort();
		systemSoftwareListHash[systemName] = softwareList;

#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListHash[%1] = %2").arg(systemName).arg(softwareList.join(", ")));
#endif

		if ( !filter.isEmpty() )
			systemSoftwareFilterHash[systemName] = filter.split(",", QString::SkipEmptyParts);
	}
#ifdef QMC2_DEBUG
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListHash[%1] = %2 (cached)").arg(systemName).arg(systemSoftwareListHash[systemName].join(", ")));
#endif

	if ( !softwareList.isEmpty() && !softwareList.contains("NO_SOFTWARE_LIST") ) {
		QString swlString = softwareList.join(", ");
		if ( toolButtonSoftwareStates->isChecked() && stateFilter->checkBoxStateFilter->isChecked() )
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (%1)").arg(swlString) + " - " + tr("filtered"));
		else
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (%1)").arg(swlString));
		toolBoxSoftwareList->setEnabled(true);

#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		// load stored device configurations, if any...
		qmc2Config->beginGroup(QString(QMC2_EMULATOR_PREFIX + "Configuration/Devices/%1").arg(systemName));
		QStringList configurationList = qmc2Config->childGroups();
		qmc2Config->endGroup();
		if ( !configurationList.isEmpty() ) {
			comboBoxDeviceConfiguration->insertItems(1, configurationList);
			comboBoxDeviceConfiguration->setEnabled(true);
		}
#endif
	} else {
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (no data available)"));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (no data available)"));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (no data available)"));
	}
}

void SoftwareList::updateMountDevices()
{
	if ( updatingMountDevices )
		return;

	updatingMountDevices = true;
	autoMounted = true;

	QTreeWidget *treeWidget = NULL;
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			/* FIXME
			if ( stackedWidgetKnownSoftware->currentIndex() == QMC2_SWLIST_KNOWN_SW_PAGE_FLAT )
				treeWidget = treeWidgetKnownSoftware;
			else
				treeWidget = treeWidgetKnownSoftwareTree;
			*/
			treeWidget = treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
	}

	QTreeWidgetItemIterator it(treeWidget);
	while ( *it ) {
		QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
		if ( comboBox ) {
			comboBox->blockSignals(true);
			comboBox->setUpdatesEnabled(false);
			comboBox->clear();
			QStringList mountList;
			successfulLookups.clear();
			QString mountDev = lookupMountDevice((*it)->text(QMC2_SWLIST_COLUMN_PART), (*it)->text(QMC2_SWLIST_COLUMN_INTERFACE), &mountList);
			if ( mountList.count() > 0 ) {
				mountList.prepend(QObject::tr("Don't mount"));
				mountList.prepend(QObject::tr("Auto mount"));
				comboBox->insertItems(0, mountList);
				comboBox->insertSeparator(QMC2_SWLIST_MSEL_SEPARATOR);
				comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_AUTO_MOUNT);
				if ( mountDev.isEmpty() )
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				else
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + mountDev);
			} else {
				(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("No mount device"));
				(*it)->setText(QMC2_SWLIST_COLUMN_PUBLISHER, QObject::tr("Unmanaged"));
			}
			comboBox->setUpdatesEnabled(true);
			comboBox->blockSignals(false);
		}
		it++;
	}

	updatingMountDevices = false;
}

bool SoftwareList::load()
{
	setEnabled(qmc2UseDefaultEmulator);

	autoMounted = true;
	interruptLoad = false;
	isLoading = true;
	fullyLoaded = false;
	validData = swlSupported;
	numSoftwareTotal = numSoftwareCorrect = numSoftwareIncorrect = numSoftwareMostlyCorrect = numSoftwareNotFound = numSoftwareUnknown = 0;
	updateStats();

	toolButtonReload->setEnabled(false);

	treeWidgetKnownSoftware->clear();
	treeWidgetFavoriteSoftware->clear();
	treeWidgetSearchResults->clear();
	softwareItemHash.clear();
	softwareHierarchyItemHash.clear();
	softwareParentHash.clear();

	treeWidgetKnownSoftware->setSortingEnabled(false);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(false);
	treeWidgetKnownSoftwareTree->setSortingEnabled(false);
	treeWidgetKnownSoftwareTree->header()->setSortIndicatorShown(false);
	treeWidgetFavoriteSoftware->setSortingEnabled(false);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(false);
	treeWidgetSearchResults->setSortingEnabled(false);
	treeWidgetSearchResults->header()->setSortIndicatorShown(false);

	bool swlCacheOkay = (swlDb->swlRowCount() > 0) && (qmc2Gamelist->emulatorVersion == swlDb->emulatorVersion());

	if ( swlSupported && !swlCacheOkay ) {
		isInitialLoad = true;
		oldMin = qmc2MainWindow->progressBarGamelist->minimum();
		oldMax = qmc2MainWindow->progressBarGamelist->maximum();
		oldFmt = qmc2MainWindow->progressBarGamelist->format();
          	qmc2MainWindow->tabSoftwareList->setUpdatesEnabled(true);
		labelLoadingSoftwareLists->setText(tr("Loading software-lists, please wait..."));
		labelLoadingSoftwareLists->setVisible(true);
		toolBoxSoftwareList->setVisible(false);
		show();
		qApp->processEvents();
		loadTimer.start();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML software list data and recreating cache"));
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("SWL data - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		swlLastLine.clear();
		swlDb->recreateDatabase(true);
		uncommittedSwlDbRows = 0;
		loadProc = new QProcess(this);
		connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(readyReadStandardError()), this, SLOT(loadReadyReadStandardError()));
		connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));

		QString command = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", QString()).toString();
		QStringList args;
		args << "-listsoftware";
		QString hashPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath", QString()).toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;

		if ( !qmc2StopParser ) {
			validData = true;
			loadFinishedFlag = false;
			QString emuWorkDir = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString();
			if ( !emuWorkDir.isEmpty() )
				loadProc->setWorkingDirectory(emuWorkDir);
			loadProc->start(command, args);
			// FIXME: this is blocking the GUI shortly
			if ( loadProc->waitForStarted() && !qmc2StopParser ) {
				while ( !loadFinishedFlag && !qmc2StopParser ) {
					qApp->processEvents();
#if defined(QMC2_OS_MAC)
					QTest::qWait(10);
#else
					loadProc->waitForFinished(10);
#endif
				}
			} else
				validData = false;
		} 

		if ( qmc2StopParser ) {
			if ( loadProc->state() == QProcess::Running ) {
				loadProc->kill();
				validData = false;
			}
		}

		if ( !validData ) {
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (no data available)"));
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (no data available)"));
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (no data available)"));

			labelLoadingSoftwareLists->setVisible(false);
			toolBoxSoftwareList->setVisible(true);

			isLoading = false;
			isInitialLoad = false;
			emit loadFinished(false);
			return false;
		}
	}

	labelLoadingSoftwareLists->setVisible(false);
	toolBoxSoftwareList->setVisible(true);

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): validData = %1").arg(validData));
#endif

	getXmlData();

	QStringList softwareList = systemSoftwareListHash[systemName];
	if ( !softwareList.contains("NO_SOFTWARE_LIST") && !interruptLoad ) {
		foreach (QString swList, softwareList) {
			if ( interruptLoad )
				break;
			QString softwareListXml = getSoftwareListXmlData(swList);
			if ( interruptLoad )
				break;
			if ( softwareListXml.size() > QMC2_SWLIST_SIZE_THRESHOLD ) {
				toolBoxSoftwareList->setVisible(false);
				labelLoadingSoftwareLists->setText(tr("Loading software-list '%1', please wait...").arg(swList));
				labelLoadingSoftwareLists->setVisible(true);
				qmc2MainWindow->tabSoftwareList->update();
				qApp->processEvents();
			}
			if ( !softwareListXml.isEmpty() ) {
#ifdef QMC2_DEBUG
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): XML data for software list '%1' follows:\n%2").arg(swList).arg(softwareListXml));
#endif
				QXmlInputSource xmlInputSource;
				xmlInputSource.setData(softwareListXml);
				SoftwareListXmlHandler xmlHandler(treeWidgetKnownSoftware);
				QXmlSimpleReader xmlReader;
				xmlReader.setContentHandler(&xmlHandler);
				if ( !xmlReader.parse(xmlInputSource) && !interruptLoad )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list '%1'").arg(swList));
				else if ( xmlHandler.newSoftwareStates )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("state info for software-list '%1': L:%2 C:%3 M:%4 I:%5 N:%6 U:%7").arg(swList).arg(xmlHandler.numTotal).arg(xmlHandler.numCorrect).arg(xmlHandler.numMostlyCorrect).arg(xmlHandler.numIncorrect).arg(xmlHandler.numNotFound).arg(xmlHandler.numUnknown));
				numSoftwareTotal += xmlHandler.numTotal;
				numSoftwareCorrect += xmlHandler.numCorrect;
				numSoftwareIncorrect += xmlHandler.numIncorrect;
				numSoftwareMostlyCorrect += xmlHandler.numMostlyCorrect;
				numSoftwareNotFound += xmlHandler.numNotFound;
				numSoftwareUnknown += xmlHandler.numUnknown;
			}
			updateStats();
		}

		if ( viewTree() )
			QTimer::singleShot(0, this, SLOT(loadTree()));

		QTimer::singleShot(0, labelLoadingSoftwareLists, SLOT(hide()));
		QTimer::singleShot(0, toolBoxSoftwareList, SLOT(show()));

		// load favorites
#if defined(QMC2_EMUTYPE_MAME)
		QStringList softwareNames = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/SoftwareNames").arg(systemName)).toStringList();
#elif defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		QStringList softwareNames = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/SoftwareNames").arg(systemName)).toStringList();
		QStringList configNames = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/DeviceConfigs").arg(systemName)).toStringList();
#endif

		QStringList compatFilters = systemSoftwareFilterHash[systemName];
		for (int i = 0; i < softwareNames.count() && !interruptLoad; i++) {
			if ( interruptLoad )
				break;
			QString software = softwareNames[i];
			QList<QTreeWidgetItem *> matchedSoftware = treeWidgetKnownSoftware->findItems(software, Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
			QTreeWidgetItem *swItem = NULL;
			if ( matchedSoftware.count() > 0 ) swItem = matchedSoftware.at(0);
			if ( swItem ) {
				SoftwareItem *item = new SoftwareItem(treeWidgetFavoriteSoftware);
				item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, swItem->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
				item->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, swItem->whatsThis(QMC2_SWLIST_COLUMN_NAME));
				bool showItem = true;
				if ( toolButtonCompatFilterToggle->isChecked() ) {
					QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
					showItem = compatList.isEmpty() || compatFilters.isEmpty();
					for (int i = 0; i < compatList.count() && !showItem; i++)
						for (int j = 0; j < compatFilters.count() && !showItem; j++)
							showItem = (compatList[i] == compatFilters[j]);
				}
				item->setHidden(!showItem);
				item->setText(QMC2_SWLIST_COLUMN_TITLE, swItem->text(QMC2_SWLIST_COLUMN_TITLE));
				item->setIcon(QMC2_SWLIST_COLUMN_TITLE, swItem->icon(QMC2_SWLIST_COLUMN_TITLE));
				item->setText(QMC2_SWLIST_COLUMN_NAME, swItem->text(QMC2_SWLIST_COLUMN_NAME));
				item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, swItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
				item->setText(QMC2_SWLIST_COLUMN_YEAR, swItem->text(QMC2_SWLIST_COLUMN_YEAR));
				item->setText(QMC2_SWLIST_COLUMN_PART, swItem->text(QMC2_SWLIST_COLUMN_PART));
				item->setText(QMC2_SWLIST_COLUMN_INTERFACE, swItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
				item->setText(QMC2_SWLIST_COLUMN_LIST, swItem->text(QMC2_SWLIST_COLUMN_LIST));
				item->setText(QMC2_SWLIST_COLUMN_SUPPORTED, swItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
				SoftwareItem *subItem = new SoftwareItem(item);
				subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
				if ( configNames.count() > i )
					item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, configNames[i]);
#endif
			}
		}
		actionSaveFavoritesToFile->setEnabled(softwareNames.count() > 0);
		toolButtonFavoritesOptions->setEnabled(true);
		toolButtonExport->setEnabled(true);
	}

	treeWidgetKnownSoftware->setSortingEnabled(true);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(true);
	treeWidgetKnownSoftwareTree->setSortingEnabled(true);
	treeWidgetKnownSoftwareTree->header()->setSortIndicatorShown(true);
	treeWidgetFavoriteSoftware->setSortingEnabled(true);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(true);
	treeWidgetSearchResults->setSortingEnabled(true);
	treeWidgetSearchResults->header()->setSortIndicatorShown(true);

	toolButtonReload->setEnabled(true);
	toolButtonToggleSoftwareInfo->setEnabled(true);
	toolButtonCompatFilterToggle->setEnabled(!viewTree());
	toolButtonToggleSnapnameAdjustment->setEnabled(true);
	toolButtonSoftwareStates->setEnabled(true);
	toolButtonAnalyzeSoftware->setEnabled(true);
	toolButtonRebuildSoftware->setEnabled(true);

	isLoading = false;
	fullyLoaded = !interruptLoad;
	isInitialLoad = false;
	emit loadFinished(true);
	return true;
}

void SoftwareList::loadTree()
{
	QHash<QString, SoftwareItem *> parentItemHash;
	QList<QTreeWidgetItem *> itemList;
	foreach (QString setKey, softwareParentHash.keys()) {
		if ( interruptLoad )
			break;
		QString parentSetKey = softwareParentHash[setKey];
		if ( parentSetKey == "<no_parent>" ) {
			if ( parentItemHash.contains(setKey) )
				continue;
			SoftwareItem *baseItem = softwareItemHash[setKey];
			if ( baseItem ) {
				SoftwareItem *parentItem = new SoftwareItem((QTreeWidget *)0);
				parentItem->setText(QMC2_SWLIST_COLUMN_TITLE, baseItem->text(QMC2_SWLIST_COLUMN_TITLE));
				parentItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, baseItem->icon(QMC2_SWLIST_COLUMN_TITLE));
				parentItem->setText(QMC2_SWLIST_COLUMN_NAME, baseItem->text(QMC2_SWLIST_COLUMN_NAME));
				parentItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, baseItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
				parentItem->setText(QMC2_SWLIST_COLUMN_YEAR, baseItem->text(QMC2_SWLIST_COLUMN_YEAR));
				parentItem->setText(QMC2_SWLIST_COLUMN_PART, baseItem->text(QMC2_SWLIST_COLUMN_PART));
				parentItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, baseItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
				parentItem->setText(QMC2_SWLIST_COLUMN_LIST, baseItem->text(QMC2_SWLIST_COLUMN_LIST));
				parentItem->setText(QMC2_SWLIST_COLUMN_SUPPORTED, baseItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
				parentItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "p");
				parentItemHash[setKey] = parentItem;
				softwareHierarchyItemHash[setKey] = parentItem;
				itemList << parentItem;
			}
		} else {
			SoftwareItem *parentItem = 0;
			if ( parentItemHash.contains(parentSetKey) )
				parentItem = parentItemHash[parentSetKey];
			if ( !parentItem ) {
				SoftwareItem *baseItem = softwareItemHash[parentSetKey];
				if ( baseItem ) {
					parentItem = new SoftwareItem((QTreeWidget *)0);
					parentItem->setText(QMC2_SWLIST_COLUMN_TITLE, baseItem->text(QMC2_SWLIST_COLUMN_TITLE));
					parentItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, baseItem->icon(QMC2_SWLIST_COLUMN_TITLE));
					parentItem->setText(QMC2_SWLIST_COLUMN_NAME, baseItem->text(QMC2_SWLIST_COLUMN_NAME));
					parentItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, baseItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
					parentItem->setText(QMC2_SWLIST_COLUMN_YEAR, baseItem->text(QMC2_SWLIST_COLUMN_YEAR));
					parentItem->setText(QMC2_SWLIST_COLUMN_PART, baseItem->text(QMC2_SWLIST_COLUMN_PART));
					parentItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, baseItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
					parentItem->setText(QMC2_SWLIST_COLUMN_LIST, baseItem->text(QMC2_SWLIST_COLUMN_LIST));
					parentItem->setText(QMC2_SWLIST_COLUMN_SUPPORTED, baseItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
					parentItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "p");
					parentItemHash[parentSetKey] = parentItem;
					softwareHierarchyItemHash[parentSetKey] = parentItem;
					itemList << parentItem;
				}
			}
			if ( parentItem ) {
				SoftwareItem *baseItem = softwareItemHash[setKey];
				if ( baseItem ) {
					SoftwareItem *childItem = new SoftwareItem(parentItem);
					childItem->setText(QMC2_SWLIST_COLUMN_TITLE, baseItem->text(QMC2_SWLIST_COLUMN_TITLE));
					childItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, baseItem->icon(QMC2_SWLIST_COLUMN_TITLE));
					childItem->setText(QMC2_SWLIST_COLUMN_NAME, baseItem->text(QMC2_SWLIST_COLUMN_NAME));
					childItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, baseItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
					childItem->setText(QMC2_SWLIST_COLUMN_YEAR, baseItem->text(QMC2_SWLIST_COLUMN_YEAR));
					childItem->setText(QMC2_SWLIST_COLUMN_PART, baseItem->text(QMC2_SWLIST_COLUMN_PART));
					childItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, baseItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
					childItem->setText(QMC2_SWLIST_COLUMN_LIST, baseItem->text(QMC2_SWLIST_COLUMN_LIST));
					childItem->setText(QMC2_SWLIST_COLUMN_SUPPORTED, baseItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
					childItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "c");
					softwareHierarchyItemHash[setKey] = childItem;
				}
			}
		}
	}
	if ( !interruptLoad )
		treeWidgetKnownSoftwareTree->insertTopLevelItems(0, itemList);
}

bool SoftwareList::save()
{
	if ( !fullyLoaded )
		return false;

	qmc2Config->remove(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1").arg(systemName));

	QStringList softwareNames;
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
	QStringList configNames;
	bool onlyEmptyConfigNames = true;
#endif

	for (int i = 0; i < treeWidgetFavoriteSoftware->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidgetFavoriteSoftware->topLevelItem(i);
		softwareNames << item->text(QMC2_SWLIST_COLUMN_NAME);
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		QString s = item->text(QMC2_SWLIST_COLUMN_DEVICECFG);
		if ( !s.isEmpty() )
			onlyEmptyConfigNames = false;
		configNames << s;
#endif
	}

	if ( !softwareNames.isEmpty() ) {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/SoftwareNames").arg(systemName), softwareNames);
#elif defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/SoftwareNames").arg(systemName), softwareNames);
		if ( onlyEmptyConfigNames )
			qmc2Config->remove(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/DeviceConfigs").arg(systemName));
		else
			qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/DeviceConfigs").arg(systemName), configNames);
#endif
	}

	return true;
}

void SoftwareList::closeEvent(QCloseEvent *e)
{
	QWidget::closeEvent(e);
}

void SoftwareList::hideEvent(QHideEvent *e)
{
	QWidget::hideEvent(e);
}

void SoftwareList::leaveEvent(QEvent *e)
{
	snapForced = false;

	if ( qmc2SoftwareSnap )
		if ( qmc2SoftwareSnap->geometry().contains(QCursor::pos()) ) {
			snapForced = true;
			snapTimer.start(QMC2_SWSNAP_DELAY);
		}

	if ( !snapForced )
		cancelSoftwareSnap();
	else
		QTimer::singleShot(QMC2_SWSNAP_UNFORCE_DELAY, this, SLOT(checkSoftwareSnap()));

	QWidget::leaveEvent(e);
}

void SoftwareList::checkSoftwareSnap()
{
	if ( qmc2SoftwareSnap && qmc2SoftwareSnap->isVisible() ) {
		if ( !qmc2SoftwareSnap->geometry().contains(QCursor::pos()) && !qmc2SoftwareSnap->ctxMenuRequested )
			cancelSoftwareSnap();
		else {
			qmc2SoftwareSnap->ctxMenuRequested = qmc2SoftwareSnap->contextMenu->isVisible();
			QTimer::singleShot(QMC2_SWSNAP_UNFORCE_DELAY, this, SLOT(checkSoftwareSnap()));
		}
	}
}

void SoftwareList::updateDetail()
{
	qmc2MainWindow->tabWidgetSoftwareDetail_updateCurrent();
}

void SoftwareList::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
}

void SoftwareList::mouseMoveEvent(QMouseEvent *e)
{
	cancelSoftwareSnap();

	QWidget::mouseMoveEvent(e);
}

void SoftwareList::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);
}

void SoftwareList::loadStarted()
{
	// we don't know how many items there are...
	loadFinishedFlag = false;
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarGamelist->setFormat(tr("SWL data - %p%"));
	else
		qmc2MainWindow->progressBarGamelist->setFormat("%p%");
	qmc2MainWindow->progressBarGamelist->setRange(0, 0);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	if ( exitStatus == QProcess::NormalExit && exitCode == 0 ) {
		validData = true;
	} else {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#elif defined(QMC2_EMUTYPE_MESS)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#elif defined(QMC2_EMUTYPE_UME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the UME software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#endif
		validData = false;
	}
	QTime elapsedTime(0, 0, 0, 0);
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data and recreating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	swlDb->commitTransaction();
	uncommittedSwlDbRows = 0;
	loadFinishedFlag = true;
	qmc2MainWindow->progressBarGamelist->setRange(oldMin, oldMax);
	qmc2MainWindow->progressBarGamelist->setFormat(oldFmt);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();

	static QString dtdBuffer;
	static QString currentListName;
	static QString currentSetName;
	static QString setXmlBuffer;
	static bool dtdReady = false;

	if ( qmc2MainWindow->progressBarGamelist->minimum() != 0 || qmc2MainWindow->progressBarGamelist->maximum() != 0 ) {
		qmc2MainWindow->progressBarGamelist->setRange(0, 0);
		qmc2MainWindow->progressBarGamelist->reset();
	}

	// this makes the GUI much more responsive, but is HAS to be called before proc->readAllStandardOutput()!
	if ( !qmc2VerifyActive )
		qApp->processEvents();

#if defined(QMC2_OS_WIN)
	QString readBuffer = swlLastLine + QString::fromUtf8(proc->readAllStandardOutput());
#else
	QString readBuffer = swlLastLine + proc->readAllStandardOutput();
#endif

	QStringList lines = readBuffer.split("\n");

	if ( readBuffer.endsWith("\n") )
		swlLastLine.clear();
	else {
		swlLastLine = lines.last();
		lines.removeLast();
	}

	if ( uncommittedSwlDbRows == 0 )
		swlDb->beginTransaction();

	foreach (QString line, lines) {
		line = line.trimmed();
		if ( !line.isEmpty() ) {
			if ( !line.startsWith("<?xml") ) {
				if ( line.startsWith("<!") )
					dtdBuffer += line + "\n";
				else if ( line.startsWith("]>") ) {
					dtdBuffer += line;
					dtdReady = true;
					swlDb->setDtd(dtdBuffer);
					dtdBuffer.clear();
				} else if ( dtdReady ) {
					int startIndex = line.indexOf("<softwarelist name=\"");
					int endIndex = -1;
					if ( startIndex >= 0 ) {
						startIndex += 20;
						endIndex = line.indexOf("\"", startIndex);
						if ( endIndex >= 0 )
							currentListName = line.mid(startIndex, endIndex - startIndex);
					} else if ( line.startsWith("</softwarelist>") )
						currentListName.clear();
					else if ( !currentListName.isEmpty() ) {
						startIndex = line.indexOf("<software name=\"");
						if ( startIndex >= 0 ) {
							startIndex += 16;
							endIndex = line.indexOf("\"", startIndex);
							if ( endIndex >= 0 )
								currentSetName = line.mid(startIndex, endIndex - startIndex);
						} else if ( line.startsWith("</software>") ) {
							if ( swlDb->exists(currentListName, currentSetName) )
								qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: software-list XML bug: the software name '%1' is used multiple times within software-list '%2'").arg(currentSetName).arg(currentListName));
							else if ( !currentSetName.isEmpty() ) {
								setXmlBuffer += line;
								swlDb->setXml(currentListName, currentSetName, setXmlBuffer);
								uncommittedSwlDbRows++;
							}
							currentSetName.clear();
							setXmlBuffer.clear();
						}
						if ( !currentSetName.isEmpty() )
							setXmlBuffer += line + "\n";
					}
				}
			} else {
				dtdBuffer.clear();
				setXmlBuffer.clear();
				currentListName.clear();
				currentSetName.clear();
				dtdReady = false;
			}
		}
	}
	if ( uncommittedSwlDbRows >= QMC2_SWLCACHE_COMMIT ) {
		swlDb->commitTransaction();
		uncommittedSwlDbRows = 0;
	}
}

void SoftwareList::loadReadyReadStandardError()
{
	QProcess *proc = (QProcess *)sender();

	QString data = proc->readAllStandardError();
	data = data.trimmed();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadReadyReadStandardError(): data = '%1'").arg(data));
#endif

	if ( data.contains("unknown option: -listsoftware") || data.contains("Unknown command 'listsoftware' specified") ) {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the currently selected MAME emulator doesn't support software lists"));
#elif defined(QMC2_EMUTYPE_MESS)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the currently selected MESS emulator doesn't support software lists"));
#elif defined(QMC2_EMUTYPE_UME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the currently selected UME emulator doesn't support software lists"));
#endif
		swlSupported = false;
	}
}

void SoftwareList::loadError(QProcess::ProcessError processError)
{
#if defined(QMC2_EMUTYPE_MAME)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists caused an error -- processError = %1").arg(processError));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists caused an error -- processError = %1").arg(processError));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the UME software lists caused an error -- processError = %1").arg(processError));
#endif
	validData = false;
	loadFinishedFlag = true;

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::checkSoftwareStates()
{
	QStringList softwareLists = systemSoftwareListHash[systemName];
	progressBar->setFormat(tr("Checking software-states - %p%"));
	progressBar->setRange(0, treeWidgetKnownSoftware->topLevelItemCount());
	progressBar->setValue(0);

	QWidget *focusWidget = qApp->focusWidget();
	qmc2MainWindow->tabWidgetGamelist->setEnabled(false);
	qmc2MainWindow->menuBar()->setEnabled(false);
	qmc2MainWindow->toolbar->setEnabled(false);
	actionCheckSoftwareStates->setEnabled(false);

	if ( treeWidgetKnownSoftware->topLevelItemCount() > QMC2_SWLIST_COUNT_THRESHOLD ) {
		labelLoadingSoftwareLists->setText(tr("Checking software-states, please wait..."));
		labelLoadingSoftwareLists->setVisible(true);
		toolBoxSoftwareList->setVisible(false);
	}

	numSoftwareCorrect = numSoftwareIncorrect = numSoftwareMostlyCorrect = numSoftwareNotFound = numSoftwareUnknown = 0;
	updateStats();

	foreach (QString softwareList, softwareLists) {
		if ( softwareList == "NO_SOFTWARE_LIST" )
			break;

		if ( verifyProc )
			delete verifyProc;

		verifyProc = new QProcess(this);

		connect(verifyProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(verifyError(QProcess::ProcessError)));
		connect(verifyProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(verifyFinished(int, QProcess::ExitStatus)));
		connect(verifyProc, SIGNAL(readyReadStandardOutput()), this, SLOT(verifyReadyReadStandardOutput()));
		connect(verifyProc, SIGNAL(readyReadStandardError()), this, SLOT(verifyReadyReadStandardError()));
		connect(verifyProc, SIGNAL(started()), this, SLOT(verifyStarted()));

		softwareListName = softwareList;
		swStatesLastLine.clear();
		softwareListStateHash[softwareListName].clear();
		softwareListItems = treeWidgetKnownSoftware->findItems(softwareList, Qt::MatchExactly, QMC2_SWLIST_COLUMN_LIST);
		favoritesListItems = treeWidgetFavoriteSoftware->findItems(softwareList, Qt::MatchExactly, QMC2_SWLIST_COLUMN_LIST);
		searchListItems = treeWidgetSearchResults->findItems(softwareList, Qt::MatchExactly, QMC2_SWLIST_COLUMN_LIST);

		QString softwareStateCachePath = QDir::toNativeSeparators(QDir::cleanPath(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache").toString() + "/" + softwareList + ".ssc"));
		softwareStateFile.setFileName(softwareStateCachePath);
		if ( !softwareStateFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open software state cache file '%1' for writing, please check access permissions").arg(softwareStateCachePath));
		else {
			softwareStateStream.setDevice(&softwareStateFile);
			softwareStateStream << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
		}

		QString command = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString();
		QStringList args;
		args << "-verifysoftlist" << softwareList;
		QString romPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath").toString().replace("~", "$HOME");
		if ( !romPath.isEmpty() )
			args << "-rompath" << romPath;
		QString hashPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath").toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;

		if ( !qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory").toString().isEmpty() )
			verifyProc->setWorkingDirectory(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString());

		verifyProc->start(command, args);

		verifyReadingStdout = false;
		int retries = 0;
		bool started = verifyProc->waitForStarted(QMC2_PROCESS_POLL_TIME);
		while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES )
			started = verifyProc->waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);

		if ( started ) {
			bool verifyProcRunning = (verifyProc->state() == QProcess::Running);
			while ( !verifyProc->waitForFinished(QMC2_PROCESS_POLL_TIME) && verifyProcRunning ) {
				qApp->processEvents();
				verifyProcRunning = (verifyProc->state() == QProcess::Running);
			}
			verifyProc->waitForFinished();
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start emulator executable within a reasonable time frame, giving up"));
			break;
		}
	}

	labelLoadingSoftwareLists->setVisible(false);
	toolBoxSoftwareList->setVisible(true);

	qmc2MainWindow->tabWidgetGamelist->setEnabled(true);
	qmc2MainWindow->menuBar()->setEnabled(true);
	qmc2MainWindow->toolbar->setEnabled(true);
	actionCheckSoftwareStates->setEnabled(true);
	if ( focusWidget )
		focusWidget->setFocus();

	QTimer::singleShot(0, progressBar, SLOT(hide()));
}

void SoftwareList::verifyStarted()
{
	progressBar->setVisible(true);
}

void SoftwareList::verifyFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	while ( verifyReadingStdout ) {
		QTest::qWait(10);
		qApp->processEvents();
	}

	bool notFoundState = true;

	if ( (exitStatus != QProcess::NormalExit || exitCode != 0) && exitCode != 2 ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to verify the states for software-list '%1' didn't exit cleanly -- exitCode = %2, exitStatus = %3").arg(softwareListName).arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
		notFoundState = false;
	}

	for (int i = 0; i < softwareListItems.count(); i++) {
		QTreeWidgetItem *softwareItem = softwareListItems[i];
		QString softwareName = softwareItem->text(QMC2_SWLIST_COLUMN_NAME);

		QTreeWidgetItem *favoriteItem = NULL;
		foreach (QTreeWidgetItem *item, favoritesListItems) {
			if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
				favoriteItem = item;
				break;
			}
		}

		QTreeWidgetItem *searchItem = NULL;
		foreach (QTreeWidgetItem *item, searchListItems) {
			if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
				searchItem = item;
				break;
			}
		}

		QString listName = softwareItem->text(QMC2_SWLIST_COLUMN_LIST);
		if ( !softwareListStateHash[listName].contains(softwareName) ) {
			progressBar->setValue(progressBar->value() + 1);
			SoftwareItem *hierarchyItem = 0;
			QString setKey(listName + ":" + softwareName);
			if ( notFoundState ) {
				softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
				softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "N");
				if ( stateFilter->checkBoxStateFilter->isChecked() )
					softwareItem->setHidden(!stateFilter->toolButtonNotFound->isChecked());
				else
					softwareItem->setHidden(false);
				if ( softwareHierarchyItemHash.contains(setKey) )
					hierarchyItem = softwareHierarchyItemHash[setKey];
				if ( hierarchyItem ) {
					hierarchyItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
					hierarchyItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "N");
				}
				if ( favoriteItem )
					favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
				if ( searchItem )
					searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
				if ( softwareStateFile.isOpen() )
					softwareStateStream << softwareName << " N\n";
				softwareListStateHash[listName][softwareName] = 'N';
				numSoftwareNotFound++;
			} else {
				softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
				softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
				if ( stateFilter->checkBoxStateFilter->isChecked() )
					softwareItem->setHidden(!stateFilter->toolButtonUnknown->isChecked());
				else
					softwareItem->setHidden(false);
				if ( softwareHierarchyItemHash.contains(setKey) )
					hierarchyItem = softwareHierarchyItemHash[setKey];
				if ( hierarchyItem ) {
					hierarchyItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
					hierarchyItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
				}
				if ( favoriteItem )
					favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
				if ( searchItem )
					searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
				if ( softwareStateFile.isOpen() )
					softwareStateStream << softwareName << " U\n";
				softwareListStateHash[listName][softwareName] = 'U';
				numSoftwareUnknown++;
			}
		}

		if ( i % QMC2_SWLIST_CHECK_RESPONSE == 0 ) {
			updateStats();
			qApp->processEvents();
		}
	}

	updateStats();

	if ( softwareStateFile.isOpen() )
		softwareStateFile.close();

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("state info for software-list '%1': L:%2 C:%3 M:%4 I:%5 N:%6 U:%7").arg(softwareListName).arg(softwareListItems.count()).arg(numSoftwareCorrect).arg(numSoftwareMostlyCorrect).arg(numSoftwareIncorrect).arg(numSoftwareNotFound).arg(numSoftwareUnknown));

	if ( toolButtonCompatFilterToggle->isChecked() )
		on_toolButtonCompatFilterToggle_clicked(true);

	softwareListItems.clear();
}

void SoftwareList::verifyReadyReadStandardOutput()
{
	verifyReadingStdout = true;

	QString s = swStatesLastLine + verifyProc->readAllStandardOutput();
	QStringList lines = s.split("\n");
	if ( s.endsWith("\n") ) {
		swStatesLastLine.clear();
	} else {
		swStatesLastLine = lines.last();
		lines.removeLast();
	}
 
	foreach (QString line, lines) {
		line = line.simplified();
		if ( !line.isEmpty() ) {
			QStringList words = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
			if ( line.startsWith("romset") ) {
				progressBar->setValue(progressBar->value() + 1);
				QStringList romsetWords = words[1].split(":", QString::SkipEmptyParts);
				QString listName = romsetWords[0];
				QString softwareName = romsetWords[1];
				QString status = words.last();

				QTreeWidgetItem *softwareItem = NULL;
				foreach (QTreeWidgetItem *item, softwareListItems) {
					if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
						softwareItem = item;
						break;
					}
				}

				if ( !softwareItem )
					continue;

				QTreeWidgetItem *favoriteItem = NULL;
				foreach (QTreeWidgetItem *item, favoritesListItems) {
					if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
						favoriteItem = item;
						break;
					}
				}

				QTreeWidgetItem *searchItem = NULL;
				foreach (QTreeWidgetItem *item, searchListItems) {
					if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
						searchItem = item;
						break;
					}
				}

				char charStatus = 'U';
				if ( status == "good" )
					charStatus = 'C';
				else if ( status == "bad" )
					charStatus = 'I';
				else if ( status == "available" )
					charStatus = 'M';

				softwareListStateHash[listName][softwareName] = charStatus;
				SoftwareItem *hierarchyItem = 0;
				QString setKey(listName + ":" + softwareName);
				if ( softwareHierarchyItemHash.contains(setKey) )
					hierarchyItem = softwareHierarchyItemHash[setKey];

				switch ( charStatus ) {
					case 'C':
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "C");
						if ( stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!stateFilter->toolButtonCorrect->isChecked());
						else
							softwareItem->setHidden(false);
						if ( hierarchyItem ) {
							hierarchyItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
							hierarchyItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "C");
						}
						if ( favoriteItem )
							favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
						if ( searchItem )
							searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
						if ( softwareStateFile.isOpen() )
							softwareStateStream << softwareName << " C\n";
						numSoftwareCorrect++;
						break;
					case 'M':
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "M");
						if ( stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!stateFilter->toolButtonMostlyCorrect->isChecked());
						else
							softwareItem->setHidden(false);
						if ( hierarchyItem ) {
							hierarchyItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
							hierarchyItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "M");
						}
						if ( favoriteItem )
							favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
						if ( searchItem )
							searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
						if ( softwareStateFile.isOpen() )
							softwareStateStream << softwareName << " M\n";
						numSoftwareMostlyCorrect++;
						break;
					case 'I':
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "I");
						if ( stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!stateFilter->toolButtonIncorrect->isChecked());
						else
							softwareItem->setHidden(false);
						if ( hierarchyItem ) {
							hierarchyItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
							hierarchyItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "I");
						}
						if ( favoriteItem )
							favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
						if ( searchItem )
							searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
						if ( softwareStateFile.isOpen() )
							softwareStateStream << softwareName << " I\n";
						numSoftwareIncorrect++;
						break;
					case 'U':
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
						if ( stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!stateFilter->toolButtonUnknown->isChecked());
						else
							softwareItem->setHidden(false);
						if ( hierarchyItem ) {
							hierarchyItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
							hierarchyItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
						}
						if ( favoriteItem )
							favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
						if ( searchItem )
							searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
						if ( softwareStateFile.isOpen() )
							softwareStateStream << softwareName << " U\n";
						numSoftwareUnknown++;
						break;
				}
			}
		}
	}

	updateStats();

	verifyReadingStdout = false;
}

void SoftwareList::verifyReadyReadStandardError()
{
#ifdef QMC2_DEBUG
	QString data = verifyProc->readAllStandardError();
	data = data.trimmed();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::verifyReadyReadStandardError(): data = '%1'").arg(data));
#endif
}

void SoftwareList::verifyError(QProcess::ProcessError processError)
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to verify software-states caused an error -- processError = %1").arg(processError));

	progressBar->setVisible(false);
}

void SoftwareList::on_toolButtonToggleSnapnameAdjustment_clicked(bool checked)
{
	if ( checked && mountedSoftware.count() > 1 )
		comboBoxSnapnameDevice->show();
	else
		comboBoxSnapnameDevice->hide();
}

void SoftwareList::on_toolButtonSoftwareStates_toggled(bool checked)
{
	QString itemText = toolBoxSoftwareList->itemText(QMC2_SWLIST_KNOWN_SW_PAGE);
	itemText.remove(QRegExp(" - " + tr("filtered") + "$"));

	if ( checked ) {
		toolButtonSoftwareStates->setMenu(menuSoftwareStates);
		if ( isReady ) {
			if ( stateFilter->checkBoxStateFilter->isChecked() )
				qmc2SoftwareList->toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, itemText + " - " + tr("filtered"));
			else
				qmc2SoftwareList->toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, itemText);
		}
	} else {
		toolButtonSoftwareStates->setMenu(NULL);
		if ( isReady )
			qmc2SoftwareList->toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, itemText);
	}
	updateStats();

	if ( isReady )
		QTimer::singleShot(0, toolButtonReload, SLOT(animateClick()));

	qApp->processEvents();
}

void SoftwareList::on_stackedWidgetKnownSoftware_currentChanged(int index)
{
	QList<QTreeWidgetItem *> selectedItems;
	switch ( index ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE_TREE:
			treeWidgetKnownSoftwareTree->header()->restoreState(treeWidgetKnownSoftware->header()->saveState());
			if ( fullyLoaded && treeWidgetKnownSoftwareTree->topLevelItemCount() == 0 )
				loadTree();
			selectedItems = treeWidgetKnownSoftware->selectedItems();
			if ( !selectedItems.isEmpty() ) {
				QTreeWidgetItem *flatItem = selectedItems.first();
				while ( flatItem->parent() )
					flatItem = flatItem->parent();
				QString setKey(flatItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + flatItem->text(QMC2_SWLIST_COLUMN_NAME));
				if ( softwareHierarchyItemHash.contains(setKey) ) {
					SoftwareItem *treeItem = softwareHierarchyItemHash[setKey];
					selectedItems = treeWidgetKnownSoftwareTree->selectedItems();
					if ( !selectedItems.isEmpty() )
						selectedItems.first()->setSelected(false);
					treeItem->setSelected(true);
					treeWidgetKnownSoftwareTree->setCurrentItem(treeItem);
					treeWidgetKnownSoftwareTree->scrollToItem(treeItem, qmc2CursorPositioningMode);
				}
			}
			break;
		case QMC2_SWLIST_KNOWN_SW_PAGE_FLAT:
			treeWidgetKnownSoftware->header()->restoreState(treeWidgetKnownSoftwareTree->header()->saveState());
			selectedItems = treeWidgetKnownSoftwareTree->selectedItems();
			if ( !selectedItems.isEmpty() ) {
				QTreeWidgetItem *treeItem = selectedItems.first();
				while ( treeItem->whatsThis(QMC2_SWLIST_COLUMN_NAME).isEmpty() && treeItem->parent() )
					treeItem = treeItem->parent();
				QString setKey(treeItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + treeItem->text(QMC2_SWLIST_COLUMN_NAME));
				if ( softwareItemHash.contains(setKey) ) {
					SoftwareItem *flatItem = softwareItemHash[setKey];
					selectedItems = treeWidgetKnownSoftware->selectedItems();
					if ( !selectedItems.isEmpty() )
						selectedItems.first()->setSelected(false);
					flatItem->setSelected(true);
					treeWidgetKnownSoftware->setCurrentItem(flatItem);
					treeWidgetKnownSoftware->scrollToItem(flatItem, qmc2CursorPositioningMode);
				}
			}
			break;
		default:
			break;
	}
}

void SoftwareList::on_toolButtonToggleSoftwareInfo_clicked(bool checked)
{
	QTreeWidget *treeWidget = 0;
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			if ( stackedWidgetKnownSoftware->currentIndex() == QMC2_SWLIST_KNOWN_SW_PAGE_FLAT )
				treeWidget = treeWidgetKnownSoftware;
			else
				treeWidget = treeWidgetKnownSoftwareTree;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
	}

	if ( !treeWidget ) {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		return;
	}

	checked &= (treeWidget->selectedItems().count() > 0);

	if ( checked ) {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
		qmc2MainWindow->tabWidgetSoftwareDetail_updateCurrent();
		if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
	}
}

void SoftwareList::on_toolButtonCompatFilterToggle_clicked(bool checked)
{
	QStringList compatFilters = systemSoftwareFilterHash[qmc2SoftwareList->systemName];
	for (int count = 0; count < treeWidgetKnownSoftware->topLevelItemCount(); count++) {
		QTreeWidgetItem *item = treeWidgetKnownSoftware->topLevelItem(count);
		QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
		bool showItem = true;
		if ( checked ) {
			showItem = compatList.isEmpty() || compatFilters.isEmpty();
			for (int i = 0; i < compatList.count() && !showItem; i++)
				for (int j = 0; j < compatFilters.count() && !showItem; j++)
					showItem = (compatList[i] == compatFilters[j]);
		}
		if ( toolButtonSoftwareStates->isChecked() && stateFilter->checkBoxStateFilter->isChecked() ) {
			switch ( item->whatsThis(QMC2_SWLIST_COLUMN_NAME).at(0).toLatin1() ) {
				case 'C':
					item->setHidden(!stateFilter->toolButtonCorrect->isChecked() || !showItem);
					break;
				case 'M':
					item->setHidden(!stateFilter->toolButtonMostlyCorrect->isChecked() || !showItem);
					break;
				case 'I':
					item->setHidden(!stateFilter->toolButtonIncorrect->isChecked() || !showItem);
					break;
				case 'N':
					item->setHidden(!stateFilter->toolButtonNotFound->isChecked() || !showItem);
					break;
				case 'U':
				default:
					item->setHidden(!stateFilter->toolButtonUnknown->isChecked() || !showItem);
					break;
			}
			if ( item->isHidden() && item->isSelected() )
				item->setSelected(false);
		} else {
			if ( !showItem ) {
				item->setHidden(true);
				if ( item->isSelected() )
					item->setSelected(false);
			} else
				item->setHidden(false);
		}
	}
	for (int count = 0; count < treeWidgetFavoriteSoftware->topLevelItemCount(); count++) {
		QTreeWidgetItem *item = treeWidgetFavoriteSoftware->topLevelItem(count);
		QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
		bool showItem = true;
		if ( checked ) {
			showItem = compatList.isEmpty() || compatFilters.isEmpty();
			for (int i = 0; i < compatList.count() && !showItem; i++)
				for (int j = 0; j < compatFilters.count() && !showItem; j++)
					showItem = (compatList[i] == compatFilters[j]);
		}
		if ( !showItem ) {
			item->setHidden(true);
			if ( item->isSelected() )
				item->setSelected(false);
		} else
			item->setHidden(false);
	}
	for (int count = 0; count < treeWidgetSearchResults->topLevelItemCount(); count++) {
		QTreeWidgetItem *item = treeWidgetSearchResults->topLevelItem(count);
		QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
		bool showItem = true;
		if ( checked ) {
			showItem = compatList.isEmpty() || compatFilters.isEmpty();
			for (int i = 0; i < compatList.count() && !showItem; i++)
				for (int j = 0; j < compatFilters.count() && !showItem; j++)
					showItem = (compatList[i] == compatFilters[j]);
		}
		if ( !showItem ) {
			item->setHidden(true);
			if ( item->isSelected() )
				item->setSelected(false);
		} else
			item->setHidden(false);
	}
}

void SoftwareList::on_toolButtonReload_clicked(bool)
{
	save();

	numSoftwareTotal = numSoftwareCorrect = numSoftwareIncorrect = numSoftwareMostlyCorrect = numSoftwareNotFound = numSoftwareUnknown = 0;
	updateStats();

	treeWidgetKnownSoftware->clear();
	treeWidgetKnownSoftwareTree->clear();
	treeWidgetFavoriteSoftware->clear();
	treeWidgetSearchResults->clear();
	toolBoxSoftwareList->setEnabled(false);
	toolButtonAddToFavorites->setEnabled(false);
	toolButtonRemoveFromFavorites->setEnabled(false);
	toolButtonFavoritesOptions->setEnabled(false);
	toolButtonPlay->setEnabled(false);
	toolButtonPlayEmbedded->setEnabled(false);
	toolButtonExport->setEnabled(false);
	toolButtonToggleSoftwareInfo->setEnabled(false);
	toolButtonCompatFilterToggle->setEnabled(false);
	toolButtonToggleSnapnameAdjustment->setEnabled(false);
	toolButtonSoftwareStates->setEnabled(false);
	toolButtonAnalyzeSoftware->setEnabled(false);
	toolButtonRebuildSoftware->setEnabled(false);
	comboBoxDeviceConfiguration->setEnabled(false);
	comboBoxDeviceConfiguration->clear();
	comboBoxDeviceConfiguration->insertItem(0, tr("Default configuration"));
	qApp->processEvents();

	QTimer::singleShot(0, this, SLOT(load()));
}

void SoftwareList::on_toolButtonExport_clicked(bool)
{
	if ( !exporter )
		exporter = new SoftwareListExporter(this);

	exporter->show();
}

void SoftwareList::on_toolButtonAddToFavorites_clicked(bool)
{
	QList<QTreeWidgetItem *> selectedItems;

	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			if ( stackedWidgetKnownSoftware->currentIndex() == QMC2_SWLIST_KNOWN_SW_PAGE_FLAT )
				selectedItems = treeWidgetKnownSoftware->selectedItems();
			else
				selectedItems = treeWidgetKnownSoftwareTree->selectedItems();
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			selectedItems = treeWidgetSearchResults->selectedItems();
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			selectedItems = treeWidgetFavoriteSoftware->selectedItems();
			break;
	}

	QTreeWidgetItem *si = NULL;

	if ( selectedItems.count() > 0 )
		si = selectedItems.at(0);

	QStringList compatFilters = systemSoftwareFilterHash[qmc2SoftwareList->systemName];
	if ( si ) {
		while ( si->parent() ) si = si->parent();
		SoftwareItem *item = NULL;
		QList<QTreeWidgetItem *> matchedItems = treeWidgetFavoriteSoftware->findItems(si->text(QMC2_SWLIST_COLUMN_NAME), Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
		if ( matchedItems.count() > 0 )
			item = (SoftwareItem *)matchedItems.at(0);
		else {
			item = new SoftwareItem(treeWidgetFavoriteSoftware);
			SoftwareItem *subItem = new SoftwareItem(item);
			subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
		}
		if ( item ) {
			item->setText(QMC2_SWLIST_COLUMN_TITLE, si->text(QMC2_SWLIST_COLUMN_TITLE));
			item->setIcon(QMC2_SWLIST_COLUMN_TITLE, si->icon(QMC2_SWLIST_COLUMN_TITLE));
			item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, si->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
			item->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, si->whatsThis(QMC2_SWLIST_COLUMN_NAME));
			QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
			bool showItem = true;
			if ( toolButtonCompatFilterToggle->isChecked() ) {
				showItem = compatList.isEmpty() || compatFilters.isEmpty();
				for (int i = 0; i < compatList.count() && !showItem; i++)
					for (int j = 0; j < compatFilters.count() && !showItem; j++)
						showItem = (compatList[i] == compatFilters[j]);
			}
			item->setHidden(!showItem);
			item->setText(QMC2_SWLIST_COLUMN_NAME, si->text(QMC2_SWLIST_COLUMN_NAME));
			item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, si->text(QMC2_SWLIST_COLUMN_PUBLISHER));
			item->setText(QMC2_SWLIST_COLUMN_YEAR, si->text(QMC2_SWLIST_COLUMN_YEAR));
			item->setText(QMC2_SWLIST_COLUMN_PART, si->text(QMC2_SWLIST_COLUMN_PART));
			item->setText(QMC2_SWLIST_COLUMN_INTERFACE, si->text(QMC2_SWLIST_COLUMN_INTERFACE));
			item->setText(QMC2_SWLIST_COLUMN_LIST, si->text(QMC2_SWLIST_COLUMN_LIST));
			item->setText(QMC2_SWLIST_COLUMN_SUPPORTED, si->text(QMC2_SWLIST_COLUMN_SUPPORTED));
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
			if ( comboBoxDeviceConfiguration->currentIndex() > 0 )
				item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, comboBoxDeviceConfiguration->currentText());
			else
				item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, QString());
#endif
		}
	}

	actionSaveFavoritesToFile->setEnabled(treeWidgetFavoriteSoftware->topLevelItemCount() > 0);
}

void SoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool)
{
	if ( toolBoxSoftwareList->currentIndex() != QMC2_SWLIST_FAVORITES_PAGE )
		return;

	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	QTreeWidgetItem *si = NULL;

	if ( selectedItems.count() > 0 ) {
		si = selectedItems.at(0);
		while ( si->parent() ) si = si->parent();
	}

	if ( si ) {
		QTreeWidgetItem *itemToBeRemoved = treeWidgetFavoriteSoftware->takeTopLevelItem(treeWidgetFavoriteSoftware->indexOfTopLevelItem(si));
		if ( itemToBeRemoved )
			delete itemToBeRemoved;
	}

	actionSaveFavoritesToFile->setEnabled(treeWidgetFavoriteSoftware->topLevelItemCount() > 0);
}

void SoftwareList::on_toolButtonPlay_clicked(bool)
{
	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_triggered()));
}

void SoftwareList::on_toolButtonPlayEmbedded_clicked(bool)
{
	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_triggered()));
}

void SoftwareList::treeWidgetKnownSoftware_headerSectionClicked(int)
{
	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	if ( !selectedItems.isEmpty() )
		treeWidgetKnownSoftware->scrollToItem(selectedItems.first(), qmc2CursorPositioningMode);
}

void SoftwareList::treeWidgetKnownSoftwareTree_headerSectionClicked(int)
{
	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftwareTree->selectedItems();
	if ( !selectedItems.isEmpty() )
		treeWidgetKnownSoftwareTree->scrollToItem(selectedItems.first(), qmc2CursorPositioningMode);
}

void SoftwareList::treeWidgetFavoriteSoftware_headerSectionClicked(int)
{
	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	if ( !selectedItems.isEmpty() )
		treeWidgetFavoriteSoftware->scrollToItem(selectedItems.first(), qmc2CursorPositioningMode);
}

void SoftwareList::treeWidgetSearchResults_headerSectionClicked(int)
{
	QList<QTreeWidgetItem *> selectedItems = treeWidgetSearchResults->selectedItems();
	if ( !selectedItems.isEmpty() )
		treeWidgetSearchResults->scrollToItem(selectedItems.first(), qmc2CursorPositioningMode);
}

void SoftwareList::on_treeWidgetKnownSoftware_itemExpanded(QTreeWidgetItem *item)
{
	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareXml = swlDb->xml(item->text(QMC2_SWLIST_COLUMN_LIST), item->text(QMC2_SWLIST_COLUMN_NAME));
		if ( !softwareXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			xmlReader.setFeature("http://xml.org/sax/features/namespaces", false);
			xmlReader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
			treeWidgetKnownSoftware->setSortingEnabled(false);
			item->child(0)->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Searching"));
			treeWidgetKnownSoftware->viewport()->update();
			qApp->processEvents();
			if ( !xmlReader.parse(xmlInputSource) )
				if ( !xmlHandler.success )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
			treeWidgetKnownSoftware->setSortingEnabled(true);
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
	}
}

void SoftwareList::on_treeWidgetKnownSoftwareTree_itemExpanded(QTreeWidgetItem *item)
{
	// FIXME
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemExpanded(QTreeWidgetItem *item)
{
	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareXml = swlDb->xml(item->text(QMC2_SWLIST_COLUMN_LIST), item->text(QMC2_SWLIST_COLUMN_NAME));
		if ( !softwareXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			xmlReader.setFeature("http://xml.org/sax/features/namespaces", false);
			xmlReader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
			treeWidgetFavoriteSoftware->setSortingEnabled(false);
			item->child(0)->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Searching"));
			treeWidgetFavoriteSoftware->viewport()->update();
			qApp->processEvents();
			if ( !xmlReader.parse(xmlInputSource) )
				if ( !xmlHandler.success )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
			treeWidgetFavoriteSoftware->setSortingEnabled(true);
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemExpanded(QTreeWidgetItem *item)
{
	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareXml = swlDb->xml(item->text(QMC2_SWLIST_COLUMN_LIST), item->text(QMC2_SWLIST_COLUMN_NAME));
		if ( !softwareXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			xmlReader.setFeature("http://xml.org/sax/features/namespaces", false);
			xmlReader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
			treeWidgetSearchResults->setSortingEnabled(false);
			item->child(0)->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Searching"));
			treeWidgetSearchResults->viewport()->update();
			qApp->processEvents();
			if ( !xmlReader.parse(xmlInputSource) )
				if ( !xmlHandler.success )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
			treeWidgetSearchResults->setSortingEnabled(true);
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
	}
}

void SoftwareList::on_toolBoxSoftwareList_currentChanged(int index)
{
	comboBoxDeviceConfiguration->setCurrentIndex(0);
	switch ( index ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			updateMountDevices();
			on_treeWidgetKnownSoftware_itemSelectionChanged();
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			on_treeWidgetFavoriteSoftware_itemSelectionChanged();
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			updateMountDevices();
			on_treeWidgetSearchResults_itemSelectionChanged();
			break;
		default:
			break;
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_itemSelectionChanged()
{
	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	bool enable = !selectedItems.isEmpty();
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
	if ( enable && qmc2SoftwareSnap ) {
		SoftwareItem *item = (SoftwareItem *)selectedItems[0];
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
		if ( toolButtonToggleSoftwareInfo->isChecked() ) {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
				qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
		} else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		}
		currentItem = item;
		while ( currentItem->parent() )
			currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		cancelSoftwareSnap();
		currentItem = NULL;
	}
}

void SoftwareList::on_treeWidgetKnownSoftwareTree_itemSelectionChanged()
{
	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftwareTree->selectedItems();
	bool enable = !selectedItems.isEmpty();
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
	if ( enable && qmc2SoftwareSnap ) {
		SoftwareItem *item = (SoftwareItem *)selectedItems[0];
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
		if ( toolButtonToggleSoftwareInfo->isChecked() ) {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
				qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
		} else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		}
		currentItem = item;
		while ( currentItem->whatsThis(QMC2_SWLIST_COLUMN_NAME).isEmpty() && currentItem->parent() )
			currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		cancelSoftwareSnap();
		currentItem = NULL;
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemSelectionChanged()
{
	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	bool enable = (selectedItems.count() > 0);
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonRemoveFromFavorites->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
	if ( enable && qmc2SoftwareSnap ) {
		SoftwareItem *item = (SoftwareItem *)selectedItems[0];
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
		if ( toolButtonToggleSoftwareInfo->isChecked() ) {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
				qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
		} else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		}
		currentItem = item;
		while ( currentItem->parent() )
			currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		cancelSoftwareSnap();
		currentItem = NULL;
	}
	if ( enable ) {
		QTreeWidgetItem *item = selectedItems[0];
		while ( item->parent() ) item = item->parent();
		QString s = item->text(QMC2_SWLIST_COLUMN_DEVICECFG);
		if ( !s.isEmpty() ) {
			int index = comboBoxDeviceConfiguration->findText(s, Qt::MatchExactly | Qt::MatchCaseSensitive);
			if ( index > 0 )
				comboBoxDeviceConfiguration->setCurrentIndex(index);
			else
				comboBoxDeviceConfiguration->setCurrentIndex(0);
		} else
			comboBoxDeviceConfiguration->setCurrentIndex(0);
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemSelectionChanged()
{
	QList<QTreeWidgetItem *> selectedItems = treeWidgetSearchResults->selectedItems();
	bool enable = (selectedItems.count() > 0);
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
	toolButtonRemoveFromFavorites->setEnabled(false);
	if ( selectedItems.count() > 0 && qmc2SoftwareSnap ) {
		SoftwareItem *item = (SoftwareItem *)selectedItems[0];
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
		if ( toolButtonToggleSoftwareInfo->isChecked() ) {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
				qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
		} else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		}
		currentItem = item;
		while ( currentItem->parent() )
			currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		cancelSoftwareSnap();
		currentItem = NULL;
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p)
{
	cancelSoftwareSnap();

	QTreeWidgetItem *item = treeWidgetKnownSoftware->itemAt(p);
	if ( !item )
		return;

	treeWidgetKnownSoftware->setItemSelected(item, true);
	actionAddToFavorites->setVisible(true);
	actionRemoveFromFavorites->setVisible(false);
	updateRebuildSoftwareMenuVisibility();
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetKnownSoftware->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::on_treeWidgetKnownSoftwareTree_customContextMenuRequested(const QPoint &p)
{
	cancelSoftwareSnap();

	QTreeWidgetItem *item = treeWidgetKnownSoftwareTree->itemAt(p);
	if ( !item )
		return;

	treeWidgetKnownSoftwareTree->setItemSelected(item, true);
	actionAddToFavorites->setVisible(true);
	actionRemoveFromFavorites->setVisible(false);
	updateRebuildSoftwareMenuVisibility();
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &p)
{
	cancelSoftwareSnap();

	QTreeWidgetItem *item = treeWidgetFavoriteSoftware->itemAt(p);
	if ( !item )
		return;

	treeWidgetFavoriteSoftware->setItemSelected(item, true);
	actionAddToFavorites->setVisible(false);
	actionRemoveFromFavorites->setVisible(true);
	updateRebuildSoftwareMenuVisibility();
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetFavoriteSoftware->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &p)
{
	cancelSoftwareSnap();

	QTreeWidgetItem *item = treeWidgetSearchResults->itemAt(p);
	if ( !item )
		return;

	treeWidgetSearchResults->setItemSelected(item, true);
	actionAddToFavorites->setVisible(true);
	actionRemoveFromFavorites->setVisible(false);
	updateRebuildSoftwareMenuVisibility();
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSearchResults->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::cancelSoftwareSnap()
{
	snapForced = false;
	snapTimer.stop();
	if ( qmc2SoftwareSnap ) {
		qmc2SoftwareSnap->myItem = NULL;
		qmc2SoftwareSnap->hide();
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_itemEntered(QTreeWidgetItem *item, int)
{
	if ( item == enteredItem )
		return;

	enteredItem = item;

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
				qmc2SoftwareSnap->myItem = (SoftwareItem *)item;
				snapTimer.start(QMC2_SWSNAP_DELAY);
			}
		}
	}
}

void SoftwareList::on_treeWidgetKnownSoftwareTree_itemEntered(QTreeWidgetItem *item, int)
{
	if ( item == enteredItem )
		return;

	enteredItem = item;

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
				qmc2SoftwareSnap->myItem = (SoftwareItem *)item;
				snapTimer.start(QMC2_SWSNAP_DELAY);
			}
		}
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemEntered(QTreeWidgetItem *item, int)
{
	if ( item == enteredItem )
		return;

	enteredItem = item;

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
				qmc2SoftwareSnap->myItem = (SoftwareItem *)item;
				snapTimer.start(QMC2_SWSNAP_DELAY);
			}
		}
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemEntered(QTreeWidgetItem *item, int)
{
	if ( item == enteredItem )
		return;

	enteredItem = item;

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
				qmc2SoftwareSnap->myItem = (SoftwareItem *)item;
				snapTimer.start(QMC2_SWSNAP_DELAY);
			}
		}
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_itemActivated(QTreeWidgetItem *item, int)
{
	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, this, SLOT(playEmbeddedActivated()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, this, SLOT(playActivated()));
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void SoftwareList::on_treeWidgetKnownSoftware_itemDoubleClicked(QTreeWidgetItem *item, int)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_treeWidgetKnownSoftwareTree_itemActivated(QTreeWidgetItem *, int)
{
	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, this, SLOT(playEmbeddedActivated()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, this, SLOT(playActivated()));
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void SoftwareList::on_treeWidgetKnownSoftwareTree_itemDoubleClicked(QTreeWidgetItem *item, int)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *item, int)
{
	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, this, SLOT(playEmbeddedActivated()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, this, SLOT(playActivated()));
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemDoubleClicked(QTreeWidgetItem *item, int)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *item, int)
{
	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, this, SLOT(playEmbeddedActivated()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, this, SLOT(playActivated()));
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void SoftwareList::on_treeWidgetSearchResults_itemDoubleClicked(QTreeWidgetItem *item, int)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_comboBoxSearch_editTextChanged(const QString &)
{
	if ( searchActive )
		stopSearch = true;

	searchTimer.start(QMC2_SEARCH_DELAY);
}

void SoftwareList::comboBoxSearch_editTextChanged_delayed()
{
	static QString lastSearchText;
	static bool lastNegatedMatch = false;

	searchTimer.stop();

	if ( isLoading || qmc2CleaningUp )
		return;

	if ( searchActive ) {
		stopSearch = true;
		searchTimer.start(QMC2_SEARCH_DELAY/10);
		return;
	} else
		stopSearch = false;

	QString pattern = comboBoxSearch->currentText();

	if ( pattern.isEmpty() ) {
		treeWidgetSearchResults->clear();
		lastSearchText.clear();
		lastNegatedMatch = negatedMatch;
		return;
	} else if ( treeWidgetSearchResults->topLevelItemCount() == 0 )
		lastSearchText.clear();

	if ( pattern == lastSearchText && lastNegatedMatch == negatedMatch )
		return;

	QString patternCopy = pattern;

	// easy pattern match
	int pos = 0;
	QRegExp rxAsterisk("(\\*)");
	while ( (pos = rxAsterisk.indexIn(pattern, pos)) != -1 ) {
		int matchedLength = rxAsterisk.matchedLength();
		if ( pos > 0 ) {
			if ( pattern[pos - 1] != '\\' ) {
				pattern.replace(pos, 1, ".*");
				matchedLength = 2;
			}
		} else {
			pattern.replace(pos, 1, ".*");
			matchedLength = 2;
		}
		pos += matchedLength;
	}
	pos = 0;
	QRegExp rxQuestionMark("(\\?)");
	while ( (pos = rxQuestionMark.indexIn(pattern, pos)) != -1 ) {
		if ( pos > 0 ) {
			if ( pattern[pos - 1] != '\\' )
				pattern.replace(pos, 1, ".");
		} else
			pattern.replace(pos, 1, ".");
		pos += rxQuestionMark.matchedLength();
	}
	pattern.replace(' ', ".* .*").replace(".*^", "").replace("$.*", "");

	treeWidgetSearchResults->clear();

	QRegExp patternRx = QRegExp(pattern, Qt::CaseInsensitive, QRegExp::RegExp2);
	if ( !patternRx.isValid() ) {
		lastSearchText.clear();
		lastNegatedMatch = negatedMatch;
		return;
	}

	searchActive = true;
	lastSearchText = patternCopy;

	progressBarSearch->setVisible(true);
	progressBarSearch->setRange(0, treeWidgetKnownSoftware->topLevelItemCount());
	progressBarSearch->setValue(0);

	QList<QTreeWidgetItem *> matches;
        QList<SoftwareItem *> itemList;
	QList<SoftwareItem *> hideList;
	QStringList compatFilters = systemSoftwareFilterHash[systemName];
	for (int i = 0; i < treeWidgetKnownSoftware->topLevelItemCount() && !stopSearch && !qmc2CleaningUp; i++) {
		QTreeWidgetItem *item = treeWidgetKnownSoftware->topLevelItem(i);
		QString itemText = item->text(QMC2_SWLIST_COLUMN_TITLE);
		QString itemName = item->text(QMC2_SWLIST_COLUMN_NAME);
		bool matched = itemText.indexOf(patternRx) > -1 || itemName.indexOf(patternRx) > -1;
		if ( negatedMatch )
			matched = !matched;
		if ( matched ) {
			matches << item;
			SoftwareItem *newItem = new SoftwareItem((QTreeWidget *)0);
			SoftwareItem *subItem = new SoftwareItem(newItem);
			subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
			newItem->setText(QMC2_SWLIST_COLUMN_TITLE, item->text(QMC2_SWLIST_COLUMN_TITLE));
			newItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, item->icon(QMC2_SWLIST_COLUMN_TITLE));
			newItem->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, item->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
			newItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, item->whatsThis(QMC2_SWLIST_COLUMN_NAME));
			newItem->setText(QMC2_SWLIST_COLUMN_NAME, item->text(QMC2_SWLIST_COLUMN_NAME));
			newItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, item->text(QMC2_SWLIST_COLUMN_PUBLISHER));
			newItem->setText(QMC2_SWLIST_COLUMN_YEAR, item->text(QMC2_SWLIST_COLUMN_YEAR));
			newItem->setText(QMC2_SWLIST_COLUMN_PART, item->text(QMC2_SWLIST_COLUMN_PART));
			newItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, item->text(QMC2_SWLIST_COLUMN_INTERFACE));
			newItem->setText(QMC2_SWLIST_COLUMN_LIST, item->text(QMC2_SWLIST_COLUMN_LIST));
			newItem->setText(QMC2_SWLIST_COLUMN_SUPPORTED, item->text(QMC2_SWLIST_COLUMN_SUPPORTED));
			itemList << newItem;
			bool showItem = true;
			if ( qmc2SoftwareList->toolButtonCompatFilterToggle->isChecked() ) {
				QStringList compatList = newItem->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
				showItem = compatList.isEmpty() || compatFilters.isEmpty();
				for (int j = 0; j < compatList.count() && !showItem; j++)
					for (int k = 0; k < compatFilters.count() && !showItem; k++)
						showItem = (compatList[j] == compatFilters[k]);
			}
			if ( !showItem )
				hideList << newItem;
		}
		progressBarSearch->setValue(progressBarSearch->value() + 1);
		if ( i % QMC2_SEARCH_RESULT_UPDATE == 0 ) {
			treeWidgetSearchResults->setUpdatesEnabled(false);
			foreach (SoftwareItem *item, itemList)
				treeWidgetSearchResults->addTopLevelItem(item);
			foreach (SoftwareItem *item, hideList)
				item->setHidden(true);
			itemList.clear();
			hideList.clear();
			treeWidgetSearchResults->setUpdatesEnabled(true);
			qApp->processEvents();
		}
	}

	if ( !stopSearch && !qmc2CleaningUp ) {
		treeWidgetSearchResults->setUpdatesEnabled(false);
		foreach (SoftwareItem *item, itemList)
			treeWidgetSearchResults->addTopLevelItem(item);
		foreach (SoftwareItem *item, hideList)
			item->setHidden(true);
		itemList.clear();
		hideList.clear();
		treeWidgetSearchResults->setUpdatesEnabled(true);
		qApp->processEvents();
	} else
		lastSearchText.clear();

	lastNegatedMatch = negatedMatch;

	progressBarSearch->setVisible(false);
	progressBarSearch->reset();

	if ( autoSelectSearchItem ) {
  		treeWidgetSearchResults->setFocus();
		if ( treeWidgetSearchResults->currentItem() )
			treeWidgetSearchResults->currentItem()->setSelected(true);
	}

	autoSelectSearchItem = false;

	searchActive = false;
}

void SoftwareList::on_comboBoxSearch_activated(const QString &pattern)
{
	autoSelectSearchItem = true;
	comboBoxSearch_editTextChanged_delayed();
}

QStringList &SoftwareList::arguments(QStringList *softwareLists, QStringList *softwareNames)
{
	static QStringList swlArgs;

	swlArgs.clear();

	// arguments to start a software list entry
	QTreeWidget *treeWidget;
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
		case QMC2_SWLIST_KNOWN_SW_PAGE:
		default:
			if ( stackedWidgetKnownSoftware->currentIndex() == QMC2_SWLIST_KNOWN_SW_PAGE_FLAT )
				treeWidget = treeWidgetKnownSoftware;
			else
				treeWidget = treeWidgetKnownSoftwareTree;
			break;
	}

#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
	// optionally add arguments for the selected device configuration
	QString devConfigName = comboBoxDeviceConfiguration->currentText();
	if ( devConfigName != tr("Default configuration") ) {
		qmc2Config->beginGroup(QMC2_EMULATOR_PREFIX + QString("Configuration/Devices/%1/%2").arg(systemName).arg(devConfigName));
		QStringList instances = qmc2Config->value("Instances").toStringList();
		QStringList files = qmc2Config->value("Files").toStringList();
		QStringList slotNames = qmc2Config->value("Slots").toStringList();
		QStringList slotOptions = qmc2Config->value("SlotOptions").toStringList();
		QStringList slotBIOSs = qmc2Config->value("SlotBIOSs").toStringList();
		qmc2Config->endGroup();
		for (int i = 0; i < slotNames.count(); i++) {
			if ( !slotOptions[i].isEmpty() ) {
				QString slotOpt = slotOptions[i];
				if ( !slotBIOSs[i].isEmpty() )
					slotOpt += ",bios=" + slotBIOSs[i];
				swlArgs << QString("-%1").arg(slotNames[i]) << slotOpt;
			}
		}
		for (int i = 0; i < instances.count(); i++) {
#if defined(QMC2_OS_WIN)
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace('/', '\\');
#else
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace("~", "$HOME");
#endif
		}
	}
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();

	QString snapnameList, snapnameSoftware;
	if ( selectedItems.count() > 0 ) {
		QTreeWidgetItemIterator it(treeWidget);
		QStringList manualMounts;
		// FIXME
		if ( qmc2SoftwareList->viewTree() )
			autoMounted = true;
		// FIXME
		if ( !autoMounted ) {
			// manually mounted
			while ( *it ) {
				QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
				if ( comboBox ) {
					if ( comboBox->currentIndex() > QMC2_SWLIST_MSEL_DONT_MOUNT ) {
						if ( snapnameList.isEmpty() ) {
							QTreeWidgetItem *item = *it;
							while ( item->parent() ) item = item->parent();
							snapnameList = item->text(QMC2_SWLIST_COLUMN_LIST);
							snapnameSoftware = item->text(QMC2_SWLIST_COLUMN_NAME);
							if ( comboBoxSnapnameDevice->isVisible() ) {
								if ( snapnameList + ":" + snapnameSoftware != comboBoxSnapnameDevice->currentText() ) {
									snapnameList.clear();
									snapnameSoftware.clear();
								}
							}
						}
						swlArgs << QString("-%1").arg(comboBox->currentText());
						QTreeWidgetItem *item = *it;
						while ( item->parent() ) item = item->parent();
						swlArgs << QString("%1:%2:%3").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)).arg(item->text(QMC2_SWLIST_COLUMN_PART));
						if ( softwareLists )
							*softwareLists << item->text(QMC2_SWLIST_COLUMN_LIST);
						if ( softwareNames )
							*softwareNames << item->text(QMC2_SWLIST_COLUMN_NAME);
					}
				}
				it++;
			}
		} else {
			// automatically mounted
			QTreeWidgetItem *item = selectedItems[0];
			if ( qmc2SoftwareList->viewTree() ) {
				while ( item->whatsThis(QMC2_SWLIST_COLUMN_NAME).isEmpty() && item->parent() )
					item = item->parent();
			} else {
				while ( item->parent() )
					item = item->parent();
			}
			snapnameList = item->text(QMC2_SWLIST_COLUMN_LIST);
			snapnameSoftware = item->text(QMC2_SWLIST_COLUMN_NAME);
			QStringList interfaces = item->text(QMC2_SWLIST_COLUMN_INTERFACE).split(",");
			QStringList parts = item->text(QMC2_SWLIST_COLUMN_PART).split(",");
			successfulLookups.clear();
			for (int i = 0; i < parts.count(); i++) {
				QString mountDev = lookupMountDevice(parts[i], interfaces[i]);
				if ( !mountDev.isEmpty() ) {
					swlArgs << QString("-%1").arg(mountDev);
					swlArgs << QString("%1:%2:%3").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)).arg(parts[i]);
				}
			}
			if ( softwareLists )
				*softwareLists << item->text(QMC2_SWLIST_COLUMN_LIST);
			if ( softwareNames )
				*softwareNames << item->text(QMC2_SWLIST_COLUMN_NAME);
		}
	}

	if ( toolButtonToggleSnapnameAdjustment->isChecked() && !snapnameList.isEmpty() ) {
		QString snapnamePattern = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SnapnamePattern", "soft-lists/$SOFTWARE_LIST$/$SOFTWARE_NAME$").toString();
		snapnamePattern.replace("$SOFTWARE_LIST$", snapnameList).replace("$SOFTWARE_NAME$", snapnameSoftware);
		swlArgs.prepend(snapnamePattern);
		swlArgs.prepend("-snapname");
	}

	return swlArgs;
}

void SoftwareList::treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &p)
{
	menuKnownSoftwareHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetKnownSoftware->header()->viewport()->mapToGlobal(p), menuKnownSoftwareHeader));
	menuKnownSoftwareHeader->show();
}

void SoftwareList::treeWidgetKnownSoftwareTreeHeader_customContextMenuRequested(const QPoint &p)
{
	menuKnownSoftwareHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetKnownSoftwareTree->header()->viewport()->mapToGlobal(p), menuKnownSoftwareHeader));
	menuKnownSoftwareHeader->show();
}

void SoftwareList::actionKnownSoftwareHeader_triggered()
{
	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetKnownSoftware->columnCount(); i++)
		if ( !treeWidgetKnownSoftware->isColumnHidden(i) )
			visibleColumns++;
	if ( action ) {
		if ( action->data().toInt() == QMC2_SWLIST_RESET ) {
			for (int i = 0; i < treeWidgetKnownSoftware->columnCount(); i++) {
				treeWidgetKnownSoftware->setColumnHidden(i, false);
				treeWidgetKnownSoftwareTree->setColumnHidden(i, false);
			}
			treeWidgetKnownSoftware->header()->resizeSections(QHeaderView::Stretch);
			treeWidgetKnownSoftwareTree->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuKnownSoftwareHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
		bool visibility = true;
		if ( action->isChecked() ) {
			treeWidgetKnownSoftware->setColumnHidden(action->data().toInt(), false);
			treeWidgetKnownSoftwareTree->setColumnHidden(action->data().toInt(), false);
		}
		else if ( visibleColumns > 1 ) {
			treeWidgetKnownSoftware->setColumnHidden(action->data().toInt(), true);
			treeWidgetKnownSoftwareTree->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void SoftwareList::treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &p)
{
	menuFavoriteSoftwareHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetFavoriteSoftware->header()->viewport()->mapToGlobal(p), menuFavoriteSoftwareHeader));
	menuFavoriteSoftwareHeader->show();
}

void SoftwareList::actionFavoriteSoftwareHeader_triggered()
{
	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetFavoriteSoftware->columnCount(); i++) if ( !treeWidgetFavoriteSoftware->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		if ( action->data().toInt() == QMC2_SWLIST_RESET ) {
			for (int i = 0; i < treeWidgetFavoriteSoftware->columnCount(); i++) treeWidgetFavoriteSoftware->setColumnHidden(i, false);
			treeWidgetFavoriteSoftware->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuFavoriteSoftwareHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetFavoriteSoftware->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetFavoriteSoftware->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void SoftwareList::treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &p)
{
	menuSearchResultsHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSearchResults->header()->viewport()->mapToGlobal(p), menuSearchResultsHeader));
	menuSearchResultsHeader->show();
}

void SoftwareList::actionSearchResultsHeader_triggered()
{
	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetSearchResults->columnCount(); i++) if ( !treeWidgetSearchResults->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		if ( action->data().toInt() == QMC2_SWLIST_RESET ) {
			for (int i = 0; i < treeWidgetSearchResults->columnCount(); i++) treeWidgetSearchResults->setColumnHidden(i, false);
			treeWidgetSearchResults->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuSearchResultsHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetSearchResults->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetSearchResults->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void SoftwareList::checkMountDeviceSelection()
{
	QComboBox *comboBoxSender = (QComboBox *)sender();

	QTreeWidget *treeWidget;
	switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			treeWidget = treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
	}

	QString mountDevice = comboBoxSender->currentText();
	QTreeWidgetItemIterator it(treeWidget);

	if ( mountDevice == QObject::tr("Auto mount") ) {
		while ( *it ) {
			if ( !(*it)->parent() ) successfulLookups.clear();
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				comboBox->blockSignals(true);
				comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_AUTO_MOUNT); // => auto mount
				comboBox->blockSignals(false);
				QString itemMountDev = lookupMountDevice((*it)->text(QMC2_SWLIST_COLUMN_PART), (*it)->text(QMC2_SWLIST_COLUMN_INTERFACE));
				if ( itemMountDev.isEmpty() )
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				else
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + itemMountDev);
			}
			it++;
		}
		autoMounted = true;
		mountedSoftware.clear();
	} else if ( mountDevice == QObject::tr("Don't mount") ) {
		while ( *it ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				QTreeWidgetItem *pItem = *it;
				while ( pItem->parent() ) pItem = pItem->parent();
				if ( comboBox == comboBoxSender ) {
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
					mountedSoftware.removeAll(pItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + pItem->text(QMC2_SWLIST_COLUMN_NAME));
				} else if ( comboBox->currentIndex() == QMC2_SWLIST_MSEL_AUTO_MOUNT ) {
					comboBox->blockSignals(true);
					comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // => don't mount
					comboBox->blockSignals(false);
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
					mountedSoftware.removeAll(pItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + pItem->text(QMC2_SWLIST_COLUMN_NAME));
				}
			}
			it++;
		}
		autoMounted = false;
	} else {
		while ( *it ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				QTreeWidgetItem *pItem = *it;
				while ( pItem->parent() ) pItem = pItem->parent();
				if ( comboBox != comboBoxSender ) {
					if ( comboBox->currentText() == mountDevice || comboBox->currentText() == QObject::tr("Auto mount") ) {
						comboBox->blockSignals(true);
						comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // => don't mount
						comboBox->blockSignals(false);
						(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
						mountedSoftware.removeAll(pItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + pItem->text(QMC2_SWLIST_COLUMN_NAME));
					}
				} else {
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + mountDevice);
					mountedSoftware << pItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + pItem->text(QMC2_SWLIST_COLUMN_NAME);
				}
			}
			it++;
		}
		autoMounted = false;
	}

	if ( toolButtonToggleSnapnameAdjustment->isChecked() ) {
		if ( !autoMounted && mountedSoftware.count() > 1 ) {
			comboBoxSnapnameDevice->setUpdatesEnabled(false);
			comboBoxSnapnameDevice->clear();
			qSort(mountedSoftware);
			comboBoxSnapnameDevice->addItems(mountedSoftware);
			comboBoxSnapnameDevice->setUpdatesEnabled(true);
			comboBoxSnapnameDevice->show();
		} else
			comboBoxSnapnameDevice->hide();
	} else
		comboBoxSnapnameDevice->hide();
}

void SoftwareList::loadFavoritesFromFile()
{
	QString proposedName = systemName + ".fav";

	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath") )
		proposedName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath").toString());

	QString filePath = QFileDialog::getOpenFileName(this, tr("Choose file to merge favorites from"), proposedName, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);

	if ( !filePath.isEmpty() ) {
		QFileInfo fiFilePath(filePath);
		QString storagePath = fiFilePath.absolutePath();
		if ( !storagePath.endsWith("/") ) storagePath.append("/");
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath", storagePath);

		// import software-list favorites
		QFile favoritesFile(filePath);
		QStringList compatFilters = systemSoftwareFilterHash[systemName];
		if ( favoritesFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading software-favorites for '%1' from '%2'").arg(systemName).arg(filePath));
			QTextStream ts(&favoritesFile);
			int lineCounter = 0;
			while ( !ts.atEnd() ){
				QString line = ts.readLine().trimmed();
				lineCounter++;
				if ( !line.startsWith("#") && !line.isEmpty()) {
					QStringList words = line.split("\t");
					if ( words.count() > 1 ) {
						QString listName = words[0];
						QString entryName = words[1];
						QList<QTreeWidgetItem *> matchedItems = treeWidgetFavoriteSoftware->findItems(entryName, Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
						if ( matchedItems.count() <= 0 ) {
							matchedItems = treeWidgetKnownSoftware->findItems(entryName, Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
							if ( matchedItems.count() > 0 ) {
								SoftwareItem *knowSoftwareItem = (SoftwareItem *)matchedItems.at(0);
								SoftwareItem *item = new SoftwareItem(treeWidgetFavoriteSoftware);
								SoftwareItem *subItem = new SoftwareItem(item);
								subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
								item->setText(QMC2_SWLIST_COLUMN_TITLE, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_TITLE));
								item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, knowSoftwareItem->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
								item->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, knowSoftwareItem->whatsThis(QMC2_SWLIST_COLUMN_NAME));
								bool showItem = true;
								if ( toolButtonCompatFilterToggle->isChecked() ) {
									QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
									showItem = compatList.isEmpty() || compatFilters.isEmpty();
									for (int i = 0; i < compatList.count() && !showItem; i++)
										for (int j = 0; j < compatFilters.count() && !showItem; j++)
											showItem = (compatList[i] == compatFilters[j]);
								}
								item->setHidden(!showItem);
								item->setText(QMC2_SWLIST_COLUMN_NAME, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_NAME));
								item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
								item->setText(QMC2_SWLIST_COLUMN_YEAR, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_YEAR));
								item->setText(QMC2_SWLIST_COLUMN_PART, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_PART));
								item->setText(QMC2_SWLIST_COLUMN_INTERFACE, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
								item->setText(QMC2_SWLIST_COLUMN_LIST, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_LIST));
								item->setText(QMC2_SWLIST_COLUMN_SUPPORTED, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
								if ( words.count() > 2 )
									item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, words[2]);
#endif
								qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("entry '%1:%2' successfully imported").arg(listName).arg(entryName));
							} else
								qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: entry '%1:%2' cannot be associated with any known software for this system (line %3 ignored)").arg(listName).arg(entryName).arg(lineCounter));
						} else
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: a favorite entry for '%1:%2' already exists (line %3 ignored)").arg(listName).arg(entryName).arg(lineCounter));
					} else
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: syntax error on line %1 (ignored)").arg(lineCounter));
				}
			}
			favoritesFile.close();
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading software-favorites for '%1' from '%2')").arg(systemName).arg(filePath));
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open '%1' for reading, please check permissions").arg(filePath));
	}
}

void SoftwareList::saveFavoritesToFile()
{
	QString proposedName = systemName + ".fav";

	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath") )
		proposedName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath").toString());

	QString filePath = QFileDialog::getSaveFileName(this, tr("Choose file to store favorites to"), proposedName, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);

	if ( !filePath.isEmpty() ) {
		QFileInfo fiFilePath(filePath);
		QString storagePath = fiFilePath.absolutePath();
		if ( !storagePath.endsWith("/") ) storagePath.append("/");
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath", storagePath);

		// export software-list favorites
		QFile favoritesFile(filePath);
		if ( favoritesFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving software-favorites for '%1' to '%2'").arg(systemName).arg(filePath));
			QTextStream ts(&favoritesFile);
#if defined(QMC2_EMUTYPE_MESS)
			ts << QString("# MESS software-list favorites export for driver '%1'\n").arg(systemName);
			ts << QString("# Format: <list-name><TAB><entry-name>[<TAB><additional-device-configuration>]\n");
#elif defined(QMC2_EMUTYPE_MAME)
			ts << QString("# MAME software-list favorites export for driver '%1'\n").arg(systemName);
			ts << QString("# Format: <list-name><TAB><entry-name>\n");
#elif defined(QMC2_EMUTYPE_UME)
			ts << QString("# UME software-list favorites export for driver '%1'\n").arg(systemName);
			ts << QString("# Format: <list-name><TAB><entry-name>[<TAB><additional-device-configuration>]\n");
#endif
			for (int i = 0; i < treeWidgetFavoriteSoftware->topLevelItemCount(); i++) {
				QTreeWidgetItem *item = treeWidgetFavoriteSoftware->topLevelItem(i);
				if ( item ) {
					ts << item->text(QMC2_SWLIST_COLUMN_LIST) << "\t" << item->text(QMC2_SWLIST_COLUMN_NAME);
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
					if ( !item->text(QMC2_SWLIST_COLUMN_DEVICECFG).isEmpty() )
						ts << "\t" << item->text(QMC2_SWLIST_COLUMN_DEVICECFG);
#endif
					ts << "\n";
				}

			}
			favoritesFile.close();
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving software-favorites for '%1' to '%2')").arg(systemName).arg(filePath));
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open '%1' for writing, please check permissions").arg(filePath));
	}
}

QString SoftwareList::softwareStatus(QString listName, QString softwareName, bool translated)
{
	if ( softwareListStateHash.contains(listName) ) {
		if ( softwareListStateHash[listName].contains(softwareName) ) {
			switch ( softwareListStateHash[listName][softwareName] ) {
				case 'C':
					if ( translated )
						return tr("correct");
					else
						return "correct";
					break;
				case 'M':
					if ( translated )
						return tr("mostly correct");
					else
						return "mostly correct";
					break;
				case 'I':
					if ( translated )
						return tr("incorrect");
					else
						return "incorrect";
					break;
				case 'N':
					if ( translated )
						return tr("not found");
					else
						return "not found";
					break;
				case 'U':
				default:
					if ( translated )
						return tr("unknown");
					else
						return "unknown";
					break;
			}
		} else {
			if ( translated )
				return tr("unknown");
			else
				return "unknown";
		}
	} else {
		if ( translated )
			return tr("unknown");
		else
			return "unknown";
	}
}

QString SoftwareList::status(SoftwareListXmlHandler *handler)
{
	QLocale locale;
	QString statusString = "<b>";
	if ( handler ) {
		statusString += "<font color=black>" + tr("L:") + locale.toString(numSoftwareTotal + handler->numTotal) + "</font> ";
		if ( toolButtonSoftwareStates->isChecked() ) {
			statusString += "<font color=\"#00cc00\">" + tr("C:") + locale.toString(numSoftwareCorrect + handler->numCorrect) + "</font> ";
			statusString += "<font color=\"#799632\">" + tr("M:") + locale.toString(numSoftwareMostlyCorrect + handler->numMostlyCorrect) + "</font> ";
			statusString += "<font color=\"#f90000\">" + tr("I:") + locale.toString(numSoftwareIncorrect + handler->numIncorrect) + "</font> ";
			statusString += "<font color=\"#7f7f7f\">" + tr("N:") + locale.toString(numSoftwareNotFound + handler->numNotFound) + "</font> ";
			statusString += "<font color=\"#0000f9\">" + tr("U:") + locale.toString(numSoftwareUnknown + handler->numUnknown) + "</font> ";
		}
	} else {
		statusString += "<font color=black>" + tr("L:") + locale.toString(numSoftwareTotal) + "</font> ";
		if ( toolButtonSoftwareStates->isChecked() ) {
			statusString += "<font color=\"#00cc00\">" + tr("C:") + locale.toString(numSoftwareCorrect) + "</font> ";
			statusString += "<font color=\"#799632\">" + tr("M:") + locale.toString(numSoftwareMostlyCorrect) + "</font> ";
			statusString += "<font color=\"#f90000\">" + tr("I:") + locale.toString(numSoftwareIncorrect) + "</font> ";
			statusString += "<font color=\"#7f7f7f\">" + tr("N:") + locale.toString(numSoftwareNotFound) + "</font> ";
			statusString += "<font color=\"#0000f9\">" + tr("U:") + locale.toString(numSoftwareUnknown) + "</font> ";
		}
	}
	statusString += "</b>";
	return statusString;
}

void SoftwareList::updateStats(SoftwareListXmlHandler *handler)
{
	labelSoftwareListStats->setText(status(handler));
}

void SoftwareList::analyzeSoftware()
{
	QTreeWidget *treeWidget;
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			treeWidget = treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
	}
	QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();
	if ( !selectedItems.isEmpty() ) {
		QTreeWidgetItem *item = selectedItems[0];
		while ( item->parent() )
			item = item->parent();
		if ( !qmc2SoftwareROMAlyzer )
			qmc2SoftwareROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SOFTWARE);
		if ( !qmc2SoftwareROMAlyzer->active() ) {
			qmc2SoftwareROMAlyzer->lineEditSoftwareLists->setText(item->text(QMC2_SWLIST_COLUMN_LIST));
			qmc2SoftwareROMAlyzer->lineEditSets->setText(item->text(QMC2_SWLIST_COLUMN_NAME));
		}
		if ( qmc2SoftwareROMAlyzer->isHidden() )
			qmc2SoftwareROMAlyzer->show();
		else if ( qmc2SoftwareROMAlyzer->isMinimized() )
			qmc2SoftwareROMAlyzer->showNormal();
		if ( qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabReport && qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabLog )
			qmc2SoftwareROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SoftwareROMAlyzer->tabReport);
		QTimer::singleShot(0, qmc2SoftwareROMAlyzer, SLOT(raise()));
		QTimer::singleShot(0, qmc2SoftwareROMAlyzer->pushButtonAnalyze, SLOT(animateClick()));
	}
}

void SoftwareList::analyzeSoftwareList()
{
	QTreeWidget *treeWidget;
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			treeWidget = treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
	}
	QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();
	if ( !selectedItems.isEmpty() ) {
		QTreeWidgetItem *item = selectedItems[0];
		while ( item->parent() )
			item = item->parent();
		if ( !qmc2SoftwareROMAlyzer )
			qmc2SoftwareROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SOFTWARE);
		if ( !qmc2SoftwareROMAlyzer->active() ) {
			qmc2SoftwareROMAlyzer->lineEditSoftwareLists->setText(item->text(QMC2_SWLIST_COLUMN_LIST));
			qmc2SoftwareROMAlyzer->lineEditSets->setText("*");
		}
		if ( qmc2SoftwareROMAlyzer->isHidden() )
			qmc2SoftwareROMAlyzer->show();
		else if ( qmc2SoftwareROMAlyzer->isMinimized() )
			qmc2SoftwareROMAlyzer->showNormal();
		if ( qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabReport && qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabLog )
			qmc2SoftwareROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SoftwareROMAlyzer->tabReport);
		QTimer::singleShot(0, qmc2SoftwareROMAlyzer, SLOT(raise()));
		QTimer::singleShot(0, qmc2SoftwareROMAlyzer->pushButtonAnalyze, SLOT(animateClick()));
	}
}

void SoftwareList::analyzeSoftwareLists()
{
	if ( !qmc2SoftwareROMAlyzer )
		qmc2SoftwareROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SOFTWARE);
	if ( !qmc2SoftwareROMAlyzer->active() ) {
		qmc2SoftwareROMAlyzer->lineEditSoftwareLists->setText(systemSoftwareListHash[systemName].join(" "));
		qmc2SoftwareROMAlyzer->lineEditSets->setText("*");
	}
	if ( qmc2SoftwareROMAlyzer->isHidden() )
		qmc2SoftwareROMAlyzer->show();
	else if ( qmc2SoftwareROMAlyzer->isMinimized() )
		qmc2SoftwareROMAlyzer->showNormal();
	if ( qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabReport && qmc2SoftwareROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SoftwareROMAlyzer->tabLog )
		qmc2SoftwareROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SoftwareROMAlyzer->tabReport);
	QTimer::singleShot(0, qmc2SoftwareROMAlyzer, SLOT(raise()));
	QTimer::singleShot(0, qmc2SoftwareROMAlyzer->pushButtonAnalyze, SLOT(animateClick()));
}

SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent, bool viewTree)
{
	parentTreeWidget = parent;
	numTotal = numCorrect = numMostlyCorrect = numIncorrect = numNotFound = numUnknown = elementCounter = 0;
	newSoftwareStates = false;
	setViewTree(viewTree);
}

SoftwareListXmlHandler::~SoftwareListXmlHandler()
{
	if ( !itemList().isEmpty() )
		foreach (QTreeWidgetItem *item, itemList())
			delete item;
}

void SoftwareListXmlHandler::loadSoftwareStates(QString listName)
{
	QString softwareStateCachePath = QDir::toNativeSeparators(QDir::cleanPath(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache").toString() + "/" + listName + ".ssc"));
	QFile stateCacheFile(softwareStateCachePath);
	if ( stateCacheFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		numTotal = numCorrect = numMostlyCorrect = numIncorrect = numNotFound = numUnknown = 0;
		QTextStream ts(&stateCacheFile);
		ts.readLine(); // comment line
		while ( !ts.atEnd() ) {
			QStringList words = ts.readLine().trimmed().split(" ", QString::SkipEmptyParts);
			if ( words.count() > 1 ) {
				switch ( words[1][0].toLatin1() ) {
					case 'C':
						softwareListStateHash[listName][words[0]] = 'C';
						break;
					case 'M':
						softwareListStateHash[listName][words[0]] = 'M';
						break;
					case 'I':
						softwareListStateHash[listName][words[0]] = 'I';
						break;
					case 'N':
						softwareListStateHash[listName][words[0]] = 'N';
						break;
					case 'U':
					default:
						softwareListStateHash[listName][words[0]] = 'U';
						break;
				}
			}
		}
		stateCacheFile.close();
	}
	newSoftwareStates = true;
}

bool SoftwareListXmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
	if ( qmc2SoftwareList->interruptLoad )
		return false;

	if ( elementCounter % QMC2_SWLIST_LOAD_RESPONSE_ULTRALONG == 0 ) {
		parentTreeWidget->insertTopLevelItems(0, itemList());
		itemList().clear();
	}

	if ( ++elementCounter % QMC2_SWLIST_LOAD_RESPONSE_LONG == 0 ) {
		qmc2SoftwareList->updateStats(this);
		qApp->processEvents();
	}

	if ( qName == "softwarelist" ) {
		softwareListName = attributes.value("name");
		compatFilters = systemSoftwareFilterHash[qmc2SoftwareList->systemName];
		if ( qmc2SoftwareList->toolButtonSoftwareStates->isChecked() )
			if ( !softwareListStateHash.contains(softwareListName) )
				loadSoftwareStates(softwareListName);
		return true;
	}

	if ( qName == "software" ) {
		softwareName = attributes.value("name");
		softwareSupported = attributes.value("supported");
		softwareParentName = attributes.value("cloneof");
		QString setKey(softwareListName + ":" + softwareName);
		if ( !softwareParentName.isEmpty() )
			softwareParentHash[setKey] = softwareListName + ":" + softwareParentName;
		else
			softwareParentHash[setKey] = "<no_parent>";
		if ( softwareSupported.isEmpty() || softwareSupported == "yes" )
			softwareSupported = QObject::tr("yes");
		else if ( softwareSupported == "no" )
			softwareSupported = QObject::tr("no");
		else
			softwareSupported = QObject::tr("partially");
		softwareItem = new SoftwareItem((QTreeWidget *)0);
		itemList() << softwareItem;
		softwareItemHash[setKey] = softwareItem;
		SoftwareItem *subItem = new SoftwareItem(softwareItem);
		subItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Waiting for data..."));
		softwareItem->setText(QMC2_SWLIST_COLUMN_NAME, softwareName);
		softwareItem->setText(QMC2_SWLIST_COLUMN_LIST, softwareListName);
		softwareItem->setText(QMC2_SWLIST_COLUMN_SUPPORTED, softwareSupported);
		numTotal++;
		if ( qmc2SoftwareList->toolButtonSoftwareStates->isChecked() ) {
			if ( softwareListStateHash[softwareListName].contains(softwareName) ) {
				switch ( softwareListStateHash[softwareListName][softwareName] ) {
					case 'C':
						numCorrect++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "C");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonCorrect->isChecked());
						else
							softwareItem->setHidden(false);
						break;
					case 'M':
						numMostlyCorrect++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "M");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonMostlyCorrect->isChecked());
						else
							softwareItem->setHidden(false);
						break;
					case 'I':
						numIncorrect++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "I");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonIncorrect->isChecked());
						else
							softwareItem->setHidden(false);
						break;
					case 'N':
						numNotFound++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "N");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonNotFound->isChecked());
						else
							softwareItem->setHidden(false);
						break;
					case 'U':
					default:
						numUnknown++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonUnknown->isChecked());
						else
							softwareItem->setHidden(false);
						break;
				}
			} else {
				numUnknown++;
				softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
				softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
				if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
					softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonUnknown->isChecked());
				else
					softwareItem->setHidden(false);
			}
		}
		return true;
	}

	if ( qName == "part" ) {
		softwarePart = attributes.value("name");
		QString parts = softwareItem->text(QMC2_SWLIST_COLUMN_PART);
		softwareInterface = attributes.value("interface");
		QString interfaces = softwareItem->text(QMC2_SWLIST_COLUMN_INTERFACE);
		if ( parts.isEmpty() )
			softwareItem->setText(QMC2_SWLIST_COLUMN_PART, softwarePart);
		else
			softwareItem->setText(QMC2_SWLIST_COLUMN_PART, parts + "," + softwarePart);
		if ( interfaces.isEmpty() )
			softwareItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, softwareInterface);
		else
			softwareItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, interfaces + "," + softwareInterface);
		return true;
	}
	
	if ( qName == "feature" ) {
		if ( attributes.value("name") == "compatibility" ) {
			// we use the invisible whatsThis data of the title column to store the software-compatibility list
			QString partCompat = attributes.value("value");
			if ( !partCompat.isEmpty() ) {
				softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, partCompat);
				if ( qmc2SoftwareList->toolButtonCompatFilterToggle->isChecked() ) {
					QStringList compatList = partCompat.split(",", QString::SkipEmptyParts);
					bool showItem = compatList.isEmpty() || compatFilters.isEmpty();
					for (int i = 0; i < compatList.count() && !showItem; i++)
						for (int j = 0; j < compatFilters.count() && !showItem; j++)
							showItem = (compatList[i] == compatFilters[j]);
					if ( !softwareItem->isHidden() )
						softwareItem->setHidden(!showItem);
				}
			}
		}
		return true;
	}

	if ( qName == "description" || qName == "year" || qName == "publisher" )
		currentText.clear();

	return true;
}

bool SoftwareListXmlHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName)
{
	if ( qmc2SoftwareList->interruptLoad )
		return false;

	if ( qName == "description" ) {
		softwareTitle = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_TITLE, softwareTitle);
		return true;
	}

	if ( qName == "year" ) {
		softwareYear = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_YEAR, softwareYear);
		return true;
	}

	if ( qName == "publisher" ) {
		softwarePublisher = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, softwarePublisher);
		return true;
	}

	if ( qName == "softwarelist" ) {
		parentTreeWidget->insertTopLevelItems(0, itemList());
		itemList().clear();
	}

	return true;
}

bool SoftwareListXmlHandler::characters(const QString &str)
{
	currentText += QString::fromUtf8(str.toLocal8Bit());
	return true;
}

SoftwareSnap::SoftwareSnap(QWidget *parent)
	: QWidget(parent, Qt::ToolTip)
{
	setAttribute(Qt::WA_TranslucentBackground);
	setFocusPolicy(Qt::NoFocus);
	snapForcedResetTimer.setSingleShot(true);
	connect(&snapForcedResetTimer, SIGNAL(timeout()), this, SLOT(resetSnapForced()));

	ctxMenuRequested = false;

	contextMenu = new QMenu(this);
	contextMenu->hide();
	
	QString s;
	QAction *action;

	s = tr("Copy image to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	s = tr("Copy file path to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyPathToClipboard()));
	actionCopyPathToClipboard = action;

	contextMenu->addSeparator();

	s = tr("Zoom in (+10%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-in.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(zoomIn()));

	s = tr("Zoom out (-10%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-out.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(zoomOut()));

	s = tr("Reset zoom (100%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-none.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(resetZoom()));

	contextMenu->addSeparator();

	s = tr("Refresh cache slot");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));

	zoom = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapZoom", 100).toInt();

	if ( useZip() ) {
		foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
			snapFileMap[filePath] = unzOpen(filePath.toLocal8Bit());
			if ( snapFileMap[filePath] == NULL )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(filePath));
		}
	} else if ( useSevenZip() ) {
		foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
			SevenZipFile *snapFile = new SevenZipFile(filePath);
			if ( !snapFile->open() ) {
				  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file %1").arg(filePath) + " - " + tr("7z error") + ": " + snapFile->lastError());
				  delete snapFile;
			} else {
				snapFileMap7z[filePath] = snapFile;
				connect(snapFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
			}
		}
	}

	reloadActiveFormats();
}

SoftwareSnap::~SoftwareSnap()
{
	if ( qmc2UseSoftwareSnapFile ) {
		foreach (unzFile snapFile, snapFileMap)
			unzClose(snapFile);
		foreach (SevenZipFile *snapFile, snapFileMap7z) {
			snapFile->close();
			delete snapFile;
		}
		snapFileMap.clear();
		snapFileMap7z.clear();
	}
}

QString SoftwareSnap::primaryPathFor(QString list, QString name)
{
	if ( !qmc2UseSoftwareSnapFile ) {
		QStringList fl = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapDirectory").toString().split(";", QString::SkipEmptyParts);
		QString baseDirectory;
		if ( fl.count() > 0 )
			baseDirectory = fl[0];
		return QDir::toNativeSeparators(QDir::cleanPath(baseDirectory + "/" + list + "/" + name + ".png"));
	} else // we don't support on-the-fly image replacement for zipped images yet!
		return QString();
}

bool SoftwareSnap::replaceImage(QString list, QString name, QPixmap &pixmap)
{
	if ( !qmc2UseSoftwareSnapFile ) {
		QString savePath = primaryPathFor(list, name);
		if ( !savePath.isEmpty() ) {
			bool goOn = true;
			if ( QFile::exists(savePath) ) {
				QString backupPath = savePath + ".bak";
				if ( QFile::exists(backupPath) )
					QFile::remove(backupPath);
				if ( !QFile::copy(savePath, backupPath) ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create backup of existing image file '%1' as '%2'").arg(savePath).arg(backupPath));
					goOn = false;
				}
			}
			if ( goOn ) {
				QString primaryPath = QFileInfo(savePath).absoluteDir().absolutePath();
				QDir ppDir(primaryPath);
				if ( !ppDir.exists() )
					ppDir.mkpath(primaryPath);
				if ( pixmap.save(savePath, "PNG") ) {
					refresh();
					if ( qmc2SoftwareSnapshot )
						qmc2SoftwareSnapshot->refresh();
					return true;
				} else {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create image file '%1'").arg(savePath));
					return false;
				}
			} else
				return false;
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't determine primary path for image-type '%1'").arg(tr("software snapshot")));
			return false;
		}
	} else // we don't support on-the-fly image replacement for zipped images yet!
		return false;
}

void SoftwareSnap::zoomIn()
{
	zoom += 10;
	if ( zoom > 400 )
		zoom = 400;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapZoom", zoom);
	refresh();
}

void SoftwareSnap::zoomOut()
{
	zoom -= 10;
	if ( zoom < 10 )
		zoom = 10;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapZoom", zoom);
	refresh();
}

void SoftwareSnap::resetZoom()
{
	zoom = 100;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapZoom", zoom);
	refresh();
}

void SoftwareSnap::mousePressEvent(QMouseEvent *e)
{
	if ( e->button() != Qt::RightButton)
		hide();
	else
		ctxMenuRequested = true;
}

void SoftwareSnap::enterEvent(QEvent *e)
{
	if ( contextMenu->isVisible() )
		QTimer::singleShot(0, contextMenu, SLOT(hide()));
	ctxMenuRequested = false;

	QWidget::enterEvent(e);
}

void SoftwareSnap::leaveEvent(QEvent *e)
{
	if ( !qmc2SoftwareList->snapForced && !ctxMenuRequested ) {
		myItem = NULL;
		hide();
	}  else if ( !qmc2SoftwareList->snapForced )
		QTimer::singleShot(QMC2_SWSNAP_UNFORCE_DELAY, qmc2SoftwareList, SLOT(checkSoftwareSnap()));

	ctxMenuRequested = contextMenu->isVisible();

	QWidget::leaveEvent(e);
}

void SoftwareSnap::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	loadSnapshot();
	p.eraseRect(rect());
	p.end();
}

void SoftwareSnap::loadSnapshot()
{
	ctxMenuRequested = false;

	if ( !qmc2SoftwareList || qmc2SoftwareSnapPosition == QMC2_SWSNAP_POS_DISABLE_SNAPS ) {
		myItem = NULL;
		resetSnapForced();
		myCacheKey.clear();
		return;
	}

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AutoDisableSoftwareSnap", true).toBool() ) {
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE || (qmc2MainWindow->tabWidgetSoftwareDetail->parent() == qmc2MainWindow && qmc2MainWindow->tabWidgetSoftwareDetail->isVisible()) ) {
			myItem = NULL;
			resetSnapForced();
			myCacheKey.clear();
			return;
		}
	}

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::loadSnapshot(): snapForced = '%1'").arg(qmc2SoftwareList->snapForced ? "true" : "false"));
#endif

	// check if the mouse cursor is still on a software item
	QTreeWidgetItem *item = NULL;
	QTreeWidget *treeWidget = NULL;
	QRect rect;

	switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			if ( qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetKnownSoftware->itemAt(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetKnownSoftware->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			if ( qmc2SoftwareList->viewTree() )
				treeWidget = qmc2SoftwareList->treeWidgetKnownSoftwareTree;
			else
				treeWidget = qmc2SoftwareList->treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			if ( qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetFavoriteSoftware->itemAt(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetFavoriteSoftware->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			treeWidget = qmc2SoftwareList->treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			if ( qmc2SoftwareList->treeWidgetSearchResults->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetSearchResults->itemAt(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetSearchResults->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetSearchResults->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			treeWidget = qmc2SoftwareList->treeWidgetSearchResults;
			break;
	}

	// try to fall back to 'selected item' if applicable (no mouse hover)
	if ( !item || qmc2SoftwareList->snapForced ) {
		if ( qmc2SoftwareList->snapForced && myItem != NULL ) {
			item = myItem;
			switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
				case QMC2_SWLIST_KNOWN_SW_PAGE:
					if ( qmc2SoftwareList->viewTree() ) {
						rect = qmc2SoftwareList->treeWidgetKnownSoftwareTree->visualItemRect(item);
						rect.setWidth(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->width());
						rect.setX(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->x());
						rect.translate(0, 4);
						position = qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(rect.bottomLeft());
						treeWidget = qmc2SoftwareList->treeWidgetKnownSoftwareTree;
					} else {
						rect = qmc2SoftwareList->treeWidgetKnownSoftware->visualItemRect(item);
						rect.setWidth(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->width());
						rect.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->x());
						rect.translate(0, 4);
						position = qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomLeft());
						treeWidget = qmc2SoftwareList->treeWidgetKnownSoftware;
					}
					break;
				case QMC2_SWLIST_FAVORITES_PAGE:
					rect = qmc2SoftwareList->treeWidgetFavoriteSoftware->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.bottomLeft());
					treeWidget = qmc2SoftwareList->treeWidgetFavoriteSoftware;
					break;
				case QMC2_SWLIST_SEARCH_PAGE:
					rect = qmc2SoftwareList->treeWidgetSearchResults->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetSearchResults->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.bottomLeft());
					treeWidget = qmc2SoftwareList->treeWidgetSearchResults;
					break;
			}
		}
	}

	// if we can't figure out which item we're on, let's escape from here...
	if ( !item || !treeWidget ) {
		myItem = NULL;
		resetSnapForced();
		qmc2SoftwareList->cancelSoftwareSnap();
		myCacheKey.clear();
		return;
	}

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() ) {
		// paranoia check :)
		QList<QTreeWidgetItem *> itemList = treeWidget->selectedItems();
		if ( !itemList.isEmpty() )
			if ( itemList[0] != item ) {
				myItem = NULL;
				resetSnapForced();
				qmc2SoftwareList->cancelSoftwareSnap();
				myCacheKey.clear();
				return;
			}
	}

	listName = item->text(QMC2_SWLIST_COLUMN_LIST);
	entryName = item->text(QMC2_SWLIST_COLUMN_NAME);
	myItem = (SoftwareItem *)item;
	myCacheKey = "sws_" + listName + "_" + entryName;

	ImagePixmap pm;
	bool pmLoaded = false;
	bool drawFrame = true;
	ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
	if ( cpm ) {
		pmLoaded = true;
		pm = *cpm;
	}

	if ( !pmLoaded ) {
		if ( qmc2UseSoftwareSnapFile ) {
			if ( useZip() ) {
				// try loading image from (semicolon-separated) ZIP archive(s)
				if ( snapFileMap.isEmpty() ) {
					foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
						snapFileMap[filePath] = unzOpen(filePath.toLocal8Bit());
						if ( snapFileMap[filePath] == NULL )
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(filePath));
					}
				}
				foreach (unzFile snapFile, snapFileMap) {
					if ( snapFile ) {
						bool fileOk = true;
						QByteArray imageData;
						foreach (int format, activeFormats) {
							QString formatName = ImageWidget::formatNames[format];
							foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString pathInZip = listName + "/" + entryName + "." + extension;
								if ( unzLocateFile(snapFile, pathInZip.toLocal8Bit().constData(), 0) == UNZ_OK ) {
									if ( unzOpenCurrentFile(snapFile) == UNZ_OK ) {
										char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
										int len;
										while ( (len = unzReadCurrentFile(snapFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
											imageData.append(imageBuffer, len);
										unzCloseCurrentFile(snapFile);
										fileOk = true;
									} else
										fileOk = false;
								} else
									fileOk = false;

								if ( fileOk ) {
									if ( pm.loadFromData(imageData, formatName.toLocal8Bit().constData()) ) {
										pmLoaded = true;
										qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
										break;
									}
								}
							}

							if ( pmLoaded )
								break;
						}
					}

					if ( pmLoaded )
						break;
				}
			} else if ( useSevenZip() ) {
				// try loading image from (semicolon-separated) 7z archive(s)
				if ( snapFileMap7z.isEmpty() ) {
					foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
						SevenZipFile *snapFile = new SevenZipFile(filePath);
						if ( !snapFile->open() ) {
							  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file %1").arg(filePath) + " - " + tr("7z error") + ": " + snapFile->lastError());
							  delete snapFile;
						} else {
							snapFileMap7z[filePath] = snapFile;
							connect(snapFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
						}
					}
				}
				foreach (SevenZipFile *snapFile, snapFileMap7z) {
					if ( snapFile ) {
						bool fileOk = true;
						QByteArray imageData;
						foreach (int format, activeFormats) {
							QString formatName = ImageWidget::formatNames[format];
							foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								bool isFillingDictionary = false;
								QString pathIn7z = listName + "/" + entryName + "." + extension;
								int index = snapFile->indexOfName(pathIn7z);
								if ( index >= 0 ) {
									m_async = true;
									quint64 readLength = snapFile->read(index, &imageData, &m_async);
									if ( readLength == 0 && m_async ) {
										qmc2ImagePixmapCache.remove(myCacheKey);
										isFillingDictionary = true;
										fileOk = true;
									} else
										fileOk = !snapFile->hasError();
								} else
									fileOk = false;

								if ( fileOk ) {
									if ( isFillingDictionary ) {
										pm = qmc2MainWindow->qmc2GhostImagePixmap.scaledToHeight(qmc2MainWindow->qmc2GhostImagePixmap.height()/2, Qt::SmoothTransformation);
										pm.isGhost = false;
										QPainter p;
										QString message = tr("Decompressing archive, please wait...");
										p.begin(&pm);
										p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
										QFont f(qApp->font());
										f.setWeight(QFont::Bold);
										f.setPointSize(f.pointSize() * 2);
										QFontMetrics fm(f);
										int adjustment = fm.height() / 2;
										p.setFont(f);
										QRect outerRect = p.boundingRect(pm.rect(), Qt::AlignCenter | Qt::TextWordWrap, message).adjusted(-adjustment, -adjustment, adjustment, adjustment);
										QPainterPath pp;
										pp.addRoundedRect(outerRect, 5, 5);
										p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
										p.setPen(QColor(255, 255, 0, 255));
										p.drawText(pm.rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
										p.end();
										pmLoaded = true;
										drawFrame = false;
										enableWidgets(false);
									} else if ( pm.loadFromData(imageData, formatName.toLocal8Bit().constData()) ) {
										pmLoaded = true;
										qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
										break;
									}
								}
							}

							if ( pmLoaded )
								break;
						}
					}

					if ( pmLoaded )
						break;
				}
			}
		} else {
			// try loading image from (semicolon-separated) software-snapshot folder(s)
			pmLoaded = false;
			foreach (QString baseDirectory, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapDirectory").toString().split(";", QString::SkipEmptyParts)) {
				QDir snapDir(baseDirectory + "/" + listName);
				foreach (int format, activeFormats) {
					foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
						QString fullEntryName = entryName + "." + extension;
						if ( snapDir.exists(fullEntryName) ) {
							QString filePath = snapDir.absoluteFilePath(fullEntryName);
							if ( pm.load(filePath) ) {
								pmLoaded = true;
								pm.imagePath = filePath;
								qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount()); 
							}
						}
						if ( pmLoaded )
							break;
					}
					if ( pmLoaded )
						break;
				}
				if ( pmLoaded )
					break;
			}
		}
	}

	if ( pmLoaded && !pm.isGhost ) {
		qreal factor = (qreal)zoom / 100.0;
		QSize zoomSize(factor * pm.size().width(), factor * pm.size().height());
		pm = pm.scaled(zoomSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		resize(pm.size());
		switch ( qmc2SoftwareSnapPosition ) {
			case QMC2_SWSNAP_POS_ABOVE_CENTER:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						if ( qmc2SoftwareList->viewTree() ) {
							position.setX(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->rect().center()).x() - width() / 2);
							position.setY(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						} else {
							position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().center()).x() - width() / 2);
							position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						}
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->rect().center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetSearchResults->viewport()->rect().center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_ABOVE_RIGHT:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						if ( qmc2SoftwareList->viewTree() ) {
							position.setX(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->rect().bottomRight()).x() - width());
							position.setY(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						} else {
							position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().bottomRight()).x() - width());
							position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						}
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->rect().bottomRight()).x() - width());
						position.setY(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetSearchResults->viewport()->rect().bottomRight()).x() - width());
						position.setY(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_ABOVE_LEFT:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						if ( qmc2SoftwareList->viewTree() )
							position.setY(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						else
							position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setY(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setY(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_BELOW_RIGHT:
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						if ( qmc2SoftwareList->viewTree() )
							position.setX(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->rect().bottomRight()).x() - width());
						else
							position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().bottomRight()).x() - width());
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->rect().bottomRight()).x() - width());
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetSearchResults->viewport()->rect().bottomRight()).x() - width());
						break;
				}
				break;

			case QMC2_SWSNAP_POS_BELOW_CENTER:
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						if ( qmc2SoftwareList->viewTree() )
							position.setX(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftwareTree->viewport()->rect().center()).x() - width() / 2);
						else
							position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().center()).x() - width() / 2);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->rect().center()).x() - width() / 2);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetSearchResults->viewport()->rect().center()).x() - width() / 2);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_BELOW_LEFT:
			default:
				// already prepared above...
				break;
		}
		move(position);
		QPalette pal = palette();
		if ( drawFrame ) {
			QPainter p;
			p.begin(&pm);
			p.setPen(QPen(QColor(0, 0, 0, 64), 1));
			rect = pm.rect();
			rect.setWidth(rect.width() - 1);
			rect.setHeight(rect.height() - 1);
			p.drawRect(rect);
			p.end();
		}
		pal.setBrush(QPalette::Window, pm);
		setPalette(pal);
		showNormal();
		update();
		snapForcedResetTimer.start(QMC2_SWSNAP_UNFORCE_DELAY);
	} else {
		myItem = NULL;
		resetSnapForced();
		qmc2SoftwareList->cancelSoftwareSnap();
	}
}

void SoftwareSnap::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		qmc2ImagePixmapCache.remove(myCacheKey);
		update();
	}
}

void SoftwareSnap::sevenZipDataReady()
{
	update();
	enableWidgets(true);
}

void SoftwareSnap::enableWidgets(bool enable)
{
	qmc2Options->radioButtonSoftwareSnapSelect->setEnabled(enable);
	qmc2Options->lineEditSoftwareSnapFile->setEnabled(enable);
	qmc2Options->comboBoxSoftwareSnapFileType->setEnabled(enable);
	qmc2Options->toolButtonBrowseSoftwareSnapFile->setEnabled(enable);
}

void SoftwareSnap::resetSnapForced()
{
	if ( qmc2SoftwareList ) {
		QTreeWidgetItem *item = NULL;
		if ( !qmc2SoftwareList->snapForced ) {
			switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
				case QMC2_SWLIST_KNOWN_SW_PAGE:
					if ( qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->underMouse() ) {
						item = qmc2SoftwareList->treeWidgetKnownSoftware->itemAt(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapFromGlobal(QCursor::pos()));
						if ( item )
							if ( item != myItem || item->text(QMC2_SWLIST_COLUMN_NAME) != entryName  ) {
								qmc2SoftwareList->cancelSoftwareSnap();
								qmc2SoftwareList->snapTimer.start(QMC2_SWSNAP_DELAY);
							}
					}
					break;
				case QMC2_SWLIST_FAVORITES_PAGE:
					if ( qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->underMouse() ) {
						item = qmc2SoftwareList->treeWidgetFavoriteSoftware->itemAt(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapFromGlobal(QCursor::pos()));
						if ( item )
							if ( item != myItem || item->text(QMC2_SWLIST_COLUMN_NAME) != entryName  ) {
								qmc2SoftwareList->cancelSoftwareSnap();
								qmc2SoftwareList->snapTimer.start(QMC2_SWSNAP_DELAY);
							}
					}
					break;
				case QMC2_SWLIST_SEARCH_PAGE:
					if ( qmc2SoftwareList->treeWidgetSearchResults->viewport()->underMouse() ) {
						item = qmc2SoftwareList->treeWidgetSearchResults->itemAt(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapFromGlobal(QCursor::pos()));
						if ( item )
							if ( item != myItem || item->text(QMC2_SWLIST_COLUMN_NAME) != entryName  ) {
								qmc2SoftwareList->cancelSoftwareSnap();
								qmc2SoftwareList->snapTimer.start(QMC2_SWSNAP_DELAY);
							}
					}
					break;
			}
		}
	}
	qmc2SoftwareList->snapForced = false;
}

void SoftwareSnap::contextMenuEvent(QContextMenuEvent *e)
{
	ctxMenuRequested = true;
	if ( !myCacheKey.isEmpty() ) {
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
		if ( cpm )
			actionCopyPathToClipboard->setVisible(!cpm->imagePath.isEmpty());
		else
			actionCopyPathToClipboard->setVisible(false);
	} else
		actionCopyPathToClipboard->setVisible(false);
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

void SoftwareSnap::copyToClipboard()
{
	QPixmap pm(size());
	render(&pm);
	qApp->clipboard()->setPixmap(pm);
}

void SoftwareSnap::copyPathToClipboard()
{
	if ( !myCacheKey.isEmpty() ) {
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
		if ( cpm )
			qApp->clipboard()->setText(cpm->imagePath);
	}
}

void SoftwareSnap::reloadActiveFormats()
{
	activeFormats.clear();
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/sws", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}

bool SoftwareSnap::useZip()
{
	return qmc2UseSoftwareSnapFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool SoftwareSnap::useSevenZip()
{
	return qmc2UseSoftwareSnapFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

SoftwareEntryXmlHandler::SoftwareEntryXmlHandler(QTreeWidgetItem *item)
{
	parentTreeWidgetItem = (SoftwareItem *)item;
	softwareName = parentTreeWidgetItem->text(QMC2_SWLIST_COLUMN_NAME);
	softwareValid = success = false;
	partItem = dataareaItem = romItem = NULL;
	elementCounter = animSequenceCounter = 0;
}

SoftwareEntryXmlHandler::~SoftwareEntryXmlHandler()
{
	// NOP
}

bool SoftwareEntryXmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
	if ( ++elementCounter % QMC2_SWLIST_LOAD_RESPONSE_LONG == 0 ) {
		QTreeWidgetItem *item = parentTreeWidgetItem->child(0);
		if ( elementCounter % QMC2_SWLIST_LOAD_ANIM_DELAY == 0 ) {
			if ( item->text(QMC2_SWLIST_COLUMN_TITLE).startsWith(QObject::tr("Searching")) ) {
				QString dot(".");
				item->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Searching") + dot.repeated(++animSequenceCounter));
			}
		}
		parentTreeWidgetItem->treeWidget()->viewport()->update();
		qApp->processEvents();
	}

	if ( !softwareValid ) {
		if ( qName == "software" ) {
			softwareValid = ( attributes.value("name") == softwareName );
			if ( softwareValid ) {
				qmc2SoftwareList->successfulLookups.clear();
				parentTreeWidgetItem->child(0)->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Updating"));
				parentTreeWidgetItem->treeWidget()->viewport()->update();
				qApp->processEvents();
			}
		}

		return true;
	}

	if ( qName == "part" ) {
		if ( partItem == NULL ) {
			partItem = new SoftwareItem((QTreeWidget *)NULL);
			partItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Part:") + " " + attributes.value("name"));
			partItem->setText(QMC2_SWLIST_COLUMN_PART, attributes.value("name"));
			partItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, attributes.value("interface"));
			partItem->setText(QMC2_SWLIST_COLUMN_LIST, parentTreeWidgetItem->text(QMC2_SWLIST_COLUMN_LIST));
			QStringList mountList;
			QString mountDev = qmc2SoftwareList->lookupMountDevice(partItem->text(QMC2_SWLIST_COLUMN_PART), partItem->text(QMC2_SWLIST_COLUMN_INTERFACE), &mountList);
			QComboBox *comboBoxMountDevices = NULL;
			if ( mountList.count() > 0 ) {
				comboBoxMountDevices = new QComboBox;
				comboBoxMountDevices->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
				mountList.prepend(QObject::tr("Don't mount"));
				mountList.prepend(QObject::tr("Auto mount"));
				comboBoxMountDevices->insertItems(0, mountList);
				comboBoxMountDevices->insertSeparator(QMC2_SWLIST_MSEL_SEPARATOR);
				if ( !qmc2SoftwareList->autoMounted ) {
					comboBoxMountDevices->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // ==> don't mount
					partItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				} else {
					comboBoxMountDevices->setCurrentIndex(QMC2_SWLIST_MSEL_AUTO_MOUNT); // ==> auto mount
					if ( mountDev.isEmpty() )
						partItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
					else
						partItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + mountDev);
				}
				parentTreeWidgetItem->treeWidget()->setItemWidget(partItem, QMC2_SWLIST_COLUMN_PUBLISHER, comboBoxMountDevices);
				QObject::connect(comboBoxMountDevices, SIGNAL(currentIndexChanged(int)), qmc2SoftwareList, SLOT(checkMountDeviceSelection()));
			} else {
				partItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("No mount device"));
				partItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, QObject::tr("Unmanaged"));
			}
			partItems << partItem;
			comboBoxes[partItem] = comboBoxMountDevices;
		}

		return true;
	}

	if ( qName == "feature" ) {
		if ( partItem != NULL ) {
			QString featureName = attributes.value("name");
			if ( featureName == "part id" || featureName == "part_id" ) {
				QString partTitle = attributes.value("value");
				if ( !partTitle.isEmpty() )
					partItem->setText(QMC2_SWLIST_COLUMN_TITLE, partItem->text(QMC2_SWLIST_COLUMN_TITLE) + " (" + partTitle + ")");
			}
		}

		return true;
	}

	if ( qName == "dataarea" ) {
		if ( partItem != NULL ) {
			dataareaItem = new SoftwareItem(partItem);
			dataareaItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Data area:") + " " + attributes.value("name"));
			QString s = attributes.value("size");
			if ( !s.isEmpty() )
				dataareaItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Size:") + " " + s);
		}

		return true;
	}

	if ( qName == "diskarea" ) {
		if ( partItem != NULL ) {
			dataareaItem = new SoftwareItem(partItem);
			dataareaItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Disk area:") + " " + attributes.value("name"));
			QString s = attributes.value("size");
			if ( !s.isEmpty() )
				dataareaItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Size:") + " " + s);
		}

		return true;
	}

	if ( qName == "rom" ) {
		if ( dataareaItem != NULL ) {
			QString romName = attributes.value("name");
			if ( !romName.isEmpty() ) {
				romItem = new SoftwareItem(dataareaItem);
				romItem->setText(QMC2_SWLIST_COLUMN_TITLE, romName);
				QString s = attributes.value("size");
				if ( !s.isEmpty() )
					romItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Size:") + " " + s);
				s = attributes.value("crc");
				if ( !s.isEmpty() )
					romItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, QObject::tr("CRC:") + " " + s);
			}
		}

		return true;
	}

	if ( qName == "disk" ) {
		if ( dataareaItem != NULL ) {
			QString diskName = attributes.value("name");
			if ( !diskName.isEmpty() ) {
				romItem = new SoftwareItem(dataareaItem);
				romItem->setText(QMC2_SWLIST_COLUMN_TITLE, diskName);
				QString s = attributes.value("sha1");
				if ( !s.isEmpty() )
					romItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("SHA1:") + " " + s);
			}
		}

		return true;
	}

	if ( qName == "info" ) {
		infoItem = new SoftwareItem((QTreeWidget *)NULL);
		infoItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Info:") + " " + attributes.value("name"));
#if defined(QMC2_OS_WIN)
		infoItem->setText(QMC2_SWLIST_COLUMN_NAME, QString::fromUtf8(attributes.value("value").toLocal8Bit()));
#else
		infoItem->setText(QMC2_SWLIST_COLUMN_NAME, attributes.value("value"));
#endif
		infoItems << infoItem;
	}

	return true;
}

bool SoftwareEntryXmlHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName)
{
	if ( !softwareValid )
		return true;

	if ( qName == "software" ) {
		// stop here...
		parentTreeWidgetItem->treeWidget()->setUpdatesEnabled(false);
		QTreeWidgetItem *childItem = parentTreeWidgetItem->takeChild(0);
		delete childItem;
		parentTreeWidgetItem->addChildren(partItems);
		for (int i = 0; i < partItems.count(); i++) {
			QTreeWidgetItem *item = partItems[i];
			QComboBox *cb = comboBoxes[item];
			if ( cb )
				parentTreeWidgetItem->treeWidget()->setItemWidget(item, QMC2_SWLIST_COLUMN_PUBLISHER, cb);
		}
		if ( !infoItems.isEmpty() )
			parentTreeWidgetItem->addChildren(infoItems);
		parentTreeWidgetItem->treeWidget()->setUpdatesEnabled(true);
		parentTreeWidgetItem->treeWidget()->viewport()->update();
		qApp->processEvents();
		success = true;
		return false;
	}

	if ( qName == "part" ) {
		partItem = NULL;
		return true;
	}

	if ( qName == "dataarea" || qName == "diskarea" ) {
		dataareaItem = NULL;
		return true;
	}

	if ( qName == "rom" ) {
		romItem = NULL;
		return true;
	}

	return true;
}

SoftwareSnapshot::SoftwareSnapshot(QWidget *parent)
#if QMC2_OPENGL == 1
	: QGLWidget(parent)
#else
	: QWidget(parent)
#endif
{
	contextMenu = new QMenu(this);
	contextMenu->hide();

	QString s;
	QAction *action;

	s = tr("Copy image to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	s = tr("Copy file path to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyPathToClipboard()));
	actionCopyPathToClipboard = action;

	contextMenu->addSeparator();

	s = tr("Refresh cache slot");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));

	reloadActiveFormats();
}

SoftwareSnapshot::~SoftwareSnapshot()
{
	if ( qmc2UseSoftwareSnapFile ) {
		foreach (unzFile snapFile, qmc2SoftwareSnap->snapFileMap)
			unzClose(snapFile);
		foreach (SevenZipFile *snapFile, qmc2SoftwareSnap->snapFileMap7z) {
			snapFile->close();
			delete snapFile;
		}
		qmc2SoftwareSnap->snapFileMap.clear();
		qmc2SoftwareSnap->snapFileMap7z.clear();
	}
}

void SoftwareSnapshot::paintEvent(QPaintEvent *e)
{
	QPainter p(this);

	if ( !qmc2SoftwareList->currentItem ) {
		drawCenteredImage(0, &p); // clear snapshot widget
		myCacheKey.clear();
		return;
	}

	QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
	QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
	myCacheKey = "sws_" + listName + "_" + entryName;

	ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
	if ( !cpm )
		loadSnapshot(listName, entryName);
	else {
		currentSnapshotPixmap = *cpm;
		currentSnapshotPixmap.imagePath = cpm->imagePath;
	}

	drawScaledImage(&currentSnapshotPixmap, &p);
}

QString SoftwareSnapshot::toBase64()
{
	ImagePixmap pm;
	if ( !currentSnapshotPixmap.isNull() )
		pm = currentSnapshotPixmap;
	else
		pm = qmc2MainWindow->qmc2GhostImagePixmap;
	QByteArray imageData;
	QBuffer buffer(&imageData);
	pm.save(&buffer, "PNG");
	return QString(imageData.toBase64());
}

void SoftwareSnapshot::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		qmc2ImagePixmapCache.remove(myCacheKey);
		update();
	}
}

void SoftwareSnapshot::sevenZipDataReady()
{
	update();
	enableWidgets(true);
}

void SoftwareSnapshot::enableWidgets(bool enable)
{
	qmc2Options->radioButtonSoftwareSnapSelect->setEnabled(enable);
	qmc2Options->lineEditSoftwareSnapFile->setEnabled(enable);
	qmc2Options->comboBoxSoftwareSnapFileType->setEnabled(enable);
	qmc2Options->toolButtonBrowseSoftwareSnapFile->setEnabled(enable);
}


bool SoftwareSnapshot::loadSnapshot(QString listName, QString entryName)
{
	ImagePixmap pm;
	bool fileOk = true;

	myCacheKey = "sws_" + listName + "_" + entryName;
	currentSnapshotPixmap.imagePath.clear();

	if ( qmc2UseSoftwareSnapFile ) {
		if ( qmc2SoftwareSnap->useZip() ) {
			// try loading image from (semicolon-separated) ZIP archive(s)
			if ( qmc2SoftwareSnap->snapFileMap.isEmpty() ) {
				foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
					qmc2SoftwareSnap->snapFileMap[filePath] = unzOpen(filePath.toLocal8Bit());
					if ( qmc2SoftwareSnap->snapFileMap[filePath] == NULL )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(filePath));
				}
			}
			foreach (unzFile snapFile, qmc2SoftwareSnap->snapFileMap) {
				if ( snapFile ) {
					foreach (int format, activeFormats) {
						QString formatName = ImageWidget::formatNames[format];
						foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
							QByteArray imageData;
							QString pathInZip = listName + "/" + entryName + "." + extension;
							if ( unzLocateFile(snapFile, pathInZip.toLocal8Bit().constData(), 0) == UNZ_OK ) {
								if ( unzOpenCurrentFile(snapFile) == UNZ_OK ) {
									char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
									int len;
									while ( (len = unzReadCurrentFile(snapFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
										imageData.append(imageBuffer, len);
									unzCloseCurrentFile(snapFile);
									fileOk = true;
								} else
									fileOk = false;
							} else
								fileOk = false;

							if ( fileOk ) {
								if ( pm.loadFromData(imageData, formatName.toLocal8Bit().constData()) ) {
									qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
									break;
								} else
									fileOk = false;
							}

							if ( fileOk )
								break;
						}

						if ( fileOk )
							break;
					}
				}

				if ( fileOk )
					break;
			}
		} else  if ( qmc2SoftwareSnap->useSevenZip() ) {
			// try loading image from (semicolon-separated) 7z archive(s)
			if ( qmc2SoftwareSnap->snapFileMap7z.isEmpty() ) {
				foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
					SevenZipFile *snapFile = new SevenZipFile(filePath);
					if ( !snapFile->open() ) {
						  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file %1").arg(filePath) + " - " + tr("7z error") + ": " + snapFile->lastError());
						  delete snapFile;
					} else {
						qmc2SoftwareSnap->snapFileMap7z[filePath] = snapFile;
						connect(snapFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
					}
				}
			}
			foreach (SevenZipFile *snapFile, qmc2SoftwareSnap->snapFileMap7z) {
				if ( snapFile ) {
					QByteArray imageData;
					foreach (int format, activeFormats) {
						QString formatName = ImageWidget::formatNames[format];
						foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
							bool isFillingDictionary = false;
							QString pathIn7z = listName + "/" + entryName + "." + extension;
							int index = snapFile->indexOfName(pathIn7z);
							if ( index >= 0 ) {
								m_async = true;
								quint64 readLength = snapFile->read(index, &imageData, &m_async);
								if ( readLength == 0 && m_async ) {
									qmc2ImagePixmapCache.remove(myCacheKey);
									isFillingDictionary = true;
									fileOk = true;
								} else
									fileOk = !snapFile->hasError();
							} else
								fileOk = false;

							if ( fileOk ) {
								if ( isFillingDictionary ) {
									currentSnapshotPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
									QPainter p;
									QString message = tr("Decompressing archive, please wait...");
									p.begin(&currentSnapshotPixmap);
									p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
									QFont f(qApp->font());
									f.setWeight(QFont::Bold);
									f.setPointSize(f.pointSize() * 2);
									QFontMetrics fm(f);
									int adjustment = fm.height() / 2;
									p.setFont(f);
									QRect outerRect = p.boundingRect(currentSnapshotPixmap.rect(), Qt::AlignCenter | Qt::TextWordWrap, message).adjusted(-adjustment, -adjustment, adjustment, adjustment);
									QPainterPath pp;
									pp.addRoundedRect(outerRect, 5, 5);
									p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
									p.setPen(QColor(255, 255, 0, 255));
									p.drawText(currentSnapshotPixmap.rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
									p.end();
									enableWidgets(false);
								} else if ( pm.loadFromData(imageData, formatName.toLocal8Bit().constData()) ) {
									qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
									break;
								} else
									fileOk = false;
							}
						}

						if ( fileOk )
							break;
					}
				}

				if ( fileOk )
					break;
			}
		}
	} else {
		// try loading image from (semicolon-separated) software-snapshot folder(s)
		fileOk = false;
		foreach (QString baseDirectory, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapDirectory").toString().split(";", QString::SkipEmptyParts)) {
			QDir snapDir(baseDirectory + "/" + listName);
			foreach (int format, activeFormats) {
				foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
					QString fullEntryName = entryName + "." + extension;
					if ( snapDir.exists(fullEntryName) ) {
						QString filePath = snapDir.absoluteFilePath(fullEntryName);
						if ( pm.load(filePath) ) {
							fileOk = true;
							currentSnapshotPixmap = pm;
							currentSnapshotPixmap.imagePath = filePath;
							qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(currentSnapshotPixmap), currentSnapshotPixmap.toImage().byteCount()); 
						} else
							fileOk = false;
					}

					if ( fileOk )
						break;
				}

				if ( fileOk )
					break;
			}

			if ( fileOk )
				break;
		}
	}

	if ( !fileOk ) {
		currentSnapshotPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
		if ( !qmc2RetryLoadingImages )
			qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(currentSnapshotPixmap), currentSnapshotPixmap.toImage().byteCount());
        }

	return fileOk;
}

void SoftwareSnapshot::drawCenteredImage(QPixmap *pm, QPainter *p)
{
	p->eraseRect(rect());

	if ( pm == NULL ) {
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	int posx = (rect().width() - pm->width()) / 2;
	int posy = (rect().height() - pm->height()) / 2;

	p->drawPixmap(posx, posy, *pm);

	if ( qmc2ShowGameName ) {
		// draw entry title
		p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
		QString title = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_TITLE);
		QFont f(qApp->font());
		f.setWeight(QFont::Bold);
		p->setFont(f);
		QFontMetrics fm(f);
		QRect r = rect();
		int adjustment = fm.height() / 2;
		r = r.adjusted(+adjustment, +adjustment, -adjustment, -adjustment);
		QRect outerRect = p->boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, title);
		r.setTop(r.bottom() - outerRect.height());
		r = p->boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, title);
		r = r.adjusted(-adjustment, -adjustment, +adjustment, +adjustment);
		r.setBottom(rect().bottom());
		QPainterPath pp;
		pp.addRoundedRect(r, 5, 5);
		p->fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
		p->setPen(QPen(QColor(255, 255, 255, 255)));
		p->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, title);
	}

	p->end();
}

void SoftwareSnapshot::drawScaledImage(QPixmap *pm, QPainter *p)
{
	if ( pm == NULL ) {
		p->eraseRect(rect());
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	double desired_width;
	double desired_height;

	if ( pm->width() > pm->height() ) {
		desired_width  = contentsRect().width();
		desired_height = (double)pm->height() * (desired_width / (double)pm->width());
		if ( desired_height > contentsRect().height() ) {
			desired_height = contentsRect().height();
			desired_width  = (double)pm->width() * (desired_height / (double)pm->height());
		}
	} else {
		desired_height = contentsRect().height();
		desired_width  = (double)pm->width() * (desired_height / (double)pm->height());
		if ( desired_width > contentsRect().width() ) {
			desired_width = contentsRect().width();
			desired_height = (double)pm->height() * (desired_width / (double)pm->width());
		}
	}

	QPixmap pmScaled;

	if ( qmc2SmoothScaling )
		pmScaled = pm->scaled((int)desired_width, (int)desired_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else
		pmScaled = pm->scaled((int)desired_width, (int)desired_height, Qt::KeepAspectRatio, Qt::FastTransformation);

	drawCenteredImage(&pmScaled, p);
}

void SoftwareSnapshot::copyToClipboard()
{
	qApp->clipboard()->setPixmap(currentSnapshotPixmap);
}

void SoftwareSnapshot::copyPathToClipboard()
{
	if ( !currentSnapshotPixmap.imagePath.isEmpty() )
		qApp->clipboard()->setText(currentSnapshotPixmap.imagePath);
}

void SoftwareSnapshot::contextMenuEvent(QContextMenuEvent *e)
{
	actionCopyPathToClipboard->setVisible(!currentSnapshotPixmap.imagePath.isEmpty());
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

void SoftwareSnapshot::reloadActiveFormats()
{
	activeFormats.clear();
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/sws", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}
