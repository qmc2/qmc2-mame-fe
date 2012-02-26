#include <QtGui>
#if defined(Q_WS_MAC)
#include <QTest>
#endif

#include "softwarelist.h"
#include "gamelist.h"
#include "macros.h"
#include "qmc2main.h"
#include "options.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern Gamelist *qmc2Gamelist;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern bool qmc2UseSoftwareSnapFile;
extern SoftwareList *qmc2SoftwareList;
extern SoftwareSnap *qmc2SoftwareSnap;
extern int qmc2SoftwareSnapPosition;
extern bool qmc2IgnoreItemActivation;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ShowGameName;
extern int qmc2UpdateDelay;
extern int qmc2DefaultLaunchMode;
extern bool qmc2StopParser;

QMap<QString, QStringList> systemSoftwareListMap;
QMap<QString, QStringList> systemSoftwareFilterMap;
QMap<QString, QString> softwareListXmlDataCache;
QString swlBuffer;
QString swlLastLine;
QString swlSelectedMountDevice;
bool swlSupported = true;

SoftwareList::SoftwareList(QString sysName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::SoftwareList(QString sysName = %1, QWidget *parent = %2)").arg(sysName).arg((qulonglong)parent));
#endif

	setupUi(this);

	if ( !qmc2SoftwareSnap )
		qmc2SoftwareSnap = new SoftwareSnap(0);

	qmc2SoftwareSnap->hide();
	snapTimer.setSingleShot(true);
	connect(&snapTimer, SIGNAL(timeout()), qmc2SoftwareSnap, SLOT(loadSnapshot()));

	systemName = sysName;
	loadProc = NULL;
	exporter = NULL;
	currentItem = NULL;
	snapForced = autoSelectSearchItem = interruptLoad = isLoading = fullyLoaded = false;
	validData = autoMounted = true;
	cachedDeviceLookupPosition = 0;

#if defined(QMC2_EMUTYPE_MAME)
	comboBoxDeviceConfiguration->setVisible(false);
	QString altText = tr("Add the currently selected software to the favorites list");
	toolButtonAddToFavorites->setToolTip(altText); toolButtonAddToFavorites->setStatusTip(altText);
	treeWidgetFavoriteSoftware->setColumnCount(QMC2_SWLIST_COLUMN_DEVICECFG);
#elif defined(QMC2_EMUTYPE_MESS)
	horizontalLayout->removeItem(horizontalSpacer);
#endif

	oldMin = 0;
	oldMax = 1;
	oldFmt = qmc2MainWindow->progressBarGamelist->format();

	comboBoxSearch->lineEdit()->setPlaceholderText(tr("Enter search string"));

	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonAddToFavorites->setIconSize(iconSize);
	toolButtonRemoveFromFavorites->setIconSize(iconSize);
	toolButtonFavoritesOptions->setIconSize(iconSize);
	toolButtonPlay->setIconSize(iconSize);
	toolButtonReload->setIconSize(iconSize);
	toolButtonExport->setIconSize(iconSize);
	toolButtonToggleSoftwareInfo->setIconSize(iconSize);
	toolButtonCompatFilterToggle->setIconSize(iconSize);
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
	toolButtonPlayEmbedded->setIconSize(iconSize);
#else
	toolButtonPlayEmbedded->setVisible(false);
#endif
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/flat.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
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
	comboBoxDeviceConfiguration->setEnabled(false);

	// software list context menu
	softwareListMenu = new QMenu(this);
	QString s = tr("Play selected software");
	QAction *action = softwareListMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playActivated()));
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
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

	// restore widget states
	treeWidgetKnownSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState").toByteArray());
	treeWidgetFavoriteSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState").toByteArray());
	treeWidgetSearchResults->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState").toByteArray());
	toolBoxSoftwareList->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex").toInt());
	toolButtonToggleSoftwareInfo->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareInfo", false).toBool());
	toolButtonCompatFilterToggle->blockSignals(true);
	toolButtonCompatFilterToggle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/CompatFilter", true).toBool());
	toolButtonCompatFilterToggle->blockSignals(false);

	connect(treeWidgetKnownSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetKnownSoftware_headerSectionClicked(int)));
	connect(treeWidgetFavoriteSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetFavoriteSoftware_headerSectionClicked(int)));
	connect(treeWidgetSearchResults->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetSearchResults_headerSectionClicked(int)));
	connect(&searchTimer, SIGNAL(timeout()), this, SLOT(comboBoxSearch_editTextChanged_delayed()));

	QHeaderView *header;

	// header context menus
	menuKnownSoftwareHeader = new QMenu(0);
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
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &)));

	menuFavoriteSoftwareHeader = new QMenu(0);
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
#if defined(QMC2_EMUTYPE_MESS)
	action = menuFavoriteSoftwareHeader->addAction(tr("Device configuration"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_DEVICECFG);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_DEVICECFG));
#endif
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &)));

	menuSearchResultsHeader = new QMenu(0);
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
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &)));

	// detail update timer
	connect(&detailUpdateTimer, SIGNAL(timeout()), this, SLOT(updateDetail()));
}

SoftwareList::~SoftwareList()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::~SoftwareList()");
#endif

	if ( exporter )
		exporter->close();

	// save widget states
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState", treeWidgetKnownSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState", treeWidgetFavoriteSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState", treeWidgetSearchResults->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex", toolBoxSoftwareList->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareInfo", toolButtonToggleSoftwareInfo->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/CompatFilter", toolButtonCompatFilterToggle->isChecked());
}

QString &SoftwareList::getSoftwareListXmlData(QString listName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::getSoftwareListXmlData(QString listName = %1)").arg(listName));
#endif

	static QString softwareListXmlBuffer;

	softwareListXmlBuffer = softwareListXmlDataCache[listName];

	if ( softwareListXmlBuffer.isEmpty() ) {
		int i = 0;
		int swlLinesMax = swlLines.count() - 1;
		QString s = "<softwarelist name=\"" + listName + "\"";
		while ( !swlLines[i].startsWith(s) && i < swlLinesMax && !interruptLoad ) i++;
		softwareListXmlBuffer = "<?xml version=\"1.0\"?>\n";
		while ( !swlLines[i].startsWith("</softwarelist>") && i < swlLinesMax && !interruptLoad )
			softwareListXmlBuffer += swlLines[i++].simplified() + "\n";
		softwareListXmlBuffer += "</softwarelist>";
		if ( i < swlLinesMax ) {
			softwareListXmlDataCache[listName] = softwareListXmlBuffer;
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: software list '%1' not found").arg(listName));
			toolBoxSoftwareList->setEnabled(false);
			toolButtonExport->setEnabled(false);
			toolButtonToggleSoftwareInfo->setEnabled(false);
			toolButtonCompatFilterToggle->setEnabled(false);
			softwareListXmlBuffer.clear();
		}
	}

	return softwareListXmlBuffer;
}

QString &SoftwareList::lookupMountDevice(QString device, QString interface, QStringList *mountList)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::lookupMountDevice(QString device = %1, QString interface = %2, QStringList *mountList = %3)").arg(device).arg(interface).arg((qulonglong)mountList));
#endif

	static QString softwareListDeviceName;

	QMap<QString, QStringList> deviceInstanceMap;
	int i = cachedDeviceLookupPosition;

	softwareListDeviceName.clear();

