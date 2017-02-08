#ifndef _USERDATADBMGR_H_
#define _USERDATADBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>

class UserDataDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit UserDataDatabaseManager(QObject *parent);
		~UserDataDatabaseManager();

		QString emulatorVersion();
		void setEmulatorVersion(QString emu_version);
		QString qmc2Version();
		void setQmc2Version(QString qmc2_version);
		int userDataVersion();
		void setUserDataVersion(int userdata_version);
		int rank(QString id);
		int rank(int rowid);
		void setRank(QString id, int rank);
		QString comment(QString id);
		QString comment(int rowid);
		void setComment(QString id, QString comment);
		QString &id(int rowid);
		bool exists(QString id);
		void cleanUp();
		void remove(QString id);

		void setHiddenLists(QString id, QStringList lists);
		QStringList hiddenLists(QString id);
		void removeHiddenLists(QString id);

		void setListFavorites(QString id, QStringList favorites);
		QStringList listFavorites(QString id);
		void removeListFavorites(QString id);

		void setDeviceConfigs(QString id, QStringList device_configs);
		QStringList deviceConfigs(QString id);
		void removeDeviceConfigs(QString id);

		void setSelectedSoftware(QString id, QString softwareList, QString softwareName);
		bool getSelectedSoftware(QString id, QString *softwareList, QString *softwareName);
		void removeSelectedSoftware(QString id);

		bool logActive() { return m_logActive; }
		void setLogActive(bool enable) { m_logActive = enable; }

		qint64 userDataRowCount();
		qint64 nextRowId(bool refreshRowIds = false);

		bool rankCacheComplete() { return userDataRowCount() == m_rankCache.count(); }
		void fillUpRankCache();
		QHash<QString, int> &rankCache() { return m_rankCache; }
		bool commentCacheComplete() { return userDataRowCount() == m_commentCache.count(); }
		void fillUpCommentCache();
		QHash<QString, QString> &commentCache() { return m_commentCache; }

		QString connectionName() { return m_connectionName; }
		QString databasePath() { return m_db.databaseName(); }
		quint64 databaseSize();
		void setCacheSize(quint64 kiloBytes);
		void setSyncMode(uint syncMode);
		void setJournalMode(uint journalMode);

		QStringList columnNames(QString tableName);

	public slots:
		void recreateDatabase();
		void beginTransaction() { m_db.driver()->beginTransaction(); }
		void commitTransaction() { m_db.driver()->commitTransaction(); }
		void clearRankCache() { m_rankCache.clear(); }
		void clearCommentCache() { m_commentCache.clear(); }
		void recreateSoftListTable();
		void recreateSystemManualTable();
		void recreateSoftwareManualTable();

	private:
		void addSoftListFavoritesColumn();
		void addSoftListDeviceConfigsColumn();
		void addSoftListSelectedSoftwareColumn();
		void renameSoftListTable();

		mutable QSqlDatabase m_db;
		QString m_tableBasename;
		QString m_tableBasenameSL;
		QString m_oldTableBasenameSL;
		QString m_tableBasenameSystemManuals;
		QString m_tableBasenameSoftwareManuals;
		QString m_connectionName;
		QString m_idString;
		bool m_logActive;
		QList<qint64> m_rowIdList;
		qint64 m_lastRowId;
		QHash<QString, int> m_rankCache;
		QHash<QString, QString> m_commentCache;
};

#endif
