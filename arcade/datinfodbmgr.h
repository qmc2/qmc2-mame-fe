#ifndef _DATINFODBMGR_H_
#define _DATINFODBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QStringList>

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
		bool softwareInfoImportRequired(QStringList pathList);

		QString emuInfo(QString id);
		void setEmuInfo(QString id, QString infotext);
		bool existsEmuInfo(QString id);
		qint64 emuInfoRowCount();
		bool emuInfoImportRequired(QStringList pathList);

		QString gameInfo(QString id);
		QString machineInfo(QString id) { return gameInfo(id); }
		QString gameInfoEmulator(QString id);
		QString machineInfoEmulator(QString id) { return gameInfoEmulator(id); }
		void setGameInfo(QString id, QString infotext, QString emulator);
		void setMachineInfo(QString id, QString infotext, QString emulator) { setGameInfo(id, infotext, emulator); }
		bool existsGameInfo(QString id);
		bool existsMachineInfo(QString id) { return existsGameInfo(id); }
		qint64 gameInfoRowCount();
		qint64 machineInfoRowCount() { return gameInfoRowCount(); }
		bool gameInfoImportRequired(QStringList pathList);
		bool machineInfoImportRequired(QStringList pathList) { return gameInfoImportRequired(pathList); }

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
		void importSoftwareInfo(QStringList pathList, bool fromScratch = true);
		void importEmuInfo(QStringList pathList, bool fromScratch = true);
		void importGameInfo(QStringList pathList, QStringList emulatorList, bool fromScratch = true);
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
