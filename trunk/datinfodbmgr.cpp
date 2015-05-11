#include <QApplication>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>
#include <QTextCodec>
#include <QRegExp>
#include <QDir>
#include <QUuid>
#include <QHash>

#include "macros.h"
#include "qmc2main.h"
#include "settings.h"
#include "datinfodbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern bool qmc2StopParser;
extern QHash<QString, QString> softwareParentHash;

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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open DAT-info database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET qmc2_version=:qmc2_version WHERE row=0").arg(m_metaDataTableName));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET datinfo_version=:datinfo_version WHERE row=0").arg(m_metaDataTableName));
			query.bindValue(":datinfo_version", datinfo_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

QString DatInfoDatabaseManager::softwareInfo(QString list, QString id, bool fromParent)
{
	if ( fromParent ) {
		QString parentKey = softwareParentHash[list + ":" + id];
		if ( !parentKey.isEmpty() && parentKey != "<no_parent>" )
			id = parentKey.split(":", QString::SkipEmptyParts)[1];
	}
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	if ( infotext.isEmpty() && !fromParent )
		return softwareInfo(list, id, true);
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE list=:list AND id=:id").arg(m_softwareInfoTableName));
			query.bindValue(":list", list);
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("list, id").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return false;
	}
}

qint64 DatInfoDatabaseManager::softwareInfoRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_softwareInfoTableName)) ) {
		if ( query.first() )
			return query.value(0).toLongLong();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE id=:id").arg(m_emuInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return false;
	}
}

qint64 DatInfoDatabaseManager::emuInfoRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_emuInfoTableName)) ) {
		if ( query.first() )
			return query.value(0).toLongLong();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return infotext;
}

QString DatInfoDatabaseManager::gameInfoEmulator(QString id)
{
	QString emulator;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emulator FROM %1 WHERE id=:id").arg(m_gameInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			emulator = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("emulator").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return emulator;
}

void DatInfoDatabaseManager::setGameInfo(QString id, QString infotext, QString emulator)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext, emulator FROM %1 WHERE id=:id").arg(m_gameInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, infotext, emulator) VALUES (:id, :infotext, :emulator)").arg(m_gameInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			query.bindValue(":emulator", emulator);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext, emulator=:emulator WHERE id=:id").arg(m_gameInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			query.bindValue(":emulator", emulator);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return false;
	}
}

