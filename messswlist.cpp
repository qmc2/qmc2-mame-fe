#include <QFileInfo>

#include "messswlist.h"
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
extern QMap<QString, QString> messXmlDataCache;

QMap<QString, QStringList> messMachineSoftwareListMap;
QMap<QString, QString> messSoftwareListXmlDataCache;
QString messSwlBuffer;
QString messSwlLastLine;
bool messSwlSupported = true;

MESSSoftwareList::MESSSoftwareList(QString machineName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::MESSSoftwareList(QString machineName = %1, QWidget *parent = %2)").arg(machineName).arg((qulonglong)parent));
#endif

	setupUi(this);

	messMachineName = machineName;
	loadProc = NULL;
	validData = false;

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
	treeWidgetKnownSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSSoftwareList/KnownSoftwareHeaderState").toByteArray());
	treeWidgetFavoriteSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSSoftwareList/FavoriteSoftwareHeaderState").toByteArray());
	treeWidgetSearchResults->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSSoftwareList/SearchResultsHeaderState").toByteArray());
	toolBoxSoftwareList->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSSoftwareList/PageIndex").toInt());

	connect(treeWidgetKnownSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetKnownSoftware_headerSectionClicked(int)));
	connect(treeWidgetFavoriteSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetFavoriteSoftware_headerSectionClicked(int)));
	connect(treeWidgetSearchResults->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetSearchResults_headerSectionClicked(int)));
}

MESSSoftwareList::~MESSSoftwareList()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::~MESSSoftwareList()");
#endif

	// save widget states
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSSoftwareList/KnownSoftwareHeaderState", treeWidgetKnownSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSSoftwareList/FavoriteSoftwareHeaderState", treeWidgetFavoriteSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSSoftwareList/SearchResultsHeaderState", treeWidgetSearchResults->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSSoftwareList/PageIndex", toolBoxSoftwareList->currentIndex());
}

QString &MESSSoftwareList::getSoftwareListXmlData(QString listName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::getSoftwareListXmlData(QString listName = %1)").arg(listName));
#endif

	static QString softwareListXmlBuffer;

	softwareListXmlBuffer = messSoftwareListXmlDataCache[listName];

	if ( softwareListXmlBuffer.isEmpty() ) {
		int i = 0;
		QString s = "<softwarelist name=\"" + listName + "\"";
		while ( !messSwlLines[i].contains(s) ) i++;
		softwareListXmlBuffer = "<?xml version=\"1.0\"?>\n";
		while ( !messSwlLines[i].contains("</softwarelist>") )
			softwareListXmlBuffer += messSwlLines[i++].simplified() + "\n";
		softwareListXmlBuffer += "</softwarelist>";
		messSoftwareListXmlDataCache[listName] = softwareListXmlBuffer;
	}

	return softwareListXmlBuffer;
}

QString &MESSSoftwareList::getXmlData(QString machineName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::getXmlData(QString machineName = %1)").arg(machineName));
#endif

	static QString xmlBuffer;

	QStringList machineSoftwareList = messMachineSoftwareListMap[machineName];
	if ( machineSoftwareList.isEmpty() ) {
		int i = 0;
		QString s = "<machine name=\"" + machineName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				machineSoftwareList << line.mid(startIndex, endIndex - startIndex); 
			}
		}
		if ( machineSoftwareList.isEmpty() )
			machineSoftwareList << "NO_SOFTWARE_LIST";
		messMachineSoftwareListMap[machineName] = machineSoftwareList;
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: messMachineSoftwareListMap[%1] = %2").arg(machineName).arg(machineSoftwareList.join(", ")));
#endif
	}
#ifdef QMC2_DEBUG
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: messMachineSoftwareListMap[%1] = %2 (cached)").arg(machineName).arg(messMachineSoftwareListMap[machineName].join(", ")));
#endif

	xmlBuffer.clear();

	if ( !machineSoftwareList.isEmpty() && !machineSoftwareList.contains("NO_SOFTWARE_LIST") ) {
		QString swlString = machineSoftwareList.join(", ");
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (%1)").arg(swlString));
		toolBoxSoftwareList->setEnabled(true);

		// load available device configurations, if any...
		qmc2Config->beginGroup(QString("MESS/Configuration/Devices/%1").arg(messMachineName));
		QStringList configurationList = qmc2Config->childGroups();
		qmc2Config->endGroup();
		if ( !configurationList.isEmpty() ) {
			comboBoxDeviceConfiguration->insertItems(1, configurationList);
			comboBoxDeviceConfiguration->setEnabled(true);
		}
	} else {
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (no data available)"));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (no data available)"));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (no data available)"));
	}

	return xmlBuffer;
}

