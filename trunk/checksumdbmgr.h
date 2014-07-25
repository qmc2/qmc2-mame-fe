#ifndef _CHECKSUMDBMGR_H_
#define _CHECKSUMDBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>

class CheckSumDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit CheckSumDatabaseManager(QObject *parent);
		~CheckSumDatabaseManager();

		QString connectionName() { return m_connectionName; }

	public slots:
		void recreateDatabase();
		void beginTransaction() { m_db.driver()->beginTransaction(); }
		void commitTransaction() { m_db.driver()->commitTransaction(); }

	private:
		mutable QSqlDatabase m_db;
		QString m_connectionName;
};

#endif