qint64 DatInfoDatabaseManager::gameInfoRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_gameInfoTableName)) ) {
		if ( query.first() )
			return query.value(0).toLongLong();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( (int)syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes[syncMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void DatInfoDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( (int)journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes[journalMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT, infotext TEXT, emulator TEXT, PRIMARY KEY (id))").arg(m_gameInfoTableName)) ) {
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

bool DatInfoDatabaseManager::softwareInfoImportRequired(QStringList pathList)
{
	QStringList importFiles = qmc2Config->value(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/SoftwareInfoImportFiles", QStringList()).toStringList();
	QStringList importDates = qmc2Config->value(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/SoftwareInfoImportDates", QStringList()).toStringList();

	if ( importFiles.isEmpty() || importDates.isEmpty() )
		return true;

	if ( importFiles.count() != importDates.count() )
		return true;

	bool sameFiles = true;
	foreach (QString path, pathList) {
		if ( !importFiles.contains(path) )
			sameFiles = false;
		if ( !sameFiles )
			break;
	}

	if ( sameFiles ) {
		bool datesChanged = false;
		foreach (QString path, pathList) {
			QFileInfo fi(path);
			QDateTime dtImport = QDateTime::fromTime_t(importDates[importFiles.indexOf(path)].toULongLong());
			if ( dtImport != fi.lastModified() )
				datesChanged = true;
			if ( datesChanged )
				break;
		}
		if ( datesChanged )
			return true;
		else
			return softwareInfoRowCount() == 0;
	} else
		return true;
}

void DatInfoDatabaseManager::importSoftwareInfo(QStringList pathList, bool fromScratch)
{
	if ( fromScratch )
		recreateSoftwareInfoTable();

	QStringList importPaths, importDates;
	foreach(QString path, pathList) {
		if ( path.isEmpty() )
			continue;
		QFile swInfoDB(path);
		if ( swInfoDB.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("importing %1 from '%2'").arg(tr("software info-texts")).arg(QDir::toNativeSeparators(path)));
			qApp->processEvents();
			qmc2StopParser = false;
			beginTransaction();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				qmc2MainWindow->progressBarGamelist->setFormat(tr("Software info - %p%"));
			else
				qmc2MainWindow->progressBarGamelist->setFormat("%p%");
			qmc2MainWindow->progressBarGamelist->setRange(0, swInfoDB.size());
			qmc2MainWindow->progressBarGamelist->setValue(0);
			qApp->processEvents();
			QTextStream ts(&swInfoDB);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegExp markRegExp("^\\$\\S+\\=\\S+\\,$");
			QRegExp reduceLinesRegExp("(<br>){2,}");
			while ( !ts.atEnd() && !qmc2StopParser ) {
				QString singleLineSimplified = ts.readLine().simplified();
				bool containsMark = singleLineSimplified.contains(markRegExp);
				while ( !containsMark && !ts.atEnd() ) {
					singleLineSimplified = ts.readLine().simplified();
					if ( recordsProcessed++ % QMC2_SWINFO_RESPONSIVENESS == 0 ) {
						qmc2MainWindow->progressBarGamelist->setValue(swInfoDB.pos());
						qApp->processEvents();
					}
					containsMark = singleLineSimplified.contains(markRegExp);
				}
				if ( containsMark && !singleLineSimplified.startsWith("$info=") ) {
					QStringList infoWords = singleLineSimplified.mid(1).split("=", QString::SkipEmptyParts);
					if ( infoWords.count() == 2 ) {
						QStringList systemNames = infoWords[0].split(",", QString::SkipEmptyParts);
						QStringList gameNames = infoWords[1].split(",", QString::SkipEmptyParts);
						bool startsWithDollarBio = false;
						while ( !startsWithDollarBio && !ts.atEnd() ) {
							singleLineSimplified = ts.readLine().simplified();
							if ( recordsProcessed++ % QMC2_SWINFO_RESPONSIVENESS == 0 ) {
								qmc2MainWindow->progressBarGamelist->setValue(swInfoDB.pos());
								qApp->processEvents();
							}
							startsWithDollarBio = singleLineSimplified.startsWith("$bio");
						}
						if ( startsWithDollarBio ) {
							QString swInfoString;
							bool firstLine = true;
							bool startsWithDollarEnd = false;
							while ( !startsWithDollarEnd && !ts.atEnd() ) {
								QString singleLine = ts.readLine();
								singleLineSimplified = singleLine.simplified();
								startsWithDollarEnd = singleLineSimplified.startsWith("$end");
								if ( !startsWithDollarEnd ) {
									if ( !firstLine )
										swInfoString.append(singleLine + "<br>");
									else if ( !singleLine.isEmpty() ) {
										swInfoString.append("<b>" + singleLine + "</b><br>");
										firstLine = false;
									}
								}
								if ( recordsProcessed++ % QMC2_SWINFO_RESPONSIVENESS == 0 ) {
									qmc2MainWindow->progressBarGamelist->setValue(swInfoDB.pos());
									qApp->processEvents();
								}
							}
							if ( startsWithDollarEnd ) {
								// reduce the number of line breaks
								swInfoString.replace(reduceLinesRegExp, "<p>");
								if ( swInfoString.endsWith("<p>") )
									swInfoString.remove(swInfoString.length() - 3, 3);
								foreach (QString gameName, gameNames) {
									foreach (QString systemName, systemNames) {
										setSoftwareInfo(systemName, gameName, swInfoString);
										pendingUpdates++;
									}
								}
								if ( pendingUpdates > QMC2_DATINFO_COMMIT ) {
									commitTransaction();
									pendingUpdates = 0;
									beginTransaction();
								}
							}
						}
					}
				}
			}
			commitTransaction();
			qmc2MainWindow->progressBarGamelist->setValue(swInfoDB.pos());
			if ( qmc2StopParser ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("import stopped, invalidating %1 table").arg(tr("software info")));
				recreateSoftwareInfoTable();
				break;
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("done (importing %1 from '%2')").arg(tr("software info-texts")).arg(QDir::toNativeSeparators(path)));
			importPaths << path;
			importDates << QString::number(QFileInfo(path).lastModified().toTime_t());
			swInfoDB.close();
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: can't open software info file '%1'").arg(QDir::toNativeSeparators(path)));
	}

	if ( !qmc2StopParser )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("%n software info record(s) imported", "", softwareInfoRowCount()));

	if ( !importPaths.isEmpty() ) {
		qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/SoftwareInfoImportFiles", importPaths);
		qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/SoftwareInfoImportDates", importDates);
	} else {
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/SoftwareInfoImportFiles");
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/SoftwareInfoImportDates");
	}

	setQmc2Version(XSTR(QMC2_VERSION));
	setDatInfoVersion(QMC2_DATINFO_VERSION);
	qmc2MainWindow->progressBarGamelist->reset();
}

bool DatInfoDatabaseManager::emuInfoImportRequired(QStringList pathList)
{
	QStringList importFiles = qmc2Config->value(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/EmuInfoImportFiles", QStringList()).toStringList();
	QStringList importDates = qmc2Config->value(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/EmuInfoImportDates", QStringList()).toStringList();

	if ( importFiles.isEmpty() || importDates.isEmpty() )
		return true;

	if ( importFiles.count() != importDates.count() )
		return true;

	bool sameFiles = true;
	foreach (QString path, pathList) {
		if ( !importFiles.contains(path) )
			sameFiles = false;
		if ( !sameFiles )
			break;
	}

	if ( sameFiles ) {
		bool datesChanged = false;
		foreach (QString path, pathList) {
			QFileInfo fi(path);
			QDateTime dtImport = QDateTime::fromTime_t(importDates[importFiles.indexOf(path)].toULongLong());
			if ( dtImport != fi.lastModified() )
				datesChanged = true;
			if ( datesChanged )
				break;
		}
		if ( datesChanged )
			return true;
		else
			return emuInfoRowCount() == 0;
	} else
		return true;
}

void DatInfoDatabaseManager::importEmuInfo(QStringList pathList, bool fromScratch)
{
	if ( fromScratch )
		recreateEmuInfoTable();

	QStringList importPaths, importDates;
	foreach(QString path, pathList) {
		if ( path.isEmpty() )
			continue;
		QFile emuInfoDB(path);
		if ( emuInfoDB.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("importing %1 from '%2'").arg(tr("emulator info-texts")).arg(QDir::toNativeSeparators(path)));
			qApp->processEvents();
			qmc2StopParser = false;
			beginTransaction();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				qmc2MainWindow->progressBarGamelist->setFormat(tr("Emu info - %p%"));
			else
				qmc2MainWindow->progressBarGamelist->setFormat("%p%");
			qmc2MainWindow->progressBarGamelist->setRange(0, emuInfoDB.size());
			qmc2MainWindow->progressBarGamelist->setValue(0);
			qApp->processEvents();
			QTextStream ts(&emuInfoDB);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegExp lineBreakRx("(<br>){2,}");
			while ( !ts.atEnd() && !qmc2StopParser ) {
				QString singleLineSimplified = ts.readLine().simplified();
				bool startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				while ( !startsWithDollarInfo && !ts.atEnd() ) {
					singleLineSimplified = ts.readLine().simplified();
					if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
						qmc2MainWindow->progressBarGamelist->setValue(emuInfoDB.pos());
						qApp->processEvents();
					}
					startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				}
				if ( startsWithDollarInfo ) {
					QStringList gameNames = singleLineSimplified.mid(6).split(",", QString::SkipEmptyParts);
					bool startsWithDollarMame = false;
					while ( !startsWithDollarMame && !ts.atEnd() ) {
						singleLineSimplified = ts.readLine().simplified();
						if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
							qmc2MainWindow->progressBarGamelist->setValue(emuInfoDB.pos());
							qApp->processEvents();
						}
						startsWithDollarMame = singleLineSimplified.startsWith("$mame");
					}
					if ( startsWithDollarMame ) {
						QString emuInfoString;
						bool startsWithDollarEnd = false;
						while ( !startsWithDollarEnd && !ts.atEnd() ) {
							QString singleLine = ts.readLine();
							singleLineSimplified = singleLine.simplified();
							startsWithDollarEnd = singleLineSimplified.startsWith("$end");
							if ( !startsWithDollarEnd )
								emuInfoString.append(singleLine + "<br>");
							if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
								qmc2MainWindow->progressBarGamelist->setValue(emuInfoDB.pos());
								qApp->processEvents();
							}
						}
						if ( startsWithDollarEnd ) {
							// reduce the number of line breaks
							emuInfoString.replace(lineBreakRx, "<p>");
							if ( emuInfoString.startsWith("<br>") )
								emuInfoString.remove(0, 4);
							if ( emuInfoString.endsWith("<p>") )
								emuInfoString.remove(emuInfoString.length() - 3, 3);
							foreach (QString setName, gameNames) {
								setEmuInfo(setName, emuInfoString);
								pendingUpdates++;
							}
							if ( pendingUpdates > QMC2_DATINFO_COMMIT ) {
								commitTransaction();
								pendingUpdates = 0;
								beginTransaction();
							}
						} else
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: missing '$end' in emulator info file %1").arg(QDir::toNativeSeparators(path)));
					} else if ( !ts.atEnd() )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: missing '$mame' in emulator info file %1").arg(QDir::toNativeSeparators(path)));
				} else if ( !ts.atEnd() )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: missing '$info' in emulator info file %1").arg(QDir::toNativeSeparators(path)));
			}
			commitTransaction();
			qmc2MainWindow->progressBarGamelist->setValue(emuInfoDB.pos());
			if ( qmc2StopParser ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("import stopped, invalidating %1 table").arg(tr("emu info")));
				recreateEmuInfoTable();
				break;
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("done (importing %1 from '%2')").arg(tr("emulator info-texts")).arg(QDir::toNativeSeparators(path)));
			importPaths << path;
			importDates << QString::number(QFileInfo(path).lastModified().toTime_t());
			emuInfoDB.close();

		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: can't open emulator info file %1").arg(QDir::toNativeSeparators(path)));
	}

	if ( !qmc2StopParser )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("%n emulator info record(s) imported", "", emuInfoRowCount()));

	if ( !importPaths.isEmpty() ) {
		qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/EmuInfoImportFiles", importPaths);
		qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/EmuInfoImportDates", importDates);
	} else {
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/EmuInfoImportFiles");
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/EmuInfoImportDates");
	}

	setQmc2Version(XSTR(QMC2_VERSION));
	setDatInfoVersion(QMC2_DATINFO_VERSION);
	qmc2MainWindow->progressBarGamelist->reset();
}

bool DatInfoDatabaseManager::gameInfoImportRequired(QStringList pathList)
{
	QStringList importFiles = qmc2Config->value(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/GameInfoImportFiles", QStringList()).toStringList();
	QStringList importDates = qmc2Config->value(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/GameInfoImportDates", QStringList()).toStringList();

	if ( importFiles.isEmpty() || importDates.isEmpty() )
		return true;

	if ( importFiles.count() != importDates.count() )
		return true;

	bool sameFiles = true;
	foreach (QString path, pathList) {
		if ( !importFiles.contains(path) )
			sameFiles = false;
		if ( !sameFiles )
			break;
	}

	if ( sameFiles ) {
		bool datesChanged = false;
		foreach (QString path, pathList) {
			QFileInfo fi(path);
			QDateTime dtImport = QDateTime::fromTime_t(importDates[importFiles.indexOf(path)].toULongLong());
			if ( dtImport != fi.lastModified() )
				datesChanged = true;
			if ( datesChanged )
				break;
		}
		if ( datesChanged )
			return true;
		else
			return gameInfoRowCount() == 0;
	} else
		return true;
}

void DatInfoDatabaseManager::importGameInfo(QStringList pathList, QStringList emulatorList, bool fromScratch)
{
	if ( fromScratch )
		recreateGameInfoTable();

	QStringList importPaths, importDates;
	for (int index = 0; index < pathList.count(); index++) {
		QString path = pathList[index];
		QString emulator = emulatorList[index];
		if ( path.isEmpty() || emulator.isEmpty() )
			continue;
		QFile gameInfoDB(path);
		if ( gameInfoDB.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("importing %1 from '%2'").arg(tr("system info-texts")).arg(QDir::toNativeSeparators(path)));
			qApp->processEvents();
			qmc2StopParser = false;
			beginTransaction();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				qmc2MainWindow->progressBarGamelist->setFormat(tr("Game info - %p%"));
			else
				qmc2MainWindow->progressBarGamelist->setFormat("%p%");
			qmc2MainWindow->progressBarGamelist->setRange(0, gameInfoDB.size());
			qmc2MainWindow->progressBarGamelist->setValue(0);
			qApp->processEvents();
			QTextStream ts(&gameInfoDB);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegExp lineBreakRx("(<br>){2,}");
			while ( !ts.atEnd() && !qmc2StopParser ) {
				QString singleLineSimplified = ts.readLine().simplified();
				bool startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				while ( !startsWithDollarInfo && !ts.atEnd() ) {
					singleLineSimplified = ts.readLine().simplified();
					if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
						qmc2MainWindow->progressBarGamelist->setValue(gameInfoDB.pos());
						qApp->processEvents();
					}
					startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				}
				if ( startsWithDollarInfo ) {
					QStringList gameNames = singleLineSimplified.mid(6).split(",", QString::SkipEmptyParts);
					bool startsWithDollarBio = false;
					while ( !startsWithDollarBio && !ts.atEnd() ) {
						singleLineSimplified = ts.readLine().simplified();
						if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
							qmc2MainWindow->progressBarGamelist->setValue(gameInfoDB.pos());
							qApp->processEvents();
						}
						startsWithDollarBio = singleLineSimplified.startsWith("$bio");
					}
					if ( startsWithDollarBio ) {
						QString gameInfoString;
						bool firstLine = true;
						bool lastLineWasHeader = false;
						bool startsWithDollarEnd = false;
						while ( !startsWithDollarEnd && !ts.atEnd() ) {
							QString singleLine = ts.readLine();
							singleLineSimplified = singleLine.simplified();
							startsWithDollarEnd = singleLineSimplified.startsWith("$end");
							if ( !startsWithDollarEnd ) {
								if ( !firstLine ) {
									if ( !lastLineWasHeader )
										gameInfoString.append(singleLine + "<br>");
									lastLineWasHeader = false;
								} else if ( !singleLine.isEmpty() ) {
									gameInfoString.append("<h2>" + singleLine + "</h2>");
									firstLine = false;
									lastLineWasHeader = true;
								}
							}
							if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
								qmc2MainWindow->progressBarGamelist->setValue(gameInfoDB.pos());
								qApp->processEvents();
							}
						}
						if ( startsWithDollarEnd ) {
							// reduce the number of line breaks
							gameInfoString.replace(lineBreakRx, "<p>");
							if ( gameInfoString.endsWith("<p>") )
								gameInfoString.remove(gameInfoString.length() - 3, 3);
							foreach (QString setName, gameNames) {
								setGameInfo(setName, gameInfoString, emulator);
								pendingUpdates++;
							}
							if ( pendingUpdates > QMC2_DATINFO_COMMIT ) {
								commitTransaction();
								pendingUpdates = 0;
								beginTransaction();
							}
						} else
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: missing '$end' in game info file %1").arg(QDir::toNativeSeparators(path)));
					} else
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: missing '$bio' in game info file %1").arg(QDir::toNativeSeparators(path)));
				} else if ( !ts.atEnd() )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: missing '$info' in game info file %1").arg(QDir::toNativeSeparators(path)));
			}
			commitTransaction();
			qmc2MainWindow->progressBarGamelist->setValue(gameInfoDB.pos());
			if ( qmc2StopParser ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("import stopped, invalidating %1 table").arg(tr("game info")));
				recreateGameInfoTable();
				break;
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("done (importing %1 from '%2')").arg(tr("system info-texts")).arg(QDir::toNativeSeparators(path)));
			importPaths << path;
			importDates << QString::number(QFileInfo(path).lastModified().toTime_t());
			gameInfoDB.close();
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: can't open game info file %1").arg(QDir::toNativeSeparators(path)));
	}

	if ( !qmc2StopParser )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("%n game info record(s) imported", "", gameInfoRowCount()));

	if ( !importPaths.isEmpty() ) {
		qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/GameInfoImportFiles", importPaths);
		qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/GameInfoImportDates", importDates);
	} else {
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/GameInfoImportFiles");
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/GameInfoImportDates");
	}

	setQmc2Version(XSTR(QMC2_VERSION));
	setDatInfoVersion(QMC2_DATINFO_VERSION);
	qmc2MainWindow->progressBarGamelist->reset();		
}