bool MESSSoftwareList::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::load()");
#endif

	bool swlCacheOkay = true;
	validData = messSwlSupported;
	QString swlCachePath = qmc2Config->value("MESS/FilesAndDirectories/SoftwareListCache").toString();

	treeWidgetKnownSoftware->clear();
	treeWidgetFavoriteSoftware->clear();
	treeWidgetSearchResults->clear();

	treeWidgetKnownSoftware->setSortingEnabled(false);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(false);
	treeWidgetFavoriteSoftware->setSortingEnabled(false);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(false);
	treeWidgetSearchResults->setSortingEnabled(false);
	treeWidgetSearchResults->header()->setSortIndicatorShown(false);

	if ( messSwlBuffer.isEmpty() && messSwlSupported ) {
		messSwlLines.clear();
		validData = false;
		swlCacheOkay = false;
		if ( !swlCachePath.isEmpty() ) {
			fileSWLCache.setFileName(swlCachePath);
			if ( fileSWLCache.open(QIODevice::ReadOnly | QIODevice::Text) ) {
				QTextStream ts(&fileSWLCache);
				QString line = ts.readLine();
				line = ts.readLine();
				if ( line.startsWith("MESS_VERSION") ) {
					QStringList words = line.split("\t");
					if ( words.count() > 1 ) {
						if ( qmc2Gamelist->emulatorVersion == words[1] )
							swlCacheOkay = true;
					}
				}
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
								messSwlBuffer += line + "\n";
							}
						}
						if ( endsWithNewLine )
							readBuffer.clear();
						else
							readBuffer = lines.last();
						qmc2MainWindow->progressBarGamelist->setValue(messSwlBuffer.length());
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
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: the file name for the MESS software list cache is empty -- please correct this and reload the machine list afterwards"));
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
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open the MESS software list cache for writing, path = %1 -- please check/correct access permissions and reload the machine list afterwards").arg(swlCachePath));
			return false;
		}

		messSwlBuffer.clear();
		messSwlLastLine.clear();

		tsSWLCache.setDevice(&fileSWLCache);
		tsSWLCache.reset();
		tsSWLCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
		tsSWLCache << "MESS_VERSION\t" + qmc2Gamelist->emulatorVersion + "\n";
		
		loadProc = new QProcess(this);

		connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(readyReadStandardError()), this, SLOT(loadReadyReadStandardError()));
		connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
		connect(loadProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(loadStateChanged(QProcess::ProcessState)));

		QString command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
		QStringList args;
		args << "-listsoftware";
		QString hashPath = qmc2Config->value("MESS/Configuration/Global/hashpath").toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;

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
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::load(): validData = %1").arg(validData ? "true" : "false"));
#endif

	QString xmlData = getXmlData(messMachineName);

	QStringList machineSoftwareList = messMachineSoftwareListMap[messMachineName];
	if ( !machineSoftwareList.contains("NO_SOFTWARE_LIST") ) {
		messSwlLines = messSwlBuffer.split("\n");
		foreach (QString swList, machineSoftwareList) {
			QString softwareListXml = getSoftwareListXmlData(swList);
#ifdef QMC2_DEBUG
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::load(): XML data for software list '%1' follows:\n%2").arg(swList).arg(softwareListXml));
#endif
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			MESSSoftwareListXmlHandler xmlHandler(treeWidgetKnownSoftware);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			if ( !xmlReader.parse(xmlInputSource) )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list '%1'").arg(swList));
#ifdef QMC2_DEBUG
			else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::load(): successfully parsed the XML data for software list '%1'").arg(swList));
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

bool MESSSoftwareList::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::save()");
#endif

	// FIXME: save favorites here...
	return true;
}

void MESSSoftwareList::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e )
		e->accept();
}

void MESSSoftwareList::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e )
		e->accept();
}

void MESSSoftwareList::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e )
		e->accept();
}

void MESSSoftwareList::loadStarted()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadStarted()"));
#endif

	// we don't know how many items there are...
	qmc2MainWindow->progressBarGamelist->setRange(0, 0);
	qmc2MainWindow->progressBarGamelist->reset();
}

void MESSSoftwareList::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

	QTime elapsedTime;
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	if ( exitStatus == QProcess::NormalExit && exitCode == 0 ) {
		validData = true;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
		validData = false;
	}
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
	qmc2MainWindow->progressBarGamelist->reset();
}

void MESSSoftwareList::loadReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadReadyReadStandardOutput()"));
#endif

	QString s = messSwlLastLine + proc->readAllStandardOutput();
#if defined(Q_WS_WIN)
	s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
	QStringList lines = s.split("\n");

	if ( s.endsWith("\n") ) {
		messSwlLastLine.clear();
	} else {
		messSwlLastLine = lines.last();
		lines.removeLast();
	}

	foreach (QString line, lines) {
		line = line.trimmed();
		if ( !line.isEmpty() )
			if ( !line.startsWith("<!") && !line.startsWith("<?xml") && !line.startsWith("]>") ) {
				tsSWLCache << line << "\n";
				messSwlBuffer += line + "\n";
			}
	}
}

