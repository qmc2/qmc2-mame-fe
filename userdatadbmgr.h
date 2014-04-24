#ifndef _USERDATADBMGR_H_
#define _USERDATADBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
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
		QString id(int rowid);
		bool exists(QString id);
		void cleanUp();
		void remove(QString id);

		bool logActive() { return m_logActive; }
		void setLogActive(bool enable) { m_logActive = enable; }

		int userDataRowCount();
		int nextRowId(bool refreshRowIds = false);

	public slots:
		void recreateDatabase();
		void beginTransaction() { m_db.driver()->beginTransaction(); }
		void commitTransaction() { m_db.driver()->commitTransaction(); }
		void clearRankCache() { m_rankCache.clear(); }
		void clearCommentCache() { m_commentCache.clear(); }

	private:
		mutable QSqlDatabase m_db;
		QString m_tableBasename;
		bool m_logActive;
		QList<int> m_rowIdList;
		int m_lastRowId;
		QHash<QString, int> m_rankCache;
		QHash<QString, QString> m_commentCache;
};

#endif
