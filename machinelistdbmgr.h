#ifndef _MACHINELISTDBMGR_H_
#define _MACHINELISTDBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QMutex>

#define QMC2_MLDB_INDEX_ID			0
#define QMC2_MLDB_INDEX_DESCRIPTION		1
#define QMC2_MLDB_INDEX_MANUFACTURER		2
#define QMC2_MLDB_INDEX_YEAR			3
#define QMC2_MLDB_INDEX_CLONEOF			4
#define QMC2_MLDB_INDEX_IS_BIOS			5
#define QMC2_MLDB_INDEX_IS_DEVICE		6
#define QMC2_MLDB_INDEX_HAS_ROMS		7
#define QMC2_MLDB_INDEX_HAS_CHDS		8
#define QMC2_MLDB_INDEX_PLAYERS			9
#define QMC2_MLDB_INDEX_DRVSTAT			10
#define QMC2_MLDB_INDEX_SRCFILE			11

class MachineListDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit MachineListDatabaseManager(QObject *parent);
		~MachineListDatabaseManager();

		QString emulatorVersion();
		void setEmulatorVersion(QString emu_version);
		QString qmc2Version();
		void setQmc2Version(QString qmc2_version);
		int machineListVersion();
		void setMachineListVersion(int machinelist_version);

		QString id(int rowid);
		bool exists(QString id);
		bool logActive() { return m_logActive; }
		void setLogActive(bool enable) { m_logActive = enable; }
		bool isEmpty();

		qint64 machineListRowCount(bool reset = false);
		qint64 nextRowId(bool refreshRowIds = false);

		void setData(const QString &id, const QString &description, const QString &manufacturer, const QString &year, const QString &cloneof, bool is_bios, bool is_device, bool has_roms, bool has_chds, int players, const QString &drvstat, const QString &srcfile);

		void queryRecords(QSqlQuery *query);
		bool nextRecord(QSqlQuery *query, QString *id, QString *description, QString *manufacturer, QString *year, QString *cloneof, bool *is_bios, bool *is_device, bool *has_roms, bool *has_chds, int *players, QString *drvstat, QString *srcfile);
		
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
		QList<qint64> m_rowIdList;
		qint64 m_lastRowId;
		bool m_logActive;
		bool m_resetRowCount;
		qint64 m_lastRowCount;
		QMutex m_queryMutex;
};

#endif
