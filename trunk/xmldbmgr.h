#ifndef _XMLDBMGR_H_
#define _XMLDBMGR_H_

#include <QObject>
#include <QList>
#include <QSqlDatabase>
#include <QSqlDriver>

class XmlDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit XmlDatabaseManager(QObject *parent);
		~XmlDatabaseManager();

		QString emulatorVersion();
		void setEmulatorVersion(QString emu_version);
		QString qmc2Version();
		void setQmc2Version(QString qmc2_version);
		int xmlCacheVersion();
		void setXmlCacheVersion(int xmlcache_version);
		QString dtd();
		void setDtd(QString dtd);
		QString xml(QString id);
		QString xml(int rowid);
		void setXml(QString id, QString xml);
		bool exists(QString id);
		QString parentOf(QString id);
		bool logActive() { return m_logActive; }
		void setLogActive(bool enable) { m_logActive = enable; }
		qint64 xmlRowCount();
		qint64 nextRowId(bool refreshRowIds = false);
		QString idOfRow(qint64 row);
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
		bool m_logActive;
		QList<qint64> m_rowIdList;
		qint64 m_lastRowId;
};

#endif
