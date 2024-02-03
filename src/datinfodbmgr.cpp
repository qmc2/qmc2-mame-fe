#include <QApplication>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>
#include <QTextCodec>
#include <QXmlStreamReader>
#include <QRegExp>
#include <QDir>
#include <QUuid>
#include <QHash>

#include "macros.h"
#include "qmc2main.h"
#include "settings.h"
#include "options.h"
#include "datinfodbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern bool qmc2LoadingInterrupted;
extern QHash<QString, QString> softwareParentHash;

DatInfoDatabaseManager::DatInfoDatabaseManager(QObject *parent)
	: QObject(parent)
{
	m_connectionName = QString("dat-info-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	m_db.setDatabaseName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/DatInfoDatabase", QString(Options::configPath() + "/%1-dat-info.db").arg(QMC2_EMU_NAME.toLower())).toString());
	m_softwareInfoTableName = "software_info";
	m_emuInfoTableName = "emu_info";
	m_machineInfoTableName = "machine_info";
	m_metaDataTableName = "meta_data";
	if ( m_db.open() ) {
		QStringList tables(m_db.driver()->tables(QSql::Tables));
		if ( tables.count() >= 4 ) {
			switch ( datInfoVersion() ) {
				case 1:
					upgradeDatabaseFormat(1, 2);
					tables = m_db.driver()->tables(QSql::Tables);
					break;
				default:
					break;
			}
		}
		if ( tables.count() != 4 || !tables.contains(m_metaDataTableName) || !tables.contains(m_softwareInfoTableName) || !tables.contains(m_emuInfoTableName) || !tables.contains(m_machineInfoTableName) )
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET qmc2_version=:qmc2_version WHERE row=0").arg(m_metaDataTableName));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(query.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET datinfo_version=:datinfo_version WHERE row=0").arg(m_metaDataTableName));
			query.bindValue(":datinfo_version", datinfo_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

QString DatInfoDatabaseManager::softwareInfo(QString list, QString id, bool fromParent)
{
	if ( fromParent ) {
		QString parentKey(softwareParentHash.value(list + ':' + id));
		if ( !parentKey.isEmpty() && parentKey != "<np>" )
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE list=:list AND id=:id").arg(m_softwareInfoTableName));
			query.bindValue(":list", list);
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
}

bool DatInfoDatabaseManager::existsSoftwareInfo(QString list, QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT list, id FROM %1 WHERE list=:list AND id=:id LIMIT 1").arg(m_softwareInfoTableName));
	query.bindValue(":list", list);
	query.bindValue(":id", id);
	if ( query.exec() )
		return query.first();
	else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("list, id").arg(query.lastQuery()).arg(query.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return -1;
	}
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE id=:id").arg(m_emuInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
}

bool DatInfoDatabaseManager::existsEmuInfo(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE id=:id LIMIT 1").arg(m_emuInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() )
		return query.first();
	 else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(query.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return -1;
	}
}

QString DatInfoDatabaseManager::machineInfo(QString id)
{
	QString infotext;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext FROM %1 WHERE id=:id").arg(m_machineInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			infotext = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(query.lastError().text()));
	return infotext;
}

QString DatInfoDatabaseManager::machineInfoEmulator(QString id)
{
	QString emulator;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emulator FROM %1 WHERE id=:id").arg(m_machineInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			emulator = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("emulator").arg(query.lastQuery()).arg(query.lastError().text()));
	return emulator;
}

void DatInfoDatabaseManager::setMachineInfo(QString id, QString infotext, QString emulator)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT infotext, emulator FROM %1 WHERE id=:id").arg(m_machineInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, infotext, emulator) VALUES (:id, :infotext, :emulator)").arg(m_machineInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			query.bindValue(":emulator", emulator);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext, emulator=:emulator WHERE id=:id").arg(m_machineInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			query.bindValue(":emulator", emulator);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(query.lastError().text()));
}

bool DatInfoDatabaseManager::existsMachineInfo(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE id=:id LIMIT 1").arg(m_machineInfoTableName));
	query.bindValue(":id", id);
	if ( query.exec() )
		return query.first();
	else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(query.lastError().text()));
		return false;
	}
}

qint64 DatInfoDatabaseManager::machineInfoRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_machineInfoTableName)) ) {
		if ( query.first() )
			return query.value(0).toLongLong();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return -1;
	}
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(query.lastError().text()));
}

void DatInfoDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( (int)syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes[syncMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(query.lastError().text()));
}

void DatInfoDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( (int)journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes[journalMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(query.lastError().text()));
}

void DatInfoDatabaseManager::recreateSoftwareInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_softwareInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_softwareInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (list TEXT, id TEXT, infotext TEXT, PRIMARY KEY (list, id))").arg(m_softwareInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (list, id)").arg(m_softwareInfoTableName)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(query.lastError().text()));
}

void DatInfoDatabaseManager::recreateEmuInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_emuInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_emuInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT, infotext TEXT, PRIMARY KEY (id))").arg(m_emuInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_emuInfoTableName)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(query.lastError().text()));
}

void DatInfoDatabaseManager::recreateMachineInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_machineInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_machineInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT, infotext TEXT, emulator TEXT, PRIMARY KEY (id))").arg(m_machineInfoTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_machineInfoTableName)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(query.lastError().text()));
}

void DatInfoDatabaseManager::recreateMetaDataTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_metaDataTableName)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("meta-data")).arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (row INTEGER PRIMARY KEY, qmc2_version TEXT, datinfo_version INTEGER)").arg(m_metaDataTableName)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("meta-data")).arg(query.lastQuery()).arg(query.lastError().text()));
}

