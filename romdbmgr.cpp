#if defined(QMC2_DATABASE_ENABLED)

#include "macros.h"
#include "qmc2main.h"
#include "romdbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

#define QMC2_DEBUG

ROMDatabaseManager::ROMDatabaseManager(QObject *parent)
	: QObject(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMDatabaseManager::ROMDatabaseManager(QObject *parent = %1)").arg((qulonglong)parent));
#endif

}

ROMDatabaseManager::~ROMDatabaseManager()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMDatabaseManager::~ROMDatabaseManager()");
#endif

	closeConnection();
}

bool ROMDatabaseManager::openConnection(int driver, QString user, QString password, QString database, QString host, int port)
{
	switch ( driver ) {
		/* FIXME: we only support MySQL for now
		case QMC2_DB_DRIVER_SQLLITE:
			break;
		*/
		case QMC2_DB_DRIVER_MYSQL:
		default:
			if ( !QSqlDatabase::contains("QMC2_ROMDB") )
				db = QSqlDatabase::addDatabase("QMYSQL", "QMC2_ROMDB");
			if ( port <= 0 )
				port = QMC2_DB_DFLT_PORT_MYSQL;
			break;
	}

	db.setHostName(host);
	db.setPort(port);
	db.setUserName(user);
	db.setPassword(password);
	db.setDatabaseName(database);

	return db.open();
}

void ROMDatabaseManager::closeConnection()
{
	if ( db.isOpen() )
		db.close();
}

bool ROMDatabaseManager::checkConnection(int driver, QString user, QString password, QString database, QString host, int port)
{
	bool checkResult = openConnection(driver, user, password, database, host, port);
	closeConnection();
	return checkResult;
}

#endif
