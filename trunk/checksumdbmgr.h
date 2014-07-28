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

		QString qmc2Version();
		void setQmc2Version(QString qmc2_version);
		int checkSumDbVersion();
		void setCheckSumDbVersion(int checksum_db_version);
		uint scanTime();
		void setScanTime(uint scan_time);

		bool logActive() { return m_logActive; }
		void setLogActive(bool enable) { m_logActive = enable; }

		int checkSumRowCount();

		QString connectionName() { return m_connectionName; }

	public slots:
		void recreateDatabase();
		void beginTransaction() { m_db.driver()->beginTransaction(); }
		void commitTransaction() { m_db.driver()->commitTransaction(); }

	private:
		mutable QSqlDatabase m_db;
		QString m_tableBasename;
		QString m_connectionName;
		bool m_logActive;
};

#endif
