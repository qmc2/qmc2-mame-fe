#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QUuid>

#include "macros.h"
#include "qmc2main.h"
#include "gamelist.h"
#include "settings.h"
#include "datinfodbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Gamelist *qmc2Gamelist;

DatInfoDatabaseManager::DatInfoDatabaseManager(QObject *parent)
	: QObject(parent)
{
	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	m_connectionName = QString("dat-info-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	m_db.setDatabaseName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/DatInfoDatabase", QString(userScopePath + "/%1-dat-info.db").arg(QMC2_EMU_NAME.toLower())).toString());
	m_softwareInfoTableName = "software_info";
	m_emuInfoTableName = "emu_info";
	m_gameInfoTableName = "game_info";
	m_metaDataTableName = "meta_data";
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() != 4 || !tables.contains(m_metaDataTableName) || !tables.contains(m_softwareInfoTableName) || !tables.contains(m_emuInfoTableName) || !tables.contains(m_gameInfoTableName) )
			recreateDatabase();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open dat-info database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
}

DatInfoDatabaseManager::~DatInfoDatabaseManager()
{
	if ( m_db.isOpen() )
		m_db.close();
}

QString DatInfoDatabaseManager::qmc2Version()
{
	QString qmc2_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1 WHERE row=0").arg(m_metaDataTableName));
	if ( query.exec() ) {
		if ( query.first() )
			qmc2_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return qmc2_version;
}

void DatInfoDatabaseManager::setQmc2Version(QString qmc2_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1 WHERE row=0").arg(m_metaDataTableName));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (qmc2_version, row) VALUES (:qmc2_version, 0)").arg(m_metaDataTableName));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to dat-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET qmc2_version=:qmc2_version WHERE row=0").arg(m_metaDataTableName));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in dat-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

int DatInfoDatabaseManager::datInfoVersion()
{
	int datinfo_version = -1;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT datinfo_version FROM %1 WHERE row=0").arg(m_metaDataTableName));
	if ( query.exec() ) {
		if ( query.first() )
			datinfo_version = query.value(0).toInt();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return datinfo_version;
}

void DatInfoDatabaseManager::setDatInfoVersion(int datinfo_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT datinfo_version FROM %1 WHERE row=0").arg(m_metaDataTableName));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (datinfo_version, row) VALUES (:datinfo_version, 0)").arg(m_metaDataTableName));
			query.bindValue(":datinfo_version", datinfo_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to dat-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET datinfo_version=:datinfo_version WHERE row=0").arg(m_metaDataTableName));
			query.bindValue(":datinfo_version", datinfo_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in dat-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

QString DatInfoDatabaseManager::softwareInfo(QString list, QString id)
{
	QString infotext;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext FROM %1 WHERE list=:list AND id=:id").arg(m_softwareInfoTableName));
	query.bindValue(":list", list);
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			infotext = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return infotext;
}

void DatInfoDatabaseManager::setSoftwareInfo(QString list, QString id, QString infotext)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext FROM %1 WHERE list=:list AND id=:id").arg(m_softwareInfoTableName));
	query.bindValue(":list", list);
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (list, id, infotext) VALUES (:list, :id, :infotext)").arg(m_softwareInfoTableName));
			query.bindValue(":list", list);
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE list=:list AND id=:id").arg(m_softwareInfoTableName));
			query.bindValue(":list", list);
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

bool DatInfoDatabaseManager::existsSoftwareInfo(QString list, QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT list, id FROM %1 WHERE list=:list AND id=:id").arg(m_softwareInfoTableName));
	query.bindValue(":list", list);
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			return (query.value(0).toString() == list) && (query.value(1).toString() == id);
		else
			return false;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("list, id").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return false;
	}
}

qint64 DatInfoDatabaseManager::softwareInfoRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_softwareInfoTableName)) ) {
		if ( query.first() )
			return query.value(0).toInt();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from dat-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}	return -1;
}

QString DatInfoDatabaseManager::emuInfo(QString id)
{
	QString infotext;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext FROM %1 WHERE id=:id").arg(m_emuInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			infotext = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return infotext;
}

void DatInfoDatabaseManager::setEmuInfo(QString id, QString infotext)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext FROM %1 WHERE id=:id").arg(m_emuInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, infotext) VALUES (:id, :infotext)").arg(m_emuInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE id=:id").arg(m_emuInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

bool DatInfoDatabaseManager::existsEmuInfo(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE id=:id").arg(m_emuInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			return (query.value(0).toString() == id);
		else
			return false;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return false;
	}
}

qint64 DatInfoDatabaseManager::emuInfoRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_emuInfoTableName)) ) {
		if ( query.first() )
			return query.value(0).toInt();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from dat-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}	return -1;
}

QString DatInfoDatabaseManager::gameInfo(QString id)
{
	QString infotext;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext FROM %1 WHERE id=:id").arg(m_gameInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			infotext = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return infotext;
}

void DatInfoDatabaseManager::setGameInfo(QString id, QString infotext)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext FROM %1 WHERE id=:id").arg(m_gameInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, infotext) VALUES (:id, :infotext)").arg(m_gameInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE id=:id").arg(m_gameInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

bool DatInfoDatabaseManager::existsGameInfo(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE id=:id").arg(m_gameInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			return (query.value(0).toString() == id);
		else
			return false;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from dat-info database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return false;
	}
}

qint64 DatInfoDatabaseManager::gameInfoRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_gameInfoTableName)) ) {
		if ( query.first() )
			return query.value(0).toInt();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from dat-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}	return -1;
}

quint64 DatInfoDatabaseManager::databaseSize()
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

void DatInfoDatabaseManager::setCacheSize(quint64 kiloBytes)
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA cache_size = -%1").arg(kiloBytes)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the dat-info database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes[syncMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the dat-info database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes[journalMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the dat-info database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::recreateSoftwareInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_softwareInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_softwareInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (list TEXT, id TEXT, infotext TEXT, PRIMARY KEY (list, id))").arg(m_softwareInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (list, id)").arg(m_softwareInfoTableName)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::recreateEmuInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_emuInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_emuInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT, infotext TEXT, PRIMARY KEY (id))").arg(m_emuInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_emuInfoTableName)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::recreateGameInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_gameInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("game-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_gameInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("game-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT, infotext TEXT, PRIMARY KEY (id))").arg(m_gameInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("game-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_gameInfoTableName)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("game-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::recreateMetaDataTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_metaDataTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("meta-data")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (row INTEGER PRIMARY KEY, qmc2_version TEXT, datinfo_version INTEGER)").arg(m_metaDataTableName)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("meta-data")).arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::recreateDatabase()
{
	recreateMetaDataTable();
	recreateSoftwareInfoTable();
	recreateEmuInfoTable();
	recreateGameInfoTable();
	setQmc2Version(XSTR(QMC2_VERSION));
	setDatInfoVersion(QMC2_DATINFO_VERSION);
}
