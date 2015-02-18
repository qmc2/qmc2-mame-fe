#ifndef _SWLDBMGR_H_
#define _SWLDBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>

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
		int swlCacheVersion();
		void setSwlCacheVersion(int swlcache_version);
		QString dtd();
		void setDtd(QString dtd);
		QStringList uniqueSoftwareLists();
		QStringList uniqueSoftwareSets(QString list);
		QString xml(QString list, QString id);
		QString xml(int rowid);
		QString list(int rowid);
		QString nextXml(QString list, QString *id, bool start = false);
		QString allXml(QString list);
		void setXml(QString list, QString id, QString xml);
		bool exists(QString list, QString id);
		qint64 swlRowCount();
		qint64 nextRowId(bool refreshRowIds = false);
		QString idOfRow(qint64 row);
		QString connectionName() { return m_connectionName; }
		QString databasePath() { return m_db.databaseName(); }
		quint64 databaseSize();
		void setCacheSize(quint64 kiloBytes);
		void setSyncMode(uint syncMode);
		void setJournalMode(uint journalMode);

	public slots:
		void recreateDatabase(bool quiet = false);
		void beginTransaction() { m_db.driver()->beginTransaction(); }
		void commitTransaction() { m_db.driver()->commitTransaction(); }

	private:
		mutable QSqlDatabase m_db;
		QString m_tableBasename;
		QString m_connectionName;
		QSqlQuery *m_listIterationQuery;
		QList<qint64> m_rowIdList;
		qint64 m_lastRowId;
};

#endif