#if defined(QMC2_EMUTYPE_MAME)
	QString s = "<game name=\"" + systemName + "\"";
	while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
	if ( qmc2Gamelist->xmlLines[i].contains(s) ) cachedDeviceLookupPosition = i - 1;
	while ( !qmc2Gamelist->xmlLines[i].contains("</game>") ) {
		QString line = qmc2Gamelist->xmlLines[i++].simplified();
		if ( line.startsWith("<device type=\"") ) {
			int startIndex = line.indexOf("interface=\"");
			int endIndex;
			if ( startIndex >= 0 ) {
				startIndex += 11;
				int endIndex = line.indexOf("\"", startIndex);
				QString devInterface = line.mid(startIndex, endIndex - startIndex);
				line = qmc2Gamelist->xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devInterface] << devName;
			} else {
				line = qmc2Gamelist->xmlLines[i++].simplified();
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
	while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
	if ( qmc2Gamelist->xmlLines[i].contains(s) ) cachedDeviceLookupPosition = i - 1;
	while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") ) {
		QString line = qmc2Gamelist->xmlLines[i++].simplified();
		if ( line.startsWith("<device type=\"") ) {
			int startIndex = line.indexOf("interface=\"");
			int endIndex;
			if ( startIndex >= 0 ) {
				startIndex += 11;
				int endIndex = line.indexOf("\"", startIndex);
				QString devInterface = line.mid(startIndex, endIndex - startIndex);
				line = qmc2Gamelist->xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devInterface] << devName;
			} else {
				line = qmc2Gamelist->xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devName] << devName;
			}
		}
	}
#endif

	QStringList briefNames = deviceInstanceMap[interface];

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

QString &SoftwareList::getXmlData()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::getXmlData()");
#endif

	static QString xmlBuffer;

	QStringList softwareList = systemSoftwareListMap[systemName];
	if ( softwareList.isEmpty() || softwareList.contains("NO_SOFTWARE_LIST") ) {
		softwareList.clear();
		int i = 0;
		QString filter;
#if defined(QMC2_EMUTYPE_MAME) 
		QString s = "<game name=\"" + systemName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) && !interruptLoad ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</game>") && !interruptLoad ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
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
		QString s = "<machine name=\"" + systemName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) && !interruptLoad ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") && !interruptLoad ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
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
		systemSoftwareListMap[systemName] = softwareList;
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListMap[%1] = %2").arg(systemName).arg(softwareList.join(", ")));
#endif

		if ( !filter.isEmpty() )
			systemSoftwareFilterMap[systemName] = filter.split(",", QString::SkipEmptyParts);
	}
#ifdef QMC2_DEBUG
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListMap[%1] = %2 (cached)").arg(systemName).arg(systemSoftwareListMap[systemName].join(", ")));
#endif

	xmlBuffer.clear();

	if ( !softwareList.isEmpty() && !softwareList.contains("NO_SOFTWARE_LIST") ) {
		QString swlString = softwareList.join(", ");
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (%1)").arg(swlString));
		toolBoxSoftwareList->setEnabled(true);

#if defined(QMC2_EMUTYPE_MESS)
		// load available device configurations, if any...
		qmc2Config->beginGroup(QString("MESS/Configuration/Devices/%1").arg(systemName));
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

	return xmlBuffer;
}

bool SoftwareList::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::load()");
#endif

	bool swlCacheOkay = true;
	autoMounted = true;
	interruptLoad = false;
	isLoading = true;
	fullyLoaded = false;
	validData = swlSupported;
#if defined(QMC2_EMUTYPE_MAME)
	QString swlCachePath = qmc2Config->value("MAME/FilesAndDirectories/SoftwareListCache").toString();
#elif defined(QMC2_EMUTYPE_MESS)
	QString swlCachePath = qmc2Config->value("MESS/FilesAndDirectories/SoftwareListCache").toString();
#endif

	toolButtonReload->setEnabled(false);

	treeWidgetKnownSoftware->clear();
	treeWidgetFavoriteSoftware->clear();
	treeWidgetSearchResults->clear();

	treeWidgetKnownSoftware->setSortingEnabled(false);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(false);
	treeWidgetFavoriteSoftware->setSortingEnabled(false);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(false);
	treeWidgetSearchResults->setSortingEnabled(false);
	treeWidgetSearchResults->header()->setSortIndicatorShown(false);

	cachedDeviceLookupPosition = 0;

	if ( swlBuffer.isEmpty() && swlSupported ) {
		oldMin = qmc2MainWindow->progressBarGamelist->minimum();
		oldMax = qmc2MainWindow->progressBarGamelist->maximum();
		oldFmt = qmc2MainWindow->progressBarGamelist->format();

          	qmc2MainWindow->tabSoftwareList->setUpdatesEnabled(true);
		labelLoadingSoftwareLists->setText(tr("Loading software-lists, please wait..."));
		labelLoadingSoftwareLists->setVisible(true);
		toolBoxSoftwareList->setVisible(false);
		show();
		qApp->processEvents();

		swlLines.clear();
		swlCacheOkay = false;
		if ( !swlCachePath.isEmpty() ) {
			fileSWLCache.setFileName(swlCachePath);
			if ( fileSWLCache.open(QIODevice::ReadOnly | QIODevice::Text) ) {
				QTextStream ts(&fileSWLCache);
				QString line = ts.readLine();
				line = ts.readLine();
#if defined(QMC2_EMUTYPE_MAME)
				if ( line.startsWith("MAME_VERSION") ) {
					QStringList words = line.split("\t");
					if ( words.count() > 1 ) {
						if ( qmc2Gamelist->emulatorVersion == words[1] )
							swlCacheOkay = true;
					}
				}
#elif defined(QMC2_EMUTYPE_MESS)
				if ( line.startsWith("MESS_VERSION") ) {
					QStringList words = line.split("\t");
					if ( words.count() > 1 ) {
						if ( qmc2Gamelist->emulatorVersion == words[1] )
							swlCacheOkay = true;
					}
				}
#endif
				if ( swlCacheOkay ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML software list data from cache"));
					QTime elapsedTime;
					loadTimer.start();

					if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
						qmc2MainWindow->progressBarGamelist->setFormat(tr("SWL cache - %p%"));
					else
						qmc2MainWindow->progressBarGamelist->setFormat("%p%");
					QFileInfo fi(swlCachePath);
					qmc2MainWindow->progressBarGamelist->setRange(0, fi.size());
					qmc2MainWindow->progressBarGamelist->setValue(0);
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
								line = lines[l];
								swlBuffer += line + "\n";
							}
						}
						if ( endsWithNewLine )
							readBuffer.clear();
						else
							readBuffer = lines.last();
						qmc2MainWindow->progressBarGamelist->setValue(swlBuffer.length());
					}
					qmc2MainWindow->progressBarGamelist->reset();
					qmc2MainWindow->progressBarGamelist->setRange(oldMin, oldMax);
					elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data from cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
					validData = true;
				}
				if ( fileSWLCache.isOpen() )
					fileSWLCache.close();
			}
		} else {
#if defined(QMC2_EMUTYPE_MAME)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: the file name for the MAME software list cache is empty -- please correct this and reload the game list afterwards"));
#elif defined(QMC2_EMUTYPE_MESS)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: the file name for the MESS software list cache is empty -- please correct this and reload the machine list afterwards"));
#endif
			labelLoadingSoftwareLists->setVisible(false);
			toolBoxSoftwareList->setVisible(true);

			isLoading = false;
			return false;
		}
        }

	if ( !swlCacheOkay ) {
          	qmc2MainWindow->tabSoftwareList->setUpdatesEnabled(true);
		labelLoadingSoftwareLists->setVisible(true);
		toolBoxSoftwareList->setVisible(false);
		show();

		loadTimer.start();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML software list data and (re)creating cache"));

		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("SWL data - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");

		if ( !fileSWLCache.open(QIODevice::WriteOnly | QIODevice::Text) ) {
#if defined(QMC2_EMUTYPE_MAME)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open the MAME software list cache for writing, path = %1 -- please check/correct access permissions and reload the game list afterwards").arg(swlCachePath));
#elif defined(QMC2_EMUTYPE_MESS)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open the MESS software list cache for writing, path = %1 -- please check/correct access permissions and reload the machine list afterwards").arg(swlCachePath));
#endif
			isLoading = false;
			return false;
		}

		swlBuffer.clear();
		swlLastLine.clear();

		tsSWLCache.setDevice(&fileSWLCache);
		tsSWLCache.reset();
		tsSWLCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
#if defined(QMC2_EMUTYPE_MAME)
		tsSWLCache << "MAME_VERSION\t" + qmc2Gamelist->emulatorVersion + "\n";
#elif defined(QMC2_EMUTYPE_MESS)
		tsSWLCache << "MESS_VERSION\t" + qmc2Gamelist->emulatorVersion + "\n";
#endif
		
		loadProc = new QProcess(this);

		connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(readyReadStandardError()), this, SLOT(loadReadyReadStandardError()));
		connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
		connect(loadProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(loadStateChanged(QProcess::ProcessState)));

