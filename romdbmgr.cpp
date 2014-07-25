#include "macros.h"
#include "qmc2main.h"
#include "romdbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

#ifndef QMC2_DEBUG
#define QMC2_DEBUG
#endif

ROMDatabaseManager::ROMDatabaseManager(QObject *parent)
	: QObject(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMDatabaseManager::ROMDatabaseManager(QObject *parent = %1)").arg((qulonglong)parent));
#endif

	hasFeatureTransactions = false;
	hasFeatureLastInsertId = false;
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMDatabaseManager::openConnection(int driver = %1, QString user = %2, QString password = ..., QString database = %3, QString host = %4, int port = %5)")
			.arg(driver).arg(user).arg(database).arg(host).arg(port));
#endif

	switch ( driver ) {
		/* FIXME: we only support MySQL for now
		case QMC2_DB_DRIVER_SQLLITE:
			if ( !QSqlDatabase::contains("QMC2_ROMDB_SQLITE") )
				db = QSqlDatabase::addDatabase("QSQLITE", "QMC2_ROMDB_SQLITE");
			break;
		*/
		case QMC2_DB_DRIVER_MYSQL:
		default:
			if ( !QSqlDatabase::contains("QMC2_ROMDB_MYSQL") )
				db = QSqlDatabase::addDatabase("QMYSQL", "QMC2_ROMDB_MYSQL");
			if ( port <= 0 )
				port = QMC2_DB_DFLT_PORT_MYSQL;
			break;
	}

	db.setHostName(host);
	db.setPort(port);
	db.setUserName(user);
	db.setPassword(password);
	db.setDatabaseName(database);

	if ( db.open() ) {
		hasFeatureTransactions = db.driver()->hasFeature(QSqlDriver::Transactions);
		hasFeatureLastInsertId = db.driver()->hasFeature(QSqlDriver::LastInsertId);
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: database successfully opened: hasFeatureTransactions = %1, hasFeatureLastInsertId = %2").arg(hasFeatureTransactions).arg(hasFeatureLastInsertId));
#endif
		return true;
	} else {
		return false;
	}
}

void ROMDatabaseManager::closeConnection()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMDatabaseManager::closeConnection()");
#endif

	if ( db.isOpen() )
		db.close();

	hasFeatureTransactions = false;
	hasFeatureLastInsertId = false;
}

bool ROMDatabaseManager::checkConnection(int driver, QString user, QString password, QString database, QString host, int port)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMDatabaseManager::checkConnection(int driver = %1, QString user = %2, QString password = ..., QString database = %3, QString host = %4, int port = %5)")
			.arg(driver).arg(user).arg(database).arg(host).arg(port));
#endif

	bool checkResult = openConnection(driver, user, password, database, host, port);
	closeConnection();
	return checkResult;
}
