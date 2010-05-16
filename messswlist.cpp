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

QMap<QString, QString> messSoftwareListXmlDataCache;

MESSSoftwareList::MESSSoftwareList(QString machineName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSSoftwareList::MESSSoftwareList(QString machineName = %1, QWidget *parent = %2)").arg(machineName).arg((qulonglong)parent));
#endif

	setupUi(this);
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

	// FIXME: this needs to search in the -lx output first, then lookup the software list...
	static QString xmlBuffer;

	xmlBuffer = messSoftwareListXmlDataCache[machineName];

	if ( xmlBuffer.isEmpty() ) {
		int i = 0;
		QString s = "<machine name=\"" + machineName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
		xmlBuffer = "<?xml version=\"1.0\"?>\n";
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") )
			xmlBuffer += qmc2Gamelist->xmlLines[i++].simplified() + "\n";
		xmlBuffer += "</machine>\n";
		messSoftwareListXmlDataCache[machineName] = xmlBuffer;
	}

	return xmlBuffer;
}

bool MESSSoftwareList::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSSoftwareList::load()");
#endif

	// FIXME: load software list here...
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
