#include <qglobal.h>

#if QT_VERSION < 0x050000
#include <QApplication>
#else
#include <QGuiApplication>
#endif
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

#include "macros.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "datinfodbmgr.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

DatInfoDatabaseManager::DatInfoDatabaseManager(QObject *parent)
	: QObject(parent)
{
	m_connectionName = QString("dat-info-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	m_db.setDatabaseName(globalConfig->datInfoDatabaseName());
	m_softwareInfoTableName = "software_info";
	m_emuInfoTableName = "emu_info";
	m_gameInfoTableName = "game_info";
	m_metaDataTableName = "meta_data";
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() != 4 || !tables.contains(m_metaDataTableName) || !tables.contains(m_softwareInfoTableName) || !tables.contains(m_emuInfoTableName) || !tables.contains(m_gameInfoTableName) )
			recreateDatabase();
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to open DAT-info database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
	}
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
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET qmc2_version=:qmc2_version WHERE row=0").arg(m_metaDataTableName));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		}
		query.finish();
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET datinfo_version=:datinfo_version WHERE row=0").arg(m_metaDataTableName));
			query.bindValue(":datinfo_version", datinfo_version);
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		}
		query.finish();
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("datinfo_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE list=:list AND id=:id").arg(m_softwareInfoTableName));
			query.bindValue(":list", list);
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		}
		query.finish();
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("list, id").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext WHERE id=:id").arg(m_emuInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		}
		query.finish();
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return -1;
	}
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
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("emulator").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to add '%1' to DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET infotext=:infotext, emulator=:emulator WHERE id=:id").arg(m_gameInfoTableName));
			query.bindValue(":id", id);
			query.bindValue(":infotext", infotext);
			query.bindValue(":emulator", emulator);
			if ( !query.exec() ) {
				QMC2_ARCADE_LOG_STR(tr("WARNING: failed to update '%1' in DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(m_db.lastError().text()));
			}
		}
		query.finish();
	} else {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("infotext, emulator").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
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
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch '%1' from DAT-info database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to fetch row count from DAT-info database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
	if ( !query.exec(QString("PRAGMA cache_size = -%1").arg(kiloBytes)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
}

void DatInfoDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( (int)syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes[syncMode])) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(m_db.lastError().text()));
    }
}

void DatInfoDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( (int)journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
    if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes[journalMode])) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to change the '%1' setting for the DAT-info database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
}

void DatInfoDatabaseManager::recreateSoftwareInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_softwareInfoTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_softwareInfoTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (list TEXT, id TEXT, infotext TEXT, PRIMARY KEY (list, id))").arg(m_softwareInfoTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (list, id)").arg(m_softwareInfoTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("software-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
}

void DatInfoDatabaseManager::recreateEmuInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_emuInfoTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_emuInfoTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT, infotext TEXT, PRIMARY KEY (id))").arg(m_emuInfoTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
    if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_emuInfoTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("emu-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
}

void DatInfoDatabaseManager::recreateGameInfoTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_gameInfoTableName)) ) {
        QMC2_ARCADE_LOG_STR(tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_gameInfoTableName)) ) {
        QMC2_ARCADE_LOG_STR(tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT, infotext TEXT, emulator TEXT, PRIMARY KEY (id))").arg(m_gameInfoTableName)) ) {
        QMC2_ARCADE_LOG_STR(tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
    if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_gameInfoTableName)) ) {
        QMC2_ARCADE_LOG_STR(tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("machine-info")).arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
}

void DatInfoDatabaseManager::recreateMetaDataTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_metaDataTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to remove %1 table: query = '%2', error = '%3'").arg(tr("meta-data")).arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (row INTEGER PRIMARY KEY, qmc2_version TEXT, datinfo_version INTEGER)").arg(m_metaDataTableName)) ) {
		QMC2_ARCADE_LOG_STR(tr("WARNING: failed to create %1 table: query = '%2', error = '%3'").arg(tr("meta-data")).arg(query.lastQuery()).arg(m_db.lastError().text()));
	}
}

