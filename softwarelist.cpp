#include <QFileInfo>
#include <QPainter>

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

QMap<QString, QStringList> systemSoftwareListMap;
QMap<QString, QString> softwareListXmlDataCache;
QString swlBuffer;
QString swlLastLine;
bool swlSupported = true;

SoftwareList::SoftwareList(QString sysName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::SoftwareList(QString sysName = %1, QWidget *parent = %2)").arg(sysName).arg((qulonglong)parent));
#endif

	setupUi(this);

	systemName = sysName;
	loadProc = NULL;
	validData = false;

#if defined(QMC2_EMUTYPE_MAME)
	comboBoxDeviceConfiguration->setVisible(false);
	QString altText = tr("Add the currently selected software to the favorites list");
	toolButtonAddToFavorites->setToolTip(altText); toolButtonAddToFavorites->setStatusTip(altText);
#elif defined(QMC2_EMUTYPE_MESS)
	horizontalLayout->removeItem(horizontalSpacer);
#endif

	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonAddToFavorites->setIconSize(iconSize);
	toolButtonRemoveFromFavorites->setIconSize(iconSize);
	toolButtonPlay->setIconSize(iconSize);
	toolButtonReload->setIconSize(iconSize);
#if defined(Q_WS_X11)
	toolButtonPlayEmbedded->setIconSize(iconSize);
#else
	toolButtonPlayEmbedded->setVisible(false);
	gridLayout->removeWidget(toolButtonPlay);
	gridLayout->addWidget(toolButtonPlay, 0, 4, 1, 2);
#endif
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/flat.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_FAVORITES_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/favorites.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_SEARCH_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/hint.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

	toolBoxSoftwareList->setEnabled(false);
	toolButtonAddToFavorites->setEnabled(false);
	toolButtonRemoveFromFavorites->setEnabled(false);
	toolButtonPlay->setEnabled(false);
	toolButtonPlayEmbedded->setEnabled(false);
	comboBoxDeviceConfiguration->setEnabled(false);

	// software list context menu
	softwareListMenu = new QMenu(this);
	QString s = tr("Play selected software");
	QAction *action = softwareListMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playActivated()));
#if defined(Q_WS_X11)
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
	actionAddToFavorites->setIcon(QIcon(QString::fromUtf8(":/data/img/plus.png")));
	connect(actionAddToFavorites, SIGNAL(triggered()), this, SLOT(addToFavorites()));
	s = tr("Remove from favorite software list");
	actionRemoveFromFavorites = softwareListMenu->addAction(tr("&Remove from favorites"));
	actionRemoveFromFavorites->setToolTip(s); actionRemoveFromFavorites->setStatusTip(s);
	actionRemoveFromFavorites->setIcon(QIcon(QString::fromUtf8(":/data/img/minus.png")));
	connect(actionRemoveFromFavorites, SIGNAL(triggered()), this, SLOT(removeFromFavorites()));

	// restore widget states
	treeWidgetKnownSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState").toByteArray());
	treeWidgetFavoriteSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState").toByteArray());
	treeWidgetSearchResults->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState").toByteArray());
	toolBoxSoftwareList->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex").toInt());

	connect(treeWidgetKnownSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetKnownSoftware_headerSectionClicked(int)));
	connect(treeWidgetFavoriteSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetFavoriteSoftware_headerSectionClicked(int)));
	connect(treeWidgetSearchResults->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetSearchResults_headerSectionClicked(int)));
}

SoftwareList::~SoftwareList()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::~SoftwareList()");
#endif

	// save widget states
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState", treeWidgetKnownSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState", treeWidgetFavoriteSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState", treeWidgetSearchResults->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex", toolBoxSoftwareList->currentIndex());
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
		while ( !swlLines[i].startsWith(s) && i < swlLinesMax ) i++;
		softwareListXmlBuffer = "<?xml version=\"1.0\"?>\n";
		while ( !swlLines[i].startsWith("</softwarelist>") && i < swlLinesMax )
			softwareListXmlBuffer += swlLines[i++].simplified() + "\n";
		softwareListXmlBuffer += "</softwarelist>";
		if ( i < swlLinesMax ) {
			softwareListXmlDataCache[listName] = softwareListXmlBuffer;
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: software list '%1' not found").arg(listName));
	}

	return softwareListXmlBuffer;
}