#if defined(QMC2_EMUTYPE_MAME)
		QString command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
		QStringList args;
		args << "-listsoftware";
		QString hashPath = qmc2Config->value("MAME/Configuration/Global/hashpath").toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;
#elif defined(QMC2_EMUTYPE_MESS)
		QString command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
		QStringList args;
		args << "-listsoftware";
		QString hashPath = qmc2Config->value("MESS/Configuration/Global/hashpath").toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;
#endif

		if ( !qmc2StopParser ) {
			validData = true;
			loadFinishedFlag = false;
			loadProc->start(command, args);
			// FIXME: this is blocking the GUI shortly
			if ( loadProc->waitForStarted() && !qmc2StopParser ) {
				while ( !loadFinishedFlag && !qmc2StopParser ) {
					qApp->processEvents();
#if defined(Q_WS_MAC)
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
			return false;
		}
	}

	labelLoadingSoftwareLists->setVisible(false);
	toolBoxSoftwareList->setVisible(true);

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): validData = %1").arg(validData));
#endif

	QString xmlData = getXmlData();

	QStringList softwareList = systemSoftwareListMap[systemName];
	if ( !softwareList.contains("NO_SOFTWARE_LIST") && !interruptLoad ) {
		swlLines = swlBuffer.split("\n");
		foreach (QString swList, softwareList) {
			if ( interruptLoad ) break;
			QString softwareListXml = getSoftwareListXmlData(swList);
			if ( interruptLoad ) break;
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
#ifdef QMC2_DEBUG
				else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): successfully parsed the XML data for software list '%1'").arg(swList));
#endif
			}
		}

		QTimer::singleShot(0, labelLoadingSoftwareLists, SLOT(hide()));
		QTimer::singleShot(0, toolBoxSoftwareList, SLOT(show()));

		// load favorites
#if defined(QMC2_EMUTYPE_MAME)
		QStringList softwareNames = qmc2Config->value(QString("MAME/Favorites/%1/SoftwareNames").arg(systemName)).toStringList();
#elif defined(QMC2_EMUTYPE_MESS)
		QStringList softwareNames = qmc2Config->value(QString("MESS/Favorites/%1/SoftwareNames").arg(systemName)).toStringList();
		QStringList configNames = qmc2Config->value(QString("MESS/Favorites/%1/DeviceConfigs").arg(systemName)).toStringList();
#endif

		QStringList compatFilters = systemSoftwareFilterMap[systemName];
		for (int i = 0; i < softwareNames.count() && !interruptLoad; i++) {
			if ( interruptLoad ) break;
			QString software = softwareNames[i];
			QList<QTreeWidgetItem *> matchedSoftware = treeWidgetKnownSoftware->findItems(software, Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
			QTreeWidgetItem *swItem = NULL;
			if ( matchedSoftware.count() > 0 ) swItem = matchedSoftware.at(0);
			if ( swItem ) {
				SoftwareItem *item = new SoftwareItem(treeWidgetFavoriteSoftware);
				item->setText(QMC2_SWLIST_COLUMN_TITLE, swItem->text(QMC2_SWLIST_COLUMN_TITLE));
				item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, swItem->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
				if ( toolButtonCompatFilterToggle->isChecked() ) {
					QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
					bool showItem = compatList.isEmpty() || compatFilters.isEmpty();
					for (int i = 0; i < compatList.count() && !showItem; i++)
						for (int j = 0; j < compatFilters.count() && !showItem; j++)
							showItem = (compatList[i] == compatFilters[j]);
					item->setHidden(!showItem);
				}
				item->setText(QMC2_SWLIST_COLUMN_NAME, swItem->text(QMC2_SWLIST_COLUMN_NAME));
				item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, swItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
				item->setText(QMC2_SWLIST_COLUMN_YEAR, swItem->text(QMC2_SWLIST_COLUMN_YEAR));
				item->setText(QMC2_SWLIST_COLUMN_PART, swItem->text(QMC2_SWLIST_COLUMN_PART));
				item->setText(QMC2_SWLIST_COLUMN_INTERFACE, swItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
				item->setText(QMC2_SWLIST_COLUMN_LIST, swItem->text(QMC2_SWLIST_COLUMN_LIST));
				SoftwareItem *subItem = new SoftwareItem(item);
				subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
#if defined(QMC2_EMUTYPE_MESS)
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
	treeWidgetFavoriteSoftware->setSortingEnabled(true);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(true);
	treeWidgetSearchResults->setSortingEnabled(true);
	treeWidgetSearchResults->header()->setSortIndicatorShown(true);

	toolButtonReload->setEnabled(true);
	toolButtonToggleSoftwareInfo->setEnabled(true);
	toolButtonCompatFilterToggle->setEnabled(true);

	isLoading = false;
	fullyLoaded = !interruptLoad;
	return true;
}

bool SoftwareList::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::save()");
#endif

	if ( !fullyLoaded )
		return false;

#if defined(QMC2_EMUTYPE_MAME)
	qmc2Config->remove(QString("MAME/Favorites/%1").arg(systemName));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2Config->remove(QString("MESS/Favorites/%1").arg(systemName));
#endif

	QList<QTreeWidgetItem *> itemList = treeWidgetFavoriteSoftware->findItems("*", Qt::MatchWildcard);

	QStringList softwareNames;
#if defined(QMC2_EMUTYPE_MESS)
	QStringList configNames;
	bool onlyEmptyConfigNames = true;
#endif

	foreach (QTreeWidgetItem *item, itemList) {
		softwareNames << item->text(QMC2_SWLIST_COLUMN_NAME);
#if defined(QMC2_EMUTYPE_MESS)
		QString s = item->text(QMC2_SWLIST_COLUMN_DEVICECFG);
		if ( !s.isEmpty() ) onlyEmptyConfigNames = false;
		configNames << s;
#endif
	}

	if ( !softwareNames.isEmpty() ) {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2Config->setValue(QString("MAME/Favorites/%1/SoftwareNames").arg(systemName), softwareNames);
#elif defined(QMC2_EMUTYPE_MESS)
		qmc2Config->setValue(QString("MESS/Favorites/%1/SoftwareNames").arg(systemName), softwareNames);
		if ( onlyEmptyConfigNames )
			qmc2Config->remove(QString("MESS/Favorites/%1/DeviceConfigs").arg(systemName));
		else
			qmc2Config->setValue(QString("MESS/Favorites/%1/DeviceConfigs").arg(systemName), configNames);
#endif
	}

	return true;
}

void SoftwareList::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::closeEvent(e);
}

void SoftwareList::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::hideEvent(e);
}

