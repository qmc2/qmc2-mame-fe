#ifndef _MACHINELISTDBMGR_H_
#define _MACHINELISTDBMGR_H_

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>

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
		bool isEmpty() { return machineListRowCount() <= 0; }

		qint64 machineListRowCount();
		qint64 nextRowId(bool refreshRowIds = false);

		void setData(const QString &id, const QString &description, const QString &manufacturer, const QString &year, const QString &cloneof, bool is_bios, bool is_device, bool has_roms, bool has_chds, int players, const QString &drvstat, const QString &srcfile);

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
		QList<qint64> m_rowIdList;
		qint64 m_lastRowId;
		bool m_logActive;
};

#endif
