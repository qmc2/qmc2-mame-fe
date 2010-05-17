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

#define QMC2_DEBUG

MESSSoftwareList::MESSSoftwareList(QString machineName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::MESSSoftwareList(QString machineName = %1, QWidget *parent = %2)").arg(machineName).arg((qulonglong)parent));
#endif

	setupUi(this);

	messMachineName = machineName;
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

	if ( messSwlBuffer.isEmpty() ) {
		// FIXME: load the overall software list cache here...
	}

	QString xmlData = getXmlData(messMachineName);

	// FIXME: parse the XML data and build the software list entries here...

	return TRUE;
}

bool MESSSoftwareList::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::save()");
#endif

	// FIXME: save favorites here...
	return TRUE;
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

}

void MESSSoftwareList::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

}

void MESSSoftwareList::loadReadyReadStandardOutput()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadReadyReadStandardOutput()"));
#endif

}

void MESSSoftwareList::loadReadyReadStandardError()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadReadyReadStandardError()"));
#endif

}

void MESSSoftwareList::loadError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

}

void MESSSoftwareList::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::loadStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif

}