void SoftwareList::leaveEvent(QEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::leaveEvent(QEvent *e = %1)").arg((qulonglong)e));
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::checkSoftwareSnap()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::updateDetail()");
#endif

	detailUpdateTimer.stop();
	qmc2MainWindow->on_tabWidgetSoftwareDetail_updateCurrent();
}

void SoftwareList::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::resizeEvent(QResizeEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::resizeEvent(e);
}

void SoftwareList::mouseMoveEvent(QMouseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::mouseMoveEvent(QMouseEvent *e = %1)").arg((qulonglong)e));
#endif

	cancelSoftwareSnap();

	QWidget::mouseMoveEvent(e);
}

void SoftwareList::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::showEvent(e);
}

void SoftwareList::loadStarted()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadStarted()"));
#endif

	// we don't know how many items there are...
	loadFinishedFlag = false;
	qmc2MainWindow->progressBarGamelist->setRange(0, 0);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

	if ( exitStatus == QProcess::NormalExit && exitCode == 0 ) {
		validData = true;
	} else {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#elif defined(QMC2_EMUTYPE_MESS)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#endif
		validData = false;
	}
	QTime elapsedTime;
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	loadFinishedFlag = true;

	qmc2MainWindow->progressBarGamelist->setRange(oldMin, oldMax);
	qmc2MainWindow->progressBarGamelist->setFormat(oldFmt);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadReadyReadStandardOutput()"));
#endif

	QString s = swlLastLine + proc->readAllStandardOutput();
#if defined(Q_WS_WIN)
	s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
	QStringList lines = s.split("\n");

	if ( s.endsWith("\n") ) {
		swlLastLine.clear();
	} else {
		swlLastLine = lines.last();
		lines.removeLast();
	}

	foreach (QString line, lines) {
		line = line.trimmed();
		if ( !line.isEmpty() )
			if ( !line.startsWith("<!") && !line.startsWith("<?xml") && !line.startsWith("]>") ) {
				tsSWLCache << line << "\n";
				swlBuffer += line + "\n";
			}
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
#endif
		swlSupported = false;
		if ( fileSWLCache.isOpen() )
			fileSWLCache.close();
		fileSWLCache.remove();
	}
}

void SoftwareList::loadError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

#if defined(QMC2_EMUTYPE_MAME)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists caused an error -- processError = %1").arg(processError));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists caused an error -- processError = %1").arg(processError));
#endif
	validData = false;
	loadFinishedFlag = true;

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif

}

void SoftwareList::on_toolButtonToggleSoftwareInfo_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonToggleSoftwareInfo_clicked(bool checked = %1)").arg(checked));
#endif

	QTreeWidget *treeWidget = NULL;

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

	if ( !treeWidget ) {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		return;
	}

	QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();
	checked &= (selectedItems.count() > 0);

	if ( checked ) {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
		qmc2MainWindow->on_tabWidgetSoftwareDetail_updateCurrent();
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
	}
}

void SoftwareList::on_toolButtonCompatFilterToggle_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonCompatFilterToggle_clicked(bool checked = %1)").arg(checked));
#endif

	QStringList compatFilters = systemSoftwareFilterMap[qmc2SoftwareList->systemName];
	for (int count = 0; count < treeWidgetKnownSoftware->topLevelItemCount(); count++) {
		QTreeWidgetItem *item = treeWidgetKnownSoftware->topLevelItem(count);
		QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
		bool showItem = !checked || compatList.isEmpty() || compatFilters.isEmpty();
		for (int i = 0; i < compatList.count() && !showItem; i++)
			for (int j = 0; j < compatFilters.count() && !showItem; j++)
				showItem = (compatList[i] == compatFilters[j]);
		if ( !showItem ) {
			item->setHidden(true);
			if ( item->isSelected() )
				item->setSelected(false);
		} else
			item->setHidden(false);
	}
	for (int count = 0; count < treeWidgetFavoriteSoftware->topLevelItemCount(); count++) {
		QTreeWidgetItem *item = treeWidgetFavoriteSoftware->topLevelItem(count);
		QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
		bool showItem = !checked || compatList.isEmpty() || compatFilters.isEmpty();
		for (int i = 0; i < compatList.count() && !showItem; i++)
			for (int j = 0; j < compatFilters.count() && !showItem; j++)
				showItem = (compatList[i] == compatFilters[j]);
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
		bool showItem = !checked || compatList.isEmpty() || compatFilters.isEmpty();
		for (int i = 0; i < compatList.count() && !showItem; i++)
			for (int j = 0; j < compatFilters.count() && !showItem; j++)
				showItem = (compatList[i] == compatFilters[j]);
		if ( !showItem ) {
			item->setHidden(true);
			if ( item->isSelected() )
				item->setSelected(false);
		} else
			item->setHidden(false);
	}
}

void SoftwareList::on_toolButtonReload_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonReload_clicked(bool checked = %1)").arg(checked));
#endif

	save();

	treeWidgetKnownSoftware->clear();
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
	comboBoxDeviceConfiguration->setEnabled(false);
	comboBoxDeviceConfiguration->clear();
	comboBoxDeviceConfiguration->insertItem(0, tr("No additional devices"));
	qApp->processEvents();

	QTimer::singleShot(0, this, SLOT(load()));
}

void SoftwareList::on_toolButtonExport_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonExport_clicked(bool checked = %1)").arg(checked));
#endif

	if ( !exporter )
		exporter = new SoftwareListExporter(this);

	exporter->show();
}

void SoftwareList::on_toolButtonAddToFavorites_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonAddToFavorites_clicked(bool checked = %1)").arg(checked));
#endif

	QList<QTreeWidgetItem *> selectedItems;

	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			selectedItems = treeWidgetKnownSoftware->selectedItems();
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

	QStringList compatFilters = systemSoftwareFilterMap[qmc2SoftwareList->systemName];
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
			item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, si->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
			QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
			bool showItem = !checked || compatList.isEmpty() || compatFilters.isEmpty();
			for (int i = 0; i < compatList.count() && !showItem; i++)
				for (int j = 0; j < compatFilters.count() && !showItem; j++)
					showItem = (compatList[i] == compatFilters[j]);
			item->setHidden(!showItem);
			item->setText(QMC2_SWLIST_COLUMN_NAME, si->text(QMC2_SWLIST_COLUMN_NAME));
			item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, si->text(QMC2_SWLIST_COLUMN_PUBLISHER));
			item->setText(QMC2_SWLIST_COLUMN_YEAR, si->text(QMC2_SWLIST_COLUMN_YEAR));
			item->setText(QMC2_SWLIST_COLUMN_PART, si->text(QMC2_SWLIST_COLUMN_PART));
			item->setText(QMC2_SWLIST_COLUMN_INTERFACE, si->text(QMC2_SWLIST_COLUMN_INTERFACE));
			item->setText(QMC2_SWLIST_COLUMN_LIST, si->text(QMC2_SWLIST_COLUMN_LIST));
