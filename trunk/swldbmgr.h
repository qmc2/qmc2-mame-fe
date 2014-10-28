#ifndef _SWLDBMGR_H_
#define _SWLDBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>

class SoftwareListXmlDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit SoftwareListXmlDatabaseManager(QObject *parent);
		~SoftwareListXmlDatabaseManager();

		QString emulatorVersion();
		void setEmulatorVersion(QString emu_version);
		QString qmc2Version();
		void setQmc2Version(QString qmc2_version);
		int xmlCacheVersion();
		void setXmlCacheVersion(int xmlcache_version);
		QString dtd();
		void setDtd(QString dtd);
		QString xml(QString list, QString id);
		QString xml(int rowid);
		void setXml(QString list, QString id, QString xml);
		bool exists(QString list, QString id);
		qint64 xmlRowCount();
		QString connectionName() { return m_connectionName; }
		QString databasePath() { return m_db.databaseName(); }
		quint64 databaseSize();
		void setCacheSize(quint64 kiloBytes);
		void setSyncMode(uint syncMode);
		void setJournalMode(uint journalMode);

	public slots:
		void recreateDatabase();
		void beginTransaction() { m_db.driver()->beginTransaction(); }
		void commitTransaction() { m_db.driver()->commitTransaction(); }

	private:
		mutable QSqlDatabase m_db;
		QString m_tableBasename;
		QString m_connectionName;
};

#endif