QString &SoftwareList::getXmlData(QString machineName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::getXmlData(QString machineName = %1)").arg(machineName));
#endif

	static QString xmlBuffer;

	QStringList softwareList = systemSoftwareListMap[systemName];
	if ( softwareList.isEmpty() ) {
		int i = 0;
#if defined(QMC2_EMUTYPE_MAME)
		QString s = "<game name=\"" + systemName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</game>") ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				softwareList << line.mid(startIndex, endIndex - startIndex); 
			}
		}
#elif defined(QMC2_EMUTYPE_MESS)
		QString s = "<machine name=\"" + systemName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				softwareList << line.mid(startIndex, endIndex - startIndex); 
			}
		}
#endif
		if ( softwareList.isEmpty() )
			softwareList << "NO_SOFTWARE_LIST";
		systemSoftwareListMap[systemName] = softwareList;
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListMap[%1] = %2").arg(systemName).arg(softwareList.join(", ")));
#endif
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
	validData = swlSupported;
#if defined(QMC2_EMUTYPE_MAME)
	QString swlCachePath = qmc2Config->value("MAME/FilesAndDirectories/SoftwareListCache").toString();
#elif defined(QMC2_EMUTYPE_MESS)
	QString swlCachePath = qmc2Config->value("MESS/FilesAndDirectories/SoftwareListCache").toString();
#endif

	treeWidgetKnownSoftware->clear();
	treeWidgetFavoriteSoftware->clear();
	treeWidgetSearchResults->clear();

	treeWidgetKnownSoftware->setSortingEnabled(false);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(false);
	treeWidgetFavoriteSoftware->setSortingEnabled(false);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(false);
	treeWidgetSearchResults->setSortingEnabled(false);
	treeWidgetSearchResults->header()->setSortIndicatorShown(false);

	if ( swlBuffer.isEmpty() && swlSupported ) {
		swlLines.clear();
		validData = false;
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
					elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data from cache, elapsed time = %1").arg(elapsedTime.toString("mm:ss.zzz")));
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
			return false;
		}
        }

	if ( !swlCacheOkay ) {
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

		loadProc->start(command, args);

		if ( loadProc ) {
			if ( loadProc->waitForStarted() ) {
				while ( loadProc->state() == QProcess::Running ) {
					loadProc->waitForFinished(100);
					qApp->processEvents();
				}
			} else
				validData = false;
		}

		if ( !validData ) {
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (no data available)"));
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (no data available)"));
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (no data available)"));
			return false;
		}
	}

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): validData = %1").arg(validData ? "true" : "false"));
#endif

	QString xmlData = getXmlData(systemName);

	QStringList softwareList = systemSoftwareListMap[systemName];
	if ( !softwareList.contains("NO_SOFTWARE_LIST") ) {
		swlLines = swlBuffer.split("\n");
		foreach (QString swList, softwareList) {
			QString softwareListXml = getSoftwareListXmlData(swList);
#ifdef QMC2_DEBUG
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): XML data for software list '%1' follows:\n%2").arg(swList).arg(softwareListXml));
#endif
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			SoftwareListXmlHandler xmlHandler(treeWidgetKnownSoftware);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			if ( !xmlReader.parse(xmlInputSource) )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list '%1'").arg(swList));
#ifdef QMC2_DEBUG
			else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): successfully parsed the XML data for software list '%1'").arg(swList));
#endif
		}
	}

	treeWidgetKnownSoftware->setSortingEnabled(true);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(true);
	treeWidgetFavoriteSoftware->setSortingEnabled(true);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(true);
	treeWidgetSearchResults->setSortingEnabled(true);
	treeWidgetSearchResults->header()->setSortIndicatorShown(true);

	return true;
}

bool SoftwareList::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::save()");
#endif

	// FIXME: save favorites here...
	return true;
}

void SoftwareList::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e )
		e->accept();
}

void SoftwareList::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e )
		e->accept();
}

void SoftwareList::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e )
		e->accept();
}

void SoftwareList::loadStarted()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadStarted()"));
#endif

	// we don't know how many items there are...
	qmc2MainWindow->progressBarGamelist->setRange(0, 0);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

	QTime elapsedTime;
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
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
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
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
	QProcess *proc = (QProcess *)sender();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

#if defined(QMC2_EMUTYPE_MAME)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists caused an error -- processError = %1").arg(processError));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists caused an error -- processError = %1").arg(processError));
#endif
	validData = false;

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
	QProcess *proc = (QProcess *)sender();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif

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
	toolButtonPlay->setEnabled(false);
	toolButtonPlayEmbedded->setEnabled(false);
	comboBoxDeviceConfiguration->setEnabled(false);
	comboBoxDeviceConfiguration->clear();
	comboBoxDeviceConfiguration->insertItem(0, tr("No additional devices"));

	QTimer::singleShot(0, this, SLOT(load()));
}