#if defined(QMC2_EMUTYPE_MESS)
			if ( comboBoxDeviceConfiguration->currentIndex() > QMC2_SWLIST_MSEL_AUTO_MOUNT )
				item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, comboBoxDeviceConfiguration->currentText());
			else
				item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, QString());
#endif
		}
	}

	actionSaveFavoritesToFile->setEnabled(treeWidgetFavoriteSoftware->topLevelItemCount() > 0);
}

void SoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool checked = %1)").arg(checked));
#endif

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

void SoftwareList::on_toolButtonPlay_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonPlay_clicked(bool checked = %1)").arg(checked));
#endif

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
}

void SoftwareList::on_toolButtonPlayEmbedded_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonPlayEmbedded_clicked(bool checked = %1)").arg(checked));
#endif

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));
}

void SoftwareList::treeWidgetKnownSoftware_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetKnownSoftware_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetKnownSoftware->scrollToItem(selectedItems[0]);
}

void SoftwareList::treeWidgetFavoriteSoftware_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetFavoriteSoftware->scrollToItem(selectedItems[0]);
}

void SoftwareList::treeWidgetSearchResults_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetSearchResults->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetSearchResults->scrollToItem(selectedItems[0]);
}

void SoftwareList::on_treeWidgetKnownSoftware_itemExpanded(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetKnownSoftware_itemExpanded(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
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
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list '%1'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)));
		}
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemExpanded(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemExpanded(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
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
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list '%1'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)));
		}
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemExpanded(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_itemExpanded(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
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
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list '%1'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)));
		}
	}
}

void SoftwareList::on_toolBoxSoftwareList_currentChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolBoxSoftwareList_currentChanged(int index = %1)").arg(index));
#endif

	comboBoxDeviceConfiguration->setCurrentIndex(0);
	switch ( index ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			on_treeWidgetKnownSoftware_itemSelectionChanged();
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			on_treeWidgetFavoriteSoftware_itemSelectionChanged();
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			on_treeWidgetSearchResults_itemSelectionChanged();
			break;
		default:
			break;
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetKnownSoftware_itemSelectionChanged()");
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	bool enable = (selectedItems.count() > 0);
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
	if ( enable && qmc2SoftwareSnap ) {
		SoftwareItem *item = (SoftwareItem *)selectedItems[0];
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
		if ( toolButtonToggleSoftwareInfo->isChecked() )
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
		else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		}
		currentItem = item;
		while ( currentItem->parent() ) currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		cancelSoftwareSnap();
		currentItem = NULL;
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemSelectionChanged()");
#endif

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
		if ( toolButtonToggleSoftwareInfo->isChecked() )
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
		else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		}
		currentItem = item;
		while ( currentItem->parent() ) currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetSearchResults_itemSelectionChanged()");
#endif

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
		if ( toolButtonToggleSoftwareInfo->isChecked() )
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
		else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		}
		currentItem = item;
		while ( currentItem->parent() ) currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		cancelSoftwareSnap();
		currentItem = NULL;
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p = ...)");
#endif

	cancelSoftwareSnap();

	QTreeWidgetItem *item = treeWidgetKnownSoftware->itemAt(p);
	if ( !item )
		return;

	treeWidgetKnownSoftware->setItemSelected(item, true);
	actionAddToFavorites->setVisible(true);
	actionRemoveFromFavorites->setVisible(false);
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetKnownSoftware->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &p = ...)");
#endif

	cancelSoftwareSnap();

	QTreeWidgetItem *item = treeWidgetFavoriteSoftware->itemAt(p);
	if ( !item )
		return;

	treeWidgetFavoriteSoftware->setItemSelected(item, true);
	actionAddToFavorites->setVisible(false);
	actionRemoveFromFavorites->setVisible(true);
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetFavoriteSoftware->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &p = ...)");
#endif

	cancelSoftwareSnap();

	QTreeWidgetItem *item = treeWidgetSearchResults->itemAt(p);
	if ( !item )
		return;

	treeWidgetSearchResults->setItemSelected(item, true);
	actionAddToFavorites->setVisible(true);
	actionRemoveFromFavorites->setVisible(false);
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSearchResults->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::cancelSoftwareSnap()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::cancelSoftwareSnap()");
#endif

	snapForced = false;
	snapTimer.stop();
	if ( qmc2SoftwareSnap ) {
		qmc2SoftwareSnap->myItem = NULL;
		qmc2SoftwareSnap->hide();
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_itemEntered(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetKnownSoftware_itemEntered(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

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

void SoftwareList::on_treeWidgetFavoriteSoftware_itemEntered(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemEntered(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

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

void SoftwareList::on_treeWidgetSearchResults_itemEntered(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_itemEntered(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

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

void SoftwareList::on_treeWidgetKnownSoftware_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetKnownSoftware_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
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

void SoftwareList::on_treeWidgetKnownSoftware_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
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

void SoftwareList::on_treeWidgetFavoriteSoftware_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
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

void SoftwareList::on_treeWidgetSearchResults_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_comboBoxSearch_editTextChanged(const QString &)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_comboBoxSearch_editTextChanged(const QString &)");
#endif

	searchTimer.start(QMC2_SEARCH_DELAY);
}

void SoftwareList::comboBoxSearch_editTextChanged_delayed()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::comboBoxSearch_editTextChanged_delayed()");
#endif

	searchTimer.stop();

	QString pattern = comboBoxSearch->currentText();

	// easy pattern match
	if ( !pattern.isEmpty() ) {
		pattern = "*" + pattern.replace(' ', "* *") + "*";
		pattern.replace(QString("*^"), "");
		pattern.replace(QString("$*"), "");
	}

	treeWidgetSearchResults->clear();

	QList<QTreeWidgetItem *> matches = treeWidgetKnownSoftware->findItems(pattern, Qt::MatchContains | Qt::MatchWildcard, QMC2_SWLIST_COLUMN_TITLE);
	QList<QTreeWidgetItem *> matchesByShortName = treeWidgetKnownSoftware->findItems(pattern, Qt::MatchContains | Qt::MatchWildcard, QMC2_SWLIST_COLUMN_NAME);

	int i;

	for (i = 0; i < matchesByShortName.count(); i++) {
		QTreeWidgetItem *item = matchesByShortName[i];
		if ( !matches.contains(item) )
			matches.append(item);
	}

	QStringList compatFilters = systemSoftwareFilterMap[qmc2SoftwareList->systemName];
	for (i = 0; i < matches.count(); i++) {
		SoftwareItem *item = new SoftwareItem(treeWidgetSearchResults);
		SoftwareItem *subItem = new SoftwareItem(item);
		subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
		QTreeWidgetItem *matchItem = matches.at(i);
		item->setText(QMC2_SWLIST_COLUMN_TITLE, matchItem->text(QMC2_SWLIST_COLUMN_TITLE));
		item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, matchItem->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
		if ( qmc2SoftwareList->toolButtonCompatFilterToggle->isChecked() ) {
			QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
			bool showItem = compatList.isEmpty() || compatFilters.isEmpty();
			for (int i = 0; i < compatList.count() && !showItem; i++)
				for (int j = 0; j < compatFilters.count() && !showItem; j++)
					showItem = (compatList[i] == compatFilters[j]);
			item->setHidden(!showItem);
		}
		item->setText(QMC2_SWLIST_COLUMN_NAME, matchItem->text(QMC2_SWLIST_COLUMN_NAME));
		item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, matchItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
		item->setText(QMC2_SWLIST_COLUMN_YEAR, matchItem->text(QMC2_SWLIST_COLUMN_YEAR));
		item->setText(QMC2_SWLIST_COLUMN_PART, matchItem->text(QMC2_SWLIST_COLUMN_PART));
		item->setText(QMC2_SWLIST_COLUMN_INTERFACE, matchItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
		item->setText(QMC2_SWLIST_COLUMN_LIST, matchItem->text(QMC2_SWLIST_COLUMN_LIST));
	}

	if ( autoSelectSearchItem ) {
  		treeWidgetSearchResults->setFocus();
		if ( treeWidgetSearchResults->currentItem() )
			treeWidgetSearchResults->currentItem()->setSelected(true);
	}

	autoSelectSearchItem = false;
}

void SoftwareList::on_comboBoxSearch_activated(const QString &pattern)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_comboBoxSearch_activated(const QString &pattern = %1)").arg(pattern));
#endif

	autoSelectSearchItem = true;
	comboBoxSearch_editTextChanged_delayed();
}

QStringList &SoftwareList::arguments()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::arguments()");
#endif

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
			treeWidget = treeWidgetKnownSoftware;
			break;
	}

	QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();

	if ( selectedItems.count() > 0 ) {
		QTreeWidgetItemIterator it(treeWidget);
		QStringList manualMounts;
		if ( !autoMounted ) {
			// manually mounted
			while ( *it ) {
				QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
				if ( comboBox ) {
					if ( comboBox->currentIndex() > QMC2_SWLIST_MSEL_DONT_MOUNT ) {
						swlArgs << QString("-%1").arg(comboBox->currentText());
						QTreeWidgetItem *item = *it;
						while ( item->parent() ) item = item->parent();
						swlArgs << QString("%1:%2:%3").arg((*it)->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)).arg((*it)->text(QMC2_SWLIST_COLUMN_PART));
					}
				}
				it++;
			}
		} else {
			// automatically mounted
			QTreeWidgetItem *item = selectedItems[0];
			while ( item->parent() ) item = item->parent();
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
		}
	}

#if defined(QMC2_EMUTYPE_MESS)
	// optionally add arguments for the selected device configuration
	QString devConfigName = comboBoxDeviceConfiguration->currentText();
	if ( devConfigName != tr("No additional devices") ) {
		qmc2Config->beginGroup(QString("MESS/Configuration/Devices/%1/%2").arg(systemName).arg(devConfigName));
		QStringList instances = qmc2Config->value("Instances").toStringList();
		QStringList files = qmc2Config->value("Files").toStringList();
		QStringList slotNames = qmc2Config->value("Slots").toStringList();
		QStringList slotOptions = qmc2Config->value("SlotOptions").toStringList();
		qmc2Config->endGroup();
		for (int i = 0; i < instances.count(); i++) {
#if defined(Q_WS_WIN)
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace('/', '\\');
#else
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace("~", "$HOME");
#endif
		}
		for (int i = 0; i < slotNames.count(); i++)
			swlArgs << QString("-%1").arg(slotNames[i]) << slotOptions[i];
	}
#endif

	return swlArgs;
}

void SoftwareList::treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuKnownSoftwareHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetKnownSoftware->header()->viewport()->mapToGlobal(p), menuKnownSoftwareHeader));
	menuKnownSoftwareHeader->show();
}