void DatInfoDatabaseManager::recreateDatabase()
{
	recreateMetaDataTable();
	recreateSoftwareInfoTable();
	recreateEmuInfoTable();
	recreateMachineInfoTable();
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
			uint dtImport = importDates[importFiles.indexOf(path)].toUInt();
			if ( dtImport < fi.lastModified().toTime_t() )
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
			qmc2LoadingInterrupted = false;
			beginTransaction();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				qmc2MainWindow->progressBarMachineList->setFormat(tr("Software info - %p%"));
			else
				qmc2MainWindow->progressBarMachineList->setFormat("%p%");
			qmc2MainWindow->progressBarMachineList->setRange(0, swInfoDB.size());
			qmc2MainWindow->progressBarMachineList->setValue(0);
			qApp->processEvents();
			QXmlStreamReader xsr(&swInfoDB);
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegularExpression headerRx("^\n?(.*)\n{2,}");
			QRegularExpression textEndRx("\n\t{2}$");
			QRegularExpression doubleLineBreakRx("\n{2,}");
			QRegularExpression singleLineBreakRx("\n");
			if ( xsr.readNextStartElement() && !qmc2LoadingInterrupted ) {
				if ( xsr.name() == "history" ) {
					if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
						qmc2MainWindow->progressBarMachineList->setValue(swInfoDB.pos());
						qApp->processEvents();
					}
					while ( xsr.readNextStartElement() && !qmc2LoadingInterrupted ) {
						if ( xsr.name() == "entry" ) {
							QStringList systemNames;
							QStringList gameNames;
							QString swInfoString;
							if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
								qmc2MainWindow->progressBarMachineList->setValue(swInfoDB.pos());
								qApp->processEvents();
							}
							while ( xsr.readNextStartElement() && !qmc2LoadingInterrupted ) {
								if ( xsr.name() == "software" ) {
									if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
										qmc2MainWindow->progressBarMachineList->setValue(swInfoDB.pos());
										qApp->processEvents();
									}
									while ( xsr.readNextStartElement() && !qmc2LoadingInterrupted ) {
										if ( xsr.name() == "item" && xsr.attributes().hasAttribute("list") && xsr.attributes().hasAttribute("name")) {
											if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
												qmc2MainWindow->progressBarMachineList->setValue(swInfoDB.pos());
												qApp->processEvents();
											}
											systemNames << xsr.attributes().value("list").toString();
											gameNames << xsr.attributes().value("name").toString();
											xsr.skipCurrentElement();
										}
									}
								}
								else if ( xsr.name() == "text") {
									if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
										qmc2MainWindow->progressBarMachineList->setValue(swInfoDB.pos());
										qApp->processEvents();
									}
									swInfoString = xsr.readElementText();
								}
								else {
									if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
										qmc2MainWindow->progressBarMachineList->setValue(swInfoDB.pos());
										qApp->processEvents();
									}
									xsr.skipCurrentElement();
								}
							}
							// format for display in frontend
							swInfoString.replace(headerRx, "<b>\\1</b><p>");
							swInfoString.replace(textEndRx, "");
							swInfoString.replace(doubleLineBreakRx, "<p>");
							swInfoString.replace(singleLineBreakRx, "<br>");
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
						else
							xsr.skipCurrentElement();
					}
				}
				else
					xsr.skipCurrentElement();
			}
			commitTransaction();
			qmc2MainWindow->progressBarMachineList->setValue(swInfoDB.pos());
			if ( qmc2LoadingInterrupted ) {
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

	if ( !qmc2LoadingInterrupted )
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
	qmc2MainWindow->progressBarMachineList->reset();
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
			uint dtImport = importDates[importFiles.indexOf(path)].toUInt();
			if ( dtImport < fi.lastModified().toTime_t() )
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
			qmc2LoadingInterrupted = false;
			beginTransaction();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				qmc2MainWindow->progressBarMachineList->setFormat(tr("Emu info - %p%"));
			else
				qmc2MainWindow->progressBarMachineList->setFormat("%p%");
			qmc2MainWindow->progressBarMachineList->setRange(0, emuInfoDB.size());
			qmc2MainWindow->progressBarMachineList->setValue(0);
			qApp->processEvents();
			QTextStream ts(&emuInfoDB);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegExp lineBreakRx("(<br>){2,}");
			while ( !ts.atEnd() && !qmc2LoadingInterrupted ) {
				QString singleLineSimplified = ts.readLine().simplified();
				bool startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				while ( !startsWithDollarInfo && !ts.atEnd() ) {
					singleLineSimplified = ts.readLine().simplified();
					if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
						qmc2MainWindow->progressBarMachineList->setValue(emuInfoDB.pos());
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
							qmc2MainWindow->progressBarMachineList->setValue(emuInfoDB.pos());
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
								qmc2MainWindow->progressBarMachineList->setValue(emuInfoDB.pos());
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
			qmc2MainWindow->progressBarMachineList->setValue(emuInfoDB.pos());
			if ( qmc2LoadingInterrupted ) {
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

	if ( !qmc2LoadingInterrupted )
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
	qmc2MainWindow->progressBarMachineList->reset();
}

bool DatInfoDatabaseManager::machineInfoImportRequired(QStringList pathList)
{
	QStringList importFiles = qmc2Config->value(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/MachineInfoImportFiles", QStringList()).toStringList();
	QStringList importDates = qmc2Config->value(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/MachineInfoImportDates", QStringList()).toStringList();

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
			uint dtImport = importDates[importFiles.indexOf(path)].toUInt();
			if ( dtImport < fi.lastModified().toTime_t() )
				datesChanged = true;
			if ( datesChanged )
				break;
		}
		if ( datesChanged )
			return true;
		else
			return machineInfoRowCount() == 0;
	} else
		return true;
}

void DatInfoDatabaseManager::importMachineInfo(QStringList pathList, QStringList emulatorList, bool fromScratch)
{
	if ( fromScratch )
		recreateMachineInfoTable();

	QStringList importPaths, importDates;
	for (int index = 0; index < pathList.count(); index++) {
		QString path = pathList[index];
		QString emulator = emulatorList[index];
		if ( path.isEmpty() || emulator.isEmpty() )
			continue;
		QFile machineInfoDB(path);
		if ( machineInfoDB.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("importing %1 from '%2'").arg(tr("machine info-texts")).arg(QDir::toNativeSeparators(path)));
			qApp->processEvents();
			qmc2LoadingInterrupted = false;
			beginTransaction();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				qmc2MainWindow->progressBarMachineList->setFormat(tr("Machine info - %p%"));
			else
				qmc2MainWindow->progressBarMachineList->setFormat("%p%");
			qmc2MainWindow->progressBarMachineList->setRange(0, machineInfoDB.size());
			qmc2MainWindow->progressBarMachineList->setValue(0);
			qApp->processEvents();
			QXmlStreamReader xsr(&machineInfoDB);
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegularExpression headerRx("^\n?(.*)\n{2,}");
			QRegularExpression textEndRx("\n\t{2}$");
			QRegularExpression doubleLineBreakRx("\n{2,}");
			QRegularExpression singleLineBreakRx("\n");
			if ( xsr.readNextStartElement() && !qmc2LoadingInterrupted ) {
				if ( xsr.name() == "history" ) {
					if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
						qmc2MainWindow->progressBarMachineList->setValue(machineInfoDB.pos());
						qApp->processEvents();
					}
					while ( xsr.readNextStartElement() && !qmc2LoadingInterrupted ) {
						if ( xsr.name() == "entry" ) {
							QStringList gameNames;
							QString machineInfoString;
							if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
								qmc2MainWindow->progressBarMachineList->setValue(machineInfoDB.pos());
								qApp->processEvents();
							}
							while ( xsr.readNextStartElement() && !qmc2LoadingInterrupted ) {
								if ( xsr.name() == "systems" ) {
									if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
										qmc2MainWindow->progressBarMachineList->setValue(machineInfoDB.pos());
										qApp->processEvents();
									}
									while ( xsr.readNextStartElement() && !qmc2LoadingInterrupted ) {
										if ( xsr.name() == "system" && xsr.attributes().hasAttribute("name")) {
											if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
												qmc2MainWindow->progressBarMachineList->setValue(machineInfoDB.pos());
												qApp->processEvents();
											}
											gameNames << xsr.attributes().value("name").toString();
											xsr.skipCurrentElement();
										}
									}
								}
								else if ( xsr.name() == "text") {
									if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
										qmc2MainWindow->progressBarMachineList->setValue(machineInfoDB.pos());
										qApp->processEvents();
									}
									machineInfoString = xsr.readElementText();
								}
								else {
									if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
										qmc2MainWindow->progressBarMachineList->setValue(machineInfoDB.pos());
										qApp->processEvents();
									}
									xsr.skipCurrentElement();
								}
							}
							// format for display in frontend
							machineInfoString.replace(headerRx, "<h2>\\1</h2>");
							machineInfoString.replace(textEndRx, "");
							machineInfoString.replace(doubleLineBreakRx, "<p>");
							machineInfoString.replace(singleLineBreakRx, "<br>");
							foreach (QString setName, gameNames) {
								setMachineInfo(setName, machineInfoString, emulator);
								pendingUpdates++;
							}
							if ( pendingUpdates > QMC2_DATINFO_COMMIT ) {
								commitTransaction();
								pendingUpdates = 0;
								beginTransaction();
							}
						}
						else
							xsr.skipCurrentElement();
					}
				}
				else
					xsr.skipCurrentElement();
			}
			commitTransaction();
			qmc2MainWindow->progressBarMachineList->setValue(machineInfoDB.pos());
			if ( qmc2LoadingInterrupted ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("import stopped, invalidating %1 table").arg(tr("machine info")));
				recreateMachineInfoTable();
				break;
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("done (importing %1 from '%2')").arg(tr("machine info-texts")).arg(QDir::toNativeSeparators(path)));
			importPaths << path;
			importDates << QString::number(QFileInfo(path).lastModified().toTime_t());
			machineInfoDB.close();
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("WARNING: can't open machine info file %1").arg(QDir::toNativeSeparators(path)));
	}

	if ( !qmc2LoadingInterrupted )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("DAT-info database") + ": " + tr("%n machine info record(s) imported", "", machineInfoRowCount()));

	if ( !importPaths.isEmpty() ) {
		qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/MachineInfoImportFiles", importPaths);
		qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/MachineInfoImportDates", importDates);
	} else {
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/MachineInfoImportFiles");
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "DatInfoDatabase/MachineInfoImportDates");
	}

	setQmc2Version(XSTR(QMC2_VERSION));
	setDatInfoVersion(QMC2_DATINFO_VERSION);
	qmc2MainWindow->progressBarMachineList->reset();		
}

void DatInfoDatabaseManager::upgradeDatabaseFormat(int from, int to)
{
	switch ( from ) {
		case 1:
			if ( to >= 2 ) {
				QSqlQuery query(m_db);
				if ( query.exec("ALTER TABLE game_info RENAME TO machine_info") ) {
					if ( query.exec("DROP INDEX IF EXISTS game_info_index") ) {
						if ( query.exec("CREATE INDEX machine_info_index ON machine_info (id)") ) {
							setQmc2Version(XSTR(QMC2_VERSION));
							setDatInfoVersion(QMC2_DATINFO_VERSION);
						} else
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to alter %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(query.lastError().text()));
					} else
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to alter %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(query.lastError().text()));
				} else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to alter %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(query.lastError().text()));
			}
			break;
		default:
			break;
	}
}