void DatInfoDatabaseManager::recreateDatabase()
{
	recreateMetaDataTable();
	recreateSoftwareInfoTable();
	recreateEmuInfoTable();
	recreateGameInfoTable();
	setQmc2Version(XSTR(QMC2_ARCADE_MAIN_UI_VERSION));
	setDatInfoVersion(QMC2_ARCADE_DATINFO_VERSION);
}

bool DatInfoDatabaseManager::softwareInfoImportRequired(QStringList pathList)
{
	QStringList importFiles = globalConfig->softwareInfoImportFiles();
	QStringList importDates = globalConfig->softwareInfoImportDates();

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
            if ( dtImport != fi.lastModified().toTime_t() )
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
			QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("importing %1 from '%2'").arg(tr("software info-texts")).arg(QDir::toNativeSeparators(path))));
			qApp->processEvents();
			beginTransaction();
			QTextStream ts(&swInfoDB);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegExp markRegExp("^\\$\\S+\\=\\S+\\,$");
			QRegExp reduceLinesRegExp("(<br>){2,}");
			while ( !ts.atEnd() ) {
				QString singleLineSimplified = ts.readLine().simplified();
				bool containsMark = singleLineSimplified.contains(markRegExp);
				while ( !containsMark && !ts.atEnd() ) {
					singleLineSimplified = ts.readLine().simplified();
					if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
						qApp->processEvents();
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
							if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
								qApp->processEvents();
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
								if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
									qApp->processEvents();
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
								if ( pendingUpdates > QMC2_ARCADE_DATINFO_COMMIT ) {
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
			QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("done (importing %1 from '%2')").arg(tr("software info-texts")).arg(QDir::toNativeSeparators(path))));
			importPaths << path;
			importDates << QString::number(QFileInfo(path).lastModified().toTime_t());
			swInfoDB.close();
		} else {
			QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: can't open software info file '%1'").arg(QDir::toNativeSeparators(path))));
		}
	}

	QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("%n software info record(s) imported", "", softwareInfoRowCount())));

	if ( !importPaths.isEmpty() ) {
		globalConfig->setSoftwareInfoImportFiles(importPaths);
		globalConfig->setSoftwareInfoImportDates(importDates);
	} else {
		globalConfig->removeSoftwareInfoImportFiles();
		globalConfig->removeSoftwareInfoImportDates();
	}

	setQmc2Version(XSTR(QMC2_ARCADE_MAIN_UI_VERSION));
	setDatInfoVersion(QMC2_ARCADE_DATINFO_VERSION);
}

bool DatInfoDatabaseManager::emuInfoImportRequired(QStringList pathList)
{
	QStringList importFiles = globalConfig->emuInfoImportFiles();
	QStringList importDates = globalConfig->emuInfoImportDates();

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
            if ( dtImport != fi.lastModified().toTime_t() )
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
			QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("importing %1 from '%2'").arg(tr("emulator info-texts")).arg(QDir::toNativeSeparators(path))));
			qApp->processEvents();
			beginTransaction();
			QTextStream ts(&emuInfoDB);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegExp lineBreakRx("(<br>){2,}");
			while ( !ts.atEnd() ) {
				QString singleLineSimplified = ts.readLine().simplified();
				bool startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				while ( !startsWithDollarInfo && !ts.atEnd() ) {
					singleLineSimplified = ts.readLine().simplified();
					if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
						qApp->processEvents();
					startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				}
				if ( startsWithDollarInfo ) {
					QStringList gameNames = singleLineSimplified.mid(6).split(",", QString::SkipEmptyParts);
					bool startsWithDollarMame = false;
					while ( !startsWithDollarMame && !ts.atEnd() ) {
						singleLineSimplified = ts.readLine().simplified();
						if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
							qApp->processEvents();
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
							if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
								qApp->processEvents();
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
							if ( pendingUpdates > QMC2_ARCADE_DATINFO_COMMIT ) {
								commitTransaction();
								pendingUpdates = 0;
								beginTransaction();
							}
						} else {
							QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: missing '$end' in emulator info file %1").arg(QDir::toNativeSeparators(path))));
						}
					} else if ( !ts.atEnd() ) {
						QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: missing '$mame' in emulator info file %1").arg(QDir::toNativeSeparators(path))));
					} else if ( !ts.atEnd() ) {
						QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: missing '$info' in emulator info file %1").arg(QDir::toNativeSeparators(path))));
					}
				}
			}
			commitTransaction();
			QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("done (importing %1 from '%2')").arg(tr("emulator info-texts")).arg(QDir::toNativeSeparators(path))));
			importPaths << path;
			importDates << QString::number(QFileInfo(path).lastModified().toTime_t());
			emuInfoDB.close();

		} else {
			QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: can't open emulator info file %1").arg(QDir::toNativeSeparators(path))));
		}
	}

	QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("%n emulator info record(s) imported", "", emuInfoRowCount())));

	if ( !importPaths.isEmpty() ) {
		globalConfig->setEmuInfoImportFiles(importPaths);
		globalConfig->setEmuInfoImportDates(importDates);
	} else {
		globalConfig->removeEmuInfoImportFiles();
		globalConfig->removeEmuInfoImportDates();
	}

	setQmc2Version(XSTR(QMC2_ARCADE_MAIN_UI_VERSION));
	setDatInfoVersion(QMC2_ARCADE_DATINFO_VERSION);
}