void SoftwareList::actionKnownSoftwareHeader_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::actionKnownSoftwareHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetKnownSoftware->columnCount(); i++) if ( !treeWidgetKnownSoftware->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetKnownSoftware->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetKnownSoftware->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void SoftwareList::treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuFavoriteSoftwareHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetFavoriteSoftware->header()->viewport()->mapToGlobal(p), menuFavoriteSoftwareHeader));
	menuFavoriteSoftwareHeader->show();
}

void SoftwareList::actionFavoriteSoftwareHeader_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::actionFavoriteSoftwareHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetFavoriteSoftware->columnCount(); i++) if ( !treeWidgetFavoriteSoftware->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuSearchResultsHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSearchResults->header()->viewport()->mapToGlobal(p), menuSearchResultsHeader));
	menuSearchResultsHeader->show();
}

void SoftwareList::actionSearchResultsHeader_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::actionSearchResultsHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetSearchResults->columnCount(); i++) if ( !treeWidgetSearchResults->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
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

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::checkMountDeviceSelection()");
#endif

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
	} else if ( mountDevice == QObject::tr("Don't mount") ) {
		while ( *it ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				if ( comboBox == comboBoxSender )
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				else if ( comboBox->currentIndex() == QMC2_SWLIST_MSEL_AUTO_MOUNT ) {
					comboBox->blockSignals(true);
					comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // => don't mount
					comboBox->blockSignals(false);
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				}
			}
			it++;
		}
		autoMounted = false;
	} else {
		while ( *it ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				if ( comboBox != comboBoxSender ) {
					if ( comboBox->currentText() == mountDevice || comboBox->currentText() == QObject::tr("Auto mount") ) {
						comboBox->blockSignals(true);
						comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // => don't mount
						comboBox->blockSignals(false);
						(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
					}
				} else
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + mountDevice);
			}
			it++;
		}
		autoMounted = false;
	}
}

void SoftwareList::loadFavoritesFromFile()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::loadFavoritesFromFile()");
#endif

	QString proposedName = systemName + ".fav";

	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath") )
		proposedName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath").toString());

	QString filePath = QFileDialog::getOpenFileName(this, tr("Choose file to merge favorites from"), proposedName, tr("All files (*)"));

	if ( !filePath.isEmpty() ) {
		QFileInfo fiFilePath(filePath);
		QString storagePath = fiFilePath.absolutePath();
		if ( !storagePath.endsWith("/") ) storagePath.append("/");
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath", storagePath);

		// import software-list favorites
		QFile favoritesFile(filePath);
		QStringList compatFilters = systemSoftwareFilterMap[systemName];
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
								if ( toolButtonCompatFilterToggle->isChecked() ) {
									QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
									bool showItem = compatList.isEmpty() || compatFilters.isEmpty();
									for (int i = 0; i < compatList.count() && !showItem; i++)
										for (int j = 0; j < compatFilters.count() && !showItem; j++)
											showItem = (compatList[i] == compatFilters[j]);
									item->setHidden(!showItem);
								}
								item->setText(QMC2_SWLIST_COLUMN_NAME, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_NAME));
								item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
								item->setText(QMC2_SWLIST_COLUMN_YEAR, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_YEAR));
								item->setText(QMC2_SWLIST_COLUMN_PART, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_PART));
								item->setText(QMC2_SWLIST_COLUMN_INTERFACE, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
								item->setText(QMC2_SWLIST_COLUMN_LIST, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_LIST));