void MESSSoftwareList::loadReadyReadStandardError()
{
	QProcess *proc = (QProcess *)sender();

	QString data = proc->readAllStandardError();
	data = data.trimmed();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadReadyReadStandardError(): data = '%1'").arg(data));
#endif

	if ( data.contains("unknown option: -listsoftware") ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: your currently selected MESS emulator doesn't support software lists -- MESS 0.138+ required"));
		messSwlSupported = false;
		if ( fileSWLCache.isOpen() )
			fileSWLCache.close();
		fileSWLCache.remove();
	}
}

void MESSSoftwareList::loadError(QProcess::ProcessError processError)
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists caused an error -- processError = %1").arg(processError));
	validData = false;

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
	qmc2MainWindow->progressBarGamelist->reset();
}

void MESSSoftwareList::loadStateChanged(QProcess::ProcessState processState)
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif

}

void MESSSoftwareList::on_toolButtonReload_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::on_toolButtonReload_clicked(bool checked = %1)").arg(checked));
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

void MESSSoftwareList::on_toolButtonAddToFavorites_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::on_toolButtonAddToFavorites_clicked(bool checked = %1)").arg(checked));
#endif

}

void MESSSoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool checked = %1)").arg(checked));
#endif

}

void MESSSoftwareList::on_toolButtonPlay_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::on_toolButtonPlay_clicked(bool checked = %1)").arg(checked));
#endif

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
}

void MESSSoftwareList::on_toolButtonPlayEmbedded_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::on_toolButtonPlayEmbedded_clicked(bool checked = %1)").arg(checked));
#endif

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));
}

void MESSSoftwareList::treeWidgetKnownSoftware_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::on_treeWidgetKnownSoftware_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetKnownSoftware->scrollToItem(selectedItems[0]);
}

void MESSSoftwareList::treeWidgetFavoriteSoftware_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::on_treeWidgetFavoriteSoftware_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetFavoriteSoftware->scrollToItem(selectedItems[0]);
}

void MESSSoftwareList::treeWidgetSearchResults_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::on_treeWidgetSearchResults_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetSearchResults->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetSearchResults->scrollToItem(selectedItems[0]);
}

void MESSSoftwareList::on_treeWidgetKnownSoftware_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::on_treeWidgetKnownSoftware_itemSelectionChanged()");
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	bool enable = (selectedItems.count() > 0);
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
}

void MESSSoftwareList::on_treeWidgetFavoriteSoftware_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::on_treeWidgetFavoriteSoftware_itemSelectionChanged()");
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	bool enable = (selectedItems.count() > 0);
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonRemoveFromFavorites->setEnabled(enable);
}

void MESSSoftwareList::on_treeWidgetSearchResults_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::on_treeWidgetSearchResults_itemSelectionChanged()");
#endif

}

void MESSSoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p = ...)");
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

void MESSSoftwareList::on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &p = ...)");
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

void MESSSoftwareList::on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &p = ...)");
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

QStringList &MESSSoftwareList::arguments()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::arguments()");
#endif

	static QStringList messSwlArgs;

	messSwlArgs.clear();

	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	if ( selectedItems.count() > 0 ) {
		QTreeWidgetItem *item = selectedItems[0];
		messSwlArgs << QString("-%1").arg(item->text(QMC2_SWLIST_COLUMN_DEVICE));
		messSwlArgs << QString("%1:%2").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME));
	}

	return messSwlArgs;
}

MESSSoftwareListXmlHandler::MESSSoftwareListXmlHandler(QTreeWidget *parent)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareListXmlHandler::MESSSoftwareListXmlHandler(QTreeWidget *parent = %1)").arg((qulonglong)parent));
#endif

	parentTreeWidget = parent;
}

MESSSoftwareListXmlHandler::~MESSSoftwareListXmlHandler()
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareListXmlHandler::~MESSSoftwareListXmlHandler()");
#endif

}

bool MESSSoftwareListXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareListXmlHandler::startElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2, const QXmlAttributes &attributes = ...)").arg(localName).arg(qName));
#endif

	if ( qName == "softwarelist" ) {
		softwareListName = attributes.value("name");
	} else if ( qName == "software" ) {
		softwareName = attributes.value("name");
		softwareItem = new QTreeWidgetItem(parentTreeWidget);
		softwareItem->setText(QMC2_SWLIST_COLUMN_NAME, softwareName);
		softwareItem->setText(QMC2_SWLIST_COLUMN_LIST, softwareListName);
	} else if ( qName == "part" ) {
		softwareDevice = attributes.value("name");
		softwareItem->setText(QMC2_SWLIST_COLUMN_DEVICE, softwareDevice);
	} else if ( qName == "description" || qName == "year" || qName == "publisher" ) {
		currentText.clear();
	}

	return true;
}

bool MESSSoftwareListXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareListXmlHandler::endElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2)").arg(localName).arg(qName));
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

bool MESSSoftwareListXmlHandler::characters(const QString &str)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareListXmlHandler::characters(const QString &str = ...)"));
#endif

	currentText += QString::fromUtf8(str.toAscii());
	return true;
}
