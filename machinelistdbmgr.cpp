#include <QApplication>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QTreeWidgetItem>
#include <QDir>
#include <QUuid>
#include <QHash>

#include "macros.h"
#include "qmc2main.h"
#include "settings.h"
#include "options.h"
#include "machinelist.h"
#include "machinelistdbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern MachineList *qmc2MachineList;

MachineListDatabaseManager::MachineListDatabaseManager(QObject *parent) :
	QObject(parent)
{
	m_connectionName = QString("machine-list-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	m_db.setDatabaseName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MachineListDatabase", QString(Options::configPath() + "/%1-machine-list.db").arg(QMC2_EMU_NAME.toLower())).toString());
	m_tableBasename = QString("%1_machine_list").arg(QMC2_EMU_NAME.toLower());
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() < 2 || !tables.contains(m_tableBasename) || !tables.contains(QString("%1_metadata").arg(m_tableBasename)) )
			recreateDatabase();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open machine list database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
	m_lastRowId = -1;
}

MachineListDatabaseManager::~MachineListDatabaseManager()
{
	if ( m_db.isOpen() )
		m_db.close();
}

QString MachineListDatabaseManager::emulatorVersion()
{
	QString emu_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			emu_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from machine list database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return emu_version;
}

void MachineListDatabaseManager::setEmulatorVersion(QString emu_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (emu_version, row) VALUES (:emu_version, 0)").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to machine list database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET emu_version=:emu_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in machine list database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from machine list database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

QString MachineListDatabaseManager::qmc2Version()
{
	QString qmc2_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			qmc2_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from machine list database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return qmc2_version;
}

void MachineListDatabaseManager::setQmc2Version(QString qmc2_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (qmc2_version, row) VALUES (:qmc2_version, 0)").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to machine list database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET qmc2_version=:qmc2_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in machine list database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from machine list database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

int MachineListDatabaseManager::machineListVersion()
{
	int machinelist_version = -1;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT machinelist_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			machinelist_version = query.value(0).toInt();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from machine list database: query = '%2', error = '%3'").arg("machinelist_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return machinelist_version;
}

void MachineListDatabaseManager::setMachineListVersion(int machinelist_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT machinelist_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (machinelist_version, row) VALUES (:machinelist_version, 0)").arg(m_tableBasename));
			query.bindValue(":machinelist_version", machinelist_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to machine list database: query = '%2', error = '%3'").arg("machinelist_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET machinelist_version=:machinelist_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":machinelist_version", machinelist_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in machine list database: query = '%2', error = '%3'").arg("machinelist_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from machine list database: query = '%2', error = '%3'").arg("machinelist_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

qint64 MachineListDatabaseManager::machineListRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_tableBasename)) ) {
		if ( query.first() )
			return query.value(0).toLongLong();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from machine list database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
	}	return -1;
}

qint64 MachineListDatabaseManager::nextRowId(bool refreshRowIds)
{
	if ( refreshRowIds ) {
		m_rowIdList.clear();
		m_lastRowId = -1;
		QSqlQuery query(m_db);
		if ( query.exec(QString("SELECT rowid FROM %1").arg(m_tableBasename)) ) {
			if ( query.first() ) {
				do {
					m_rowIdList << query.value(0).toLongLong();
				} while ( query.next() );
				m_lastRowId = 0;
				return m_rowIdList.at(0);
			}
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row IDs from machine list database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
			return -1;
		}
	} else if ( m_lastRowId > -1 ) {
		m_lastRowId++;
		if ( m_lastRowId < m_rowIdList.count() )
			return m_rowIdList.at(m_lastRowId);
		else
			return -1;
	}
	return -1;
}

QString MachineListDatabaseManager::id(int rowid)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE rowid=:rowid").arg(m_tableBasename));
	query.bindValue(":rowid", rowid);
	if ( query.exec() ) {
		if ( query.first() )
			return query.value(0).toString();
		else
			return QString();
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from machine list database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(query.lastError().text()));
		return QString();
	}
}

bool MachineListDatabaseManager::exists(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE id=:id LIMIT 1").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( query.exec() )
		return query.first();
	else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from machine list database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(query.lastError().text()));
		return false;
	}
}

void MachineListDatabaseManager::setData(const QString &id, const QString &description, const QString &manufacturer, const QString &year, const QString &cloneof, bool is_bios, bool is_device, bool has_roms, bool has_chds, int players, const QString &drvstat, const QString &srcfile)
{
	QSqlQuery query(m_db);
	query.prepare(QString("INSERT INTO %1 (id, description, manufacturer, year, cloneof, is_bios, is_device, has_roms, has_chds, players, drvstat, srcfile) VALUES (:id, :description, :manufacturer, :year, :cloneof, :is_bios, :is_device, :has_roms, :has_chds, :players, :drvstat, :srcfile)").arg(m_tableBasename));
	query.bindValue(":id", id);
	query.bindValue(":description", description);
	query.bindValue(":manufacturer", manufacturer);
	query.bindValue(":year", year);
	query.bindValue(":cloneof", cloneof);
	query.bindValue(":is_bios", is_bios);
	query.bindValue(":is_device", is_device);
	query.bindValue(":has_roms", has_roms);
	query.bindValue(":has_chds", has_chds);
	query.bindValue(":players", players);
	query.bindValue(":drvstat", drvstat);
	query.bindValue(":srcfile", srcfile);
	if ( !query.exec() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to machine list database: query = '%2', error = '%3'").arg(id).arg(query.lastQuery()).arg(query.lastError().text()));
}

quint64 MachineListDatabaseManager::databaseSize()
{
	QSqlQuery query(m_db);
	if ( query.exec("PRAGMA page_count") ) {
		if ( query.first() ) {
			quint64 page_count = query.value(0).toULongLong();
			query.finish();
			if ( query.exec("PRAGMA page_size") ) {
				if ( query.first() ) {
					quint64 page_size = query.value(0).toULongLong();
					return page_count * page_size;
				} else
					return 0;
			} else
				return 0;
		} else
			return 0;
	} else
		return 0;
}

void MachineListDatabaseManager::setCacheSize(quint64 kiloBytes)
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA cache_size = -%1").arg(kiloBytes)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the machine list database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(query.lastError().text()));
}

void MachineListDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( (int)syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes.at(syncMode))) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the machine list database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(query.lastError().text()));
}

void MachineListDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( (int)journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes.at(journalMode))) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the machine list database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(query.lastError().text()));
}

void MachineListDatabaseManager::recreateDatabase()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove machine list database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove machine list database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1_metadata").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove machine list database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT PRIMARY KEY, description TEXT, manufacturer TEXT, year TEXT, cloneof TEXT, is_bios BOOL, is_device BOOL, has_roms BOOL, has_chds BOOL, players INT, drvstat TEXT, srcfile TEXT, CONSTRAINT %1_unique_id UNIQUE (id))").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create machine list database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create machine list database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1_metadata (row INTEGER PRIMARY KEY, emu_version TEXT, qmc2_version TEXT, machinelist_version INTEGER)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create machine list database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
}
