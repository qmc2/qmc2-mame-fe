#ifndef _DATINFODBMGR_H_
#define _DATINFODBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>

class DatInfoDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit DatInfoDatabaseManager(QObject *parent);
		~DatInfoDatabaseManager();

		QString qmc2Version();
		void setQmc2Version(QString qmc2_version);
		int datInfoVersion();
		void setDatInfoVersion(int datinfo_version);

		QString softwareInfo(QString list, QString id);
		void setSoftwareInfo(QString list, QString id, QString infotext);
		bool existsSoftwareInfo(QString list, QString id);
		qint64 softwareInfoRowCount();

		QString emuInfo(QString id);
		void setEmuInfo(QString id, QString infotext);
		bool existsEmuInfo(QString id);
		qint64 emuInfoRowCount();

		QString gameInfo(QString id);
		void setGameInfo(QString id, QString infotext);
		bool existsGameInfo(QString id);
		qint64 gameInfoRowCount();

		QString connectionName() { return m_connectionName; }
		QString databasePath() { return m_db.databaseName(); }
		quint64 databaseSize();
		void setCacheSize(quint64 kiloBytes);
		void setSyncMode(uint syncMode);
		void setJournalMode(uint journalMode);

	public slots:
		void recreateSoftwareInfoTable();
		void recreateEmuInfoTable();
		void recreateGameInfoTable();
		void recreateMetaDataTable();
		void recreateDatabase();
		void beginTransaction() { m_db.driver()->beginTransaction(); }
		void commitTransaction() { m_db.driver()->commitTransaction(); }

	private:
		mutable QSqlDatabase m_db;
		QString m_softwareInfoTableName;
		QString m_emuInfoTableName;
		QString m_gameInfoTableName;
		QString m_metaDataTableName;
		QString m_connectionName;
};

#endif
