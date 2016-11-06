#ifndef _ICONDBMGR_H_
#define _ICONDBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QByteArray>

#define QMC2_ICDB_INDEX_ID		0
#define QMC2_ICDB_INDEX_ICON_DATA	1

class IconCacheDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit IconCacheDatabaseManager(QObject *parent);
		~IconCacheDatabaseManager();

		QString emulatorVersion();
		void setEmulatorVersion(QString emu_version);
		QString qmc2Version();
		void setQmc2Version(QString qmc2_version);
		int iconCacheVersion();
		void setIconCacheVersion(int icon_cache_version);

		bool exists(const QString &id);
		bool logActive() { return m_logActive; }
		void setLogActive(bool enable) { m_logActive = enable; }
		bool isEmpty();
		bool importRequired(const QStringList &pathList);
		qint64 iconCacheRowCount(bool reset = false);

		void setIconData(const QString &id, const QByteArray &icon_data);
		void queryIconData();
		bool nextIconData(QString *id, QByteArray *icon_data);
		
		QString connectionName() { return m_connectionName; }
		QString databasePath() { return m_db.databaseName(); }
		quint64 databaseSize();
		void setCacheSize(quint64 kiloBytes);
		void setSyncMode(uint syncMode);
		void setJournalMode(uint journalMode);

		QSqlDatabase &db() { return m_db; }

	public slots:
		void recreateDatabase();
		void beginTransaction() { m_db.driver()->beginTransaction(); }
		void commitTransaction() { m_db.driver()->commitTransaction(); }

	private:
		mutable QSqlDatabase m_db;
		QString m_tableBasename;
		QString m_connectionName;
		bool m_logActive;
		bool m_resetRowCount;
		qint64 m_lastRowCount;
		QSqlQuery *m_query;
};

#endif
