#ifndef _ROMDBMGR_H_
#define _ROMDBMGR_H_

#include <QtSql>

class ROMDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		QSqlDatabase db;

		bool hasFeatureTransactions;
		bool hasFeatureLastInsertId;

		ROMDatabaseManager(QObject *parent);
		~ROMDatabaseManager();

		bool isConnected() { return db.isOpen(); }
		QString errorText() { return db.lastError().text(); }
		int errorNumber() { return db.lastError().number(); }

		void closeConnection();
		bool openConnection(int driver, QString user, QString password, QString database, QString host = QString(), int port = 0);
		bool checkConnection(int driver, QString user, QString password, QString database, QString host = QString(), int port = 0);

	public slots:
};

#endif
