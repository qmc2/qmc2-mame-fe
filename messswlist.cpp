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

QMap<QString, QString> messMachineSoftwareListMap;
QMap<QString, QString> messSoftwareListXmlDataCache;
QString messSwlBuffer;
QString messSwlLastLine;
bool messSwlSupported = true;

#define QMC2_DEBUG

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
}

MESSSoftwareList::~MESSSoftwareList()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::~MESSSoftwareList()");
#endif

}

QString &MESSSoftwareList::getXmlData(QString machineName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::getXmlData(QString machineName = %1)").arg(machineName));
#endif

	static QString xmlBuffer;

	QString machineSoftwareList = messMachineSoftwareListMap[machineName];
	if ( machineSoftwareList.isEmpty() ) {
		int i = 0;
		QString s = "<machine name=\"" + machineName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				machineSoftwareList = line.mid(startIndex, endIndex - startIndex); 
			}
		}
		if ( machineSoftwareList.isEmpty() )
			machineSoftwareList = "NO_SOFTWARE_LIST";
		messMachineSoftwareListMap[machineName] = machineSoftwareList;
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: messMachineSoftwareListMap[%1] = %2").arg(machineName).arg(machineSoftwareList));
#endif
	}
#ifdef QMC2_DEBUG
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: messMachineSoftwareListMap[%1] = %2 (cached)").arg(machineName).arg(messMachineSoftwareListMap[machineName]));
#endif

	xmlBuffer.clear();

	if ( !machineSoftwareList.isEmpty() && machineSoftwareList != "NO_SOFTWARE_LIST" ) {
		xmlBuffer = messSoftwareListXmlDataCache[machineSoftwareList];
		if ( xmlBuffer.isEmpty() ) {
			// FIXME: retrieve the software list XML data for the current machine here...

			messSoftwareListXmlDataCache[machineSoftwareList] = xmlBuffer;
		}
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
	if ( messSwlBuffer.isEmpty() && messSwlSupported ) {
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
					qmc2MainWindow->progressBarGamelist->setRange(0, fi.size() / QMC2_FILE_BUFFER_SIZE);
					qmc2MainWindow->progressBarGamelist->reset();
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

		if ( !validData )
			return false;
	}

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::load(): validData = %1").arg(validData ? "true" : "false"));
#endif

	QString xmlData = getXmlData(messMachineName);

	// FIXME: parse the XML data and build the software list entries here...

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
			if ( !line.startsWith("<!") && !line.startsWith("<?xml") && !line.startsWith("]>") )
				tsSWLCache << line << "\n";
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