bool DatInfoDatabaseManager::gameInfoImportRequired(QStringList pathList)
{
	QStringList importFiles = globalConfig->gameInfoImportFiles();
	QStringList importDates = globalConfig->gameInfoImportDates();

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
            if ( dtImport != fi.lastModified().toTime_t() )
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
			QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("importing %1 from '%2'").arg(tr("system info-texts")).arg(QDir::toNativeSeparators(path))));
			qApp->processEvents();
			beginTransaction();
			QTextStream ts(&gameInfoDB);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			quint64 recordsProcessed = 0, pendingUpdates = 0;
			QRegExp lineBreakRx("(<br>){2,}");
			while ( !ts.atEnd() ) {
				QString singleLineSimplified = ts.readLine().simplified();
				bool startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				while ( !startsWithDollarInfo && !ts.atEnd() ) {
					singleLineSimplified = ts.readLine().simplified();
					if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
						qApp->processEvents();
					startsWithDollarInfo = singleLineSimplified.startsWith("$info=");
				}
				if ( startsWithDollarInfo ) {
					QStringList gameNames = singleLineSimplified.mid(6).split(",", QString::SkipEmptyParts);
					bool startsWithDollarBio = false;
					while ( !startsWithDollarBio && !ts.atEnd() ) {
						singleLineSimplified = ts.readLine().simplified();
						if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
							qApp->processEvents();
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
							if ( recordsProcessed++ % QMC2_ARCADE_LOAD_RESPONSE == 0 )
								qApp->processEvents();
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
							if ( pendingUpdates > QMC2_ARCADE_DATINFO_COMMIT ) {
								commitTransaction();
								pendingUpdates = 0;
								beginTransaction();
							}
						} else {
                            QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: missing '$end' in machine info file %1").arg(QDir::toNativeSeparators(path))));
						}
					} else {
                        QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: missing '$bio' in machine info file %1").arg(QDir::toNativeSeparators(path))));
					}
				} else if ( !ts.atEnd() ) {
                    QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: missing '$info' in machine info file %1").arg(QDir::toNativeSeparators(path))));
				}
			}
			commitTransaction();
			QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("done (importing %1 from '%2')").arg(tr("system info-texts")).arg(QDir::toNativeSeparators(path))));
			importPaths << path;
			importDates << QString::number(QFileInfo(path).lastModified().toTime_t());
			gameInfoDB.close();
		} else {
            QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("WARNING: can't open machine info file %1").arg(QDir::toNativeSeparators(path))));
		}
	}

    QMC2_ARCADE_LOG_STR(QString(tr("DAT-info database") + ": " + tr("%n machine info record(s) imported", "", gameInfoRowCount())));

	if ( !importPaths.isEmpty() ) {
		globalConfig->setGameInfoImportFiles(importPaths);
		globalConfig->setGameInfoImportDates(importDates);
	} else {
		globalConfig->removeGameInfoImportFiles();
		globalConfig->removeGameInfoImportDates();
	}

	setQmc2Version(XSTR(QMC2_ARCADE_MAIN_UI_VERSION));
	setDatInfoVersion(QMC2_ARCADE_DATINFO_VERSION);
}