void SoftwareList::on_toolButtonAddToFavorites_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonAddToFavorites_clicked(bool checked = %1)").arg(checked));
#endif

}

void SoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool checked = %1)").arg(checked));
#endif

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
}

void SoftwareList::on_treeWidgetSearchResults_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetSearchResults_itemSelectionChanged()");
#endif

}

void SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p = ...)");
#endif

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

	QTreeWidgetItem *item = treeWidgetSearchResults->itemAt(p);

	if ( !item )
		return;

	treeWidgetSearchResults->setItemSelected(item, true);
	actionAddToFavorites->setVisible(true);
	actionRemoveFromFavorites->setVisible(false);
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSearchResults->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

QStringList &SoftwareList::arguments()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::arguments()");
#endif

	static QStringList swlArgs;

	swlArgs.clear();

	// arguments to start a software list entry
	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	if ( selectedItems.count() > 0 ) {
		QTreeWidgetItem *item = selectedItems[0];
		foreach (QString device, item->text(QMC2_SWLIST_COLUMN_PART).split(",")) {
			swlArgs << QString("-%1").arg(device);
			swlArgs << QString("%1:%2").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME));
			break; // FIXME: for now we just stop after the first "part" because MESS can't handle multiple device parts yet
		}
	}

#if defined(QMC2_EMUTYPE_MESS)
	// optionally add arguments for the selected device configuration
	QString devConfigName = comboBoxDeviceConfiguration->currentText();
	if ( devConfigName != tr("No additional devices") ) {
		qmc2Config->beginGroup(QString("MESS/Configuration/Devices/%1/%2").arg(systemName).arg(devConfigName));
		QStringList instances = qmc2Config->value("Instances").toStringList();
		QStringList files = qmc2Config->value("Files").toStringList();
		qmc2Config->endGroup();
		for (int i = 0; i < instances.count(); i++) {
#if defined(Q_WS_WIN)
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace('/', '\\');
#else
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace("~", "$HOME");
#endif
		}
	}
#endif

	return swlArgs;
}

SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent = %1)").arg((qulonglong)parent));
#endif

	parentTreeWidget = parent;
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

	if ( qName == "softwarelist" ) {
		softwareListName = attributes.value("name");
	} else if ( qName == "software" ) {
		softwareName = attributes.value("name");
		softwareItem = new QTreeWidgetItem(parentTreeWidget);
		softwareItem->setText(QMC2_SWLIST_COLUMN_NAME, softwareName);
		softwareItem->setText(QMC2_SWLIST_COLUMN_LIST, softwareListName);
	} else if ( qName == "part" ) {
		softwarePart = attributes.value("name");
		QString parts = softwareItem->text(QMC2_SWLIST_COLUMN_PART);
		if ( parts.isEmpty() )
			softwareItem->setText(QMC2_SWLIST_COLUMN_PART, softwarePart);
		else
			softwareItem->setText(QMC2_SWLIST_COLUMN_PART, parts + "," + softwarePart);
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
	: QWidget(parent, Qt::Window | Qt::CustomizeWindowHint | Qt::FramelessWindowHint)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::SoftwareSnap(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	setWindowTitle(tr("Snapshot viewer"));

	contextMenu = new QMenu(this);
	contextMenu->hide();

	QString s;
	QAction *action;

	s = tr("Save as...");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(saveAs()));

	s = tr("Copy to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
}

void SoftwareSnap::leaveEvent(QEvent *)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::leaveEvent(QEvent *)");
#endif

	if ( contextMenu->isHidden() )
		hide();
}

void SoftwareSnap::mousePressEvent(QMouseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::mousePressEvent(QMouseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e->button() != Qt::RightButton )
		hide();
}

void SoftwareSnap::keyPressEvent(QKeyEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::keyPressEvent(QKeyPressEvent *e)");
#endif

	if ( e->key() == Qt::Key_Escape )
		hide();
}

void SoftwareSnap::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

void SoftwareSnap::copyToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::copyToClipboard()");
#endif

	// FIXME
}

void SoftwareSnap::saveAs()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::saveAs()");
#endif

	// FIXME
}

void SoftwareSnap::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.eraseRect(rect());
	p.end();
}