#if defined(QMC2_EMUTYPE_MESS)
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::saveFavoritesToFile()");
#endif

	QString proposedName = systemName + ".fav";

	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath") )
		proposedName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath").toString());

	QString filePath = QFileDialog::getSaveFileName(this, tr("Choose file to store favorites to"), proposedName, tr("All files (*)"));

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
#endif
			for (int i = 0; i < treeWidgetFavoriteSoftware->topLevelItemCount(); i++) {
				QTreeWidgetItem *item = treeWidgetFavoriteSoftware->topLevelItem(i);
				if ( item ) {
					ts << item->text(QMC2_SWLIST_COLUMN_LIST) << "\t" << item->text(QMC2_SWLIST_COLUMN_NAME);
#if defined(QMC2_EMUTYPE_MESS)
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

SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent = %1)").arg((qulonglong)parent));
#endif

	parentTreeWidget = parent;
	elementCounter = 0;
}

SoftwareListXmlHandler::~SoftwareListXmlHandler()
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareListXmlHandler::~SoftwareListXmlHandler()");
#endif

}

bool SoftwareListXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::startElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2, const QXmlAttributes &attributes = ...)").arg(localName).arg(qName));
#endif

	if ( qmc2SoftwareList->interruptLoad )
		return false;

	if ( ++elementCounter % QMC2_SWLIST_LOAD_RESPONSE == 0 )
		qApp->processEvents();

	if ( qName == "softwarelist" ) {
		softwareListName = attributes.value("name");
		compatFilters = systemSoftwareFilterMap[qmc2SoftwareList->systemName];
	} else if ( qName == "software" ) {
		softwareName = attributes.value("name");
		softwareItem = new SoftwareItem(parentTreeWidget);
		SoftwareItem *subItem = new SoftwareItem(softwareItem);
		subItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Waiting for data..."));
		softwareItem->setText(QMC2_SWLIST_COLUMN_NAME, softwareName);
		softwareItem->setText(QMC2_SWLIST_COLUMN_LIST, softwareListName);
	} else if ( qName == "part" ) {
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
	} else if ( qName == "feature" ) {
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
					softwareItem->setHidden(!showItem);
				}
			}
		}
	} else if ( qName == "description" || qName == "year" || qName == "publisher" ) {
		currentText.clear();
	}

	return true;
}

bool SoftwareListXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::endElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2)").arg(localName).arg(qName));
#endif

	if ( qmc2SoftwareList->interruptLoad )
		return false;

	if ( qName == "description" ) {
		softwareTitle = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_TITLE, softwareTitle);
	} else if ( qName == "year" ) {
		softwareYear = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_YEAR, softwareYear);
	} else if ( qName == "publisher" ) {
		softwarePublisher = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, softwarePublisher);
	}

	return true;
}

bool SoftwareListXmlHandler::characters(const QString &str)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::characters(const QString &str = ...)"));
#endif

	currentText += QString::fromUtf8(str.toAscii());
	return true;
}

SoftwareSnap::SoftwareSnap(QWidget *parent)
	: QWidget(parent, Qt::ToolTip)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::SoftwareSnap(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	setFocusPolicy(Qt::NoFocus);
	snapForcedResetTimer.setSingleShot(true);
	connect(&snapForcedResetTimer, SIGNAL(timeout()), this, SLOT(resetSnapForced()));

	snapFile = NULL;
	ctxMenuRequested = false;
	contextMenu = new QMenu(this);
	contextMenu->hide();
	
	QString s;
	QAction *action;

	s = tr("Copy to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
	s = tr("Refresh");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));
}

SoftwareSnap::~SoftwareSnap()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::~SoftwareSnap()");
#endif

	if ( qmc2UseSoftwareSnapFile && snapFile ) {
		unzClose(snapFile);
		snapFile = NULL;
	}
}

void SoftwareSnap::mousePressEvent(QMouseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::mousePressEvent(QMouseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e->button() != Qt::RightButton)
		hide();
	else
		ctxMenuRequested = true;
}

void SoftwareSnap::enterEvent(QEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::enterEvent(QEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( contextMenu->isVisible() )
		QTimer::singleShot(0, contextMenu, SLOT(hide()));
	ctxMenuRequested = false;

	QWidget::enterEvent(e);
}

void SoftwareSnap::leaveEvent(QEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::leaveEvent(QEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2SoftwareList->snapForced && !ctxMenuRequested ) {
		myItem = NULL;
		hide();
	}  else if ( !qmc2SoftwareList->snapForced )
		QTimer::singleShot(QMC2_SWSNAP_UNFORCE_DELAY, qmc2SoftwareList, SLOT(checkSoftwareSnap()));

	ctxMenuRequested = contextMenu->isVisible();

	QWidget::leaveEvent(e);
}

void SoftwareSnap::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.eraseRect(rect());
	p.end();
}

void SoftwareSnap::loadSnapshot()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::loadSnapshot()");
#endif

	ctxMenuRequested = false;

	if ( !qmc2SoftwareList || qmc2SoftwareSnapPosition == QMC2_SWSNAP_POS_DISABLE_SNAPS ) {
		myItem = NULL;
		resetSnapForced();
		return;
	}

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AutoDisableSoftwareSnap", true).toBool() ) {
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE || (qmc2MainWindow->tabWidgetSoftwareDetail->parent() == qmc2MainWindow && qmc2MainWindow->tabWidgetSoftwareDetail->isVisible()) ) {
			myItem = NULL;
			resetSnapForced();
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
					rect = qmc2SoftwareList->treeWidgetKnownSoftware->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomLeft());
					treeWidget = qmc2SoftwareList->treeWidgetKnownSoftware;
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
				return;
			}
	}

	listName = item->text(QMC2_SWLIST_COLUMN_LIST);
	entryName = item->text(QMC2_SWLIST_COLUMN_NAME);
	myItem = (SoftwareItem *)item;

	QPixmap pm;
	bool pmLoaded = QPixmapCache::find("sws_" + listName + "_" + entryName, &pm);

	if ( !pmLoaded ) {
		if ( qmc2UseSoftwareSnapFile ) {
			// try loading image from ZIP
			if ( !snapFile ) {
#if defined(QMC2_EMUTYPE_MAME)
				snapFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/SoftwareSnapFile").toString().toAscii());
				if ( snapFile == NULL )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(qmc2Config->value("MAME/FilesAndDirectories/SoftwareSnapFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
				snapFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/SoftwareSnapFile").toString().toAscii());
				if ( snapFile == NULL )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(qmc2Config->value("MESS/FilesAndDirectories/SoftwareSnapFile").toString()));
#endif
			}
			if ( snapFile ) {
				bool fileOk = true;
				QByteArray imageData;
				QString pathInZip = listName + "/" + entryName + ".png";
				if ( unzLocateFile(snapFile, (const char *)pathInZip.toAscii(), 0) == UNZ_OK ) {
					if ( unzOpenCurrentFile(snapFile) == UNZ_OK ) {
						char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
						int len;
						while ( (len = unzReadCurrentFile(snapFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
							for (int i = 0; i < len; i++)
								imageData += imageBuffer[i];
						}
						unzCloseCurrentFile(snapFile);
					} else
						fileOk = false;
				} else
					fileOk = false;
				if ( fileOk ) {
					if ( pm.loadFromData(imageData, "PNG") ) {
						pmLoaded = true;
						QPixmapCache::insert("sws_" + listName + "_" + entryName, pm);
					}
				}
			}
		} else {
			// try loading image from folder
#if defined(QMC2_EMUTYPE_MAME)
			QDir snapDir(qmc2Config->value("MAME/FilesAndDirectories/SoftwareSnapDirectory").toString() + "/" + listName);
#elif defined(QMC2_EMUTYPE_MESS)
			QDir snapDir(qmc2Config->value("MESS/FilesAndDirectories/SoftwareSnapDirectory").toString() + "/" + listName);
#endif
			if ( snapDir.exists(entryName + ".png") ) {
				QString filePath = snapDir.absoluteFilePath(entryName + ".png");
				if ( pm.load(filePath) ) {
					pmLoaded = true;
					QPixmapCache::insert("sws_" + listName + "_" + entryName, pm); 
				}
			}
		}
	}

	if ( pmLoaded ) {
		resize(pm.size());
		switch ( qmc2SoftwareSnapPosition ) {
			case QMC2_SWSNAP_POS_ABOVE_CENTER:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
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
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().bottomRight()).x() - width());
						position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
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
		QPainter p;
		p.begin(&pm);
		p.setPen(QPen(QColor(0, 0, 0, 64), 1));
		rect = pm.rect();
		rect.setWidth(rect.width() - 1);
		rect.setHeight(rect.height() - 1);
		p.drawRect(rect);
		p.end();
		pal.setBrush(QPalette::Window, pm);
		setPalette(pal);
		showNormal();
		update();
		raise();
		snapForcedResetTimer.start(QMC2_SWSNAP_UNFORCE_DELAY);
		myCacheKey = "sws_" + listName + "_" + entryName;
	} else {
		myItem = NULL;
		resetSnapForced();
		qmc2SoftwareList->cancelSoftwareSnap();
	}
}

void SoftwareSnap::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		QPixmapCache::remove(myCacheKey);
		repaint();
	}
}

void SoftwareSnap::resetSnapForced()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::resetSnapForced()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

	ctxMenuRequested = true;
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

void SoftwareSnap::copyToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::copyToClipboard()");
#endif

	QPixmap pm(size());
	render(&pm);
	qApp->clipboard()->setPixmap(pm);
}

SoftwareEntryXmlHandler::SoftwareEntryXmlHandler(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::SoftwareEntryXmlHandler(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	parentTreeWidgetItem = (SoftwareItem *)item;
	softwareName = parentTreeWidgetItem->text(QMC2_SWLIST_COLUMN_NAME);
	softwareValid = success = false;
	partItem = dataareaItem = romItem = NULL;
	elementCounter = animSequenceCounter = 0;
}

SoftwareEntryXmlHandler::~SoftwareEntryXmlHandler()
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareEntryXmlHandler::~SoftwareEntryXmlHandler()");
#endif

}

bool SoftwareEntryXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::startElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2, const QXmlAttributes &attributes = ...)").arg(localName).arg(qName));
#endif

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
			if ( featureName == "part id" ) {
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

	return true;
}

bool SoftwareEntryXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::endElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2)").arg(localName).arg(qName));
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnapshot::SoftwareSnapshot(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	contextMenu = new QMenu(this);
	contextMenu->hide();

	QString s;
	QAction *action;

	s = tr("Copy to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
	s = tr("Refresh");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));
}

SoftwareSnapshot::~SoftwareSnapshot()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnapshot::~SoftwareSnapshot()");
#endif

	if ( qmc2UseSoftwareSnapFile && qmc2SoftwareSnap->snapFile ) {
		unzClose(qmc2SoftwareSnap->snapFile);
		qmc2SoftwareSnap->snapFile = NULL;
	}
}

void SoftwareSnapshot::paintEvent(QPaintEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnapshot::paintEvent(QPaintEvent *e = %1)").arg((qulonglong)e));
#endif

	QPainter p(this);

	if ( !qmc2SoftwareList->currentItem ) {
		drawCenteredImage(0, &p); // clear snapshot widget
		return;
	}

	QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
	QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);

	if ( !QPixmapCache::find("sws_" + listName + "_" + entryName, &currentSnapshotPixmap) )
		loadSnapshot(listName, entryName);

	drawScaledImage(&currentSnapshotPixmap, &p);
}

void SoftwareSnapshot::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		QPixmapCache::remove(myCacheKey);
		repaint();
	}
}

bool SoftwareSnapshot::loadSnapshot(QString listName, QString entryName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnapshot::loadSnapshot(QString listName = %1, QString entryName = %2)").arg(listName).arg(entryName));
#endif

	QPixmap pm;

	bool fileOk = true;
	if ( qmc2UseSoftwareSnapFile ) {
		// try loading image from ZIP
		if ( !qmc2SoftwareSnap->snapFile ) {
#if defined(QMC2_EMUTYPE_MAME)
			qmc2SoftwareSnap->snapFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/SoftwareSnapFile").toString().toAscii());
			if ( qmc2SoftwareSnap->snapFile == NULL )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(qmc2Config->value("MAME/FilesAndDirectories/SoftwareSnapFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
			qmc2SoftwareSnap->snapFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/SoftwareSnapFile").toString().toAscii());
			if ( qmc2SoftwareSnap->snapFile == NULL )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(qmc2Config->value("MESS/FilesAndDirectories/SoftwareSnapFile").toString()));
#endif
		}

		if ( qmc2SoftwareSnap->snapFile ) {
			QByteArray imageData;
			QString pathInZip = listName + "/" + entryName + ".png";
			if ( unzLocateFile(qmc2SoftwareSnap->snapFile, (const char *)pathInZip.toAscii(), 0) == UNZ_OK ) {
				if ( unzOpenCurrentFile(qmc2SoftwareSnap->snapFile) == UNZ_OK ) {
					char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
					int len;
					while ( (len = unzReadCurrentFile(qmc2SoftwareSnap->snapFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
						for (int i = 0; i < len; i++)
							imageData += imageBuffer[i];
					}
					unzCloseCurrentFile(qmc2SoftwareSnap->snapFile);
				} else
					fileOk = false;
			} else
				fileOk = false;
			if ( fileOk ) {
				if ( pm.loadFromData(imageData, "PNG") )
					QPixmapCache::insert("sws_" + listName + "_" + entryName, pm);
				else
					fileOk = false;
			}
		}
	} else {
		// try loading image from folder
#if defined(QMC2_EMUTYPE_MAME)
		QDir snapDir(qmc2Config->value("MAME/FilesAndDirectories/SoftwareSnapDirectory").toString() + "/" + listName);
#elif defined(QMC2_EMUTYPE_MESS)
		QDir snapDir(qmc2Config->value("MESS/FilesAndDirectories/SoftwareSnapDirectory").toString() + "/" + listName);
#endif
		if ( snapDir.exists(entryName + ".png") ) {
			QString filePath = snapDir.absoluteFilePath(entryName + ".png");
			if ( pm.load(filePath) ) {
				fileOk = true;
				QPixmapCache::insert("sws_" + listName + "_" + entryName, pm); 
			} else
				fileOk = false;
		}
	}

	if ( !fileOk ) {
		if ( !qmc2RetryLoadingImages )
			QPixmapCache::insert("sws_" + listName + "_"+ entryName, qmc2MainWindow->qmc2GhostImagePixmap);
		currentSnapshotPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
        } else
		currentSnapshotPixmap = pm;

	myCacheKey = "sws_" + listName + "_" + entryName;

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
		p->setPen(QPen(QColor(255, 255, 255, 0)));
		p->fillRect(r, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
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

void SoftwareSnapshot::contextMenuEvent(QContextMenuEvent *e)
{
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}
