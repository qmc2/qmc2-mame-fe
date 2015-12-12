#include <QApplication>
#include <QDateTime>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDir>
#include <QUuid>
#include <QRegExp>

#include "macros.h"
#include "settings.h"
#include "options.h"
#include "checksumdbmgr.h"

// external global variables
extern Settings *qmc2Config;

CheckSumDatabaseManager::CheckSumDatabaseManager(QObject *parent, QString settingsKey)
	: QObject(parent)
{
	m_settingsKey = settingsKey;
	m_fileTypes << "ZIP" << "7Z" << "CHD" << "FILE";
	QString userScopePath = Options::configPath();
	m_connectionName = QString("checksum-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	QString variantName = QMC2_VARIANT_NAME.toLower().replace(QRegExp("\\..*$"), "");
	if ( m_settingsKey == "ROMAlyzer" )
		m_db.setDatabaseName(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDatabasePath", QString(userScopePath + "/%1-checksum.db").arg(variantName)).toString());
	else
		m_db.setDatabaseName(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDatabasePath", QString(userScopePath + "/%1-software-checksum.db").arg(variantName)).toString());
	m_tableBasename = QString("%1_checksum").arg(variantName.replace("-", "_"));
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() != 2 || !tables.contains(m_tableBasename) || !tables.contains(QString("%1_metadata").arg(m_tableBasename)) || checkSumDbVersion() != QMC2_CHECKSUM_DB_VERSION )
			recreateDatabase();
	} else
		emitlog(tr("WARNING: failed to open check-sum database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
	m_lastRowId = -1;
}

CheckSumDatabaseManager::~CheckSumDatabaseManager()
{
	if ( m_db.isOpen() )
		m_db.close();
}

QString CheckSumDatabaseManager::qmc2Version()
{
	QString qmc2_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			qmc2_version = query.value(0).toString();
		query.finish();
	} else
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return qmc2_version;
}

void CheckSumDatabaseManager::setQmc2Version(QString qmc2_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (qmc2_version, row) VALUES (:qmc2_version, 0)").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				emitlog(tr("WARNING: failed to add '%1' to check-sum database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET qmc2_version=:qmc2_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				emitlog(tr("WARNING: failed to update '%1' in check-sum database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

int CheckSumDatabaseManager::checkSumDbVersion()
{
	int checksum_db_version = -1;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT checksum_db_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			checksum_db_version = query.value(0).toInt();
		query.finish();
	} else
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("checksum_db_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return checksum_db_version;
}

void CheckSumDatabaseManager::setCheckSumDbVersion(int checksum_db_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT checksum_db_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (checksum_db_version, row) VALUES (:checksum_db_version, 0)").arg(m_tableBasename));
			query.bindValue(":checksum_db_version", checksum_db_version);
			if ( !query.exec() )
				emitlog(tr("WARNING: failed to add '%1' to check-sum database: query = '%2', error = '%3'").arg("checksum_db_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET checksum_db_version=:checksum_db_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":checksum_db_version", checksum_db_version);
			if ( !query.exec() )
				emitlog(tr("WARNING: failed to update '%1' in check-sum database: query = '%2', error = '%3'").arg("checksum_db_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("checksum_db_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

uint CheckSumDatabaseManager::scanTime()
{
	uint scan_time = 0;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT scan_time FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			scan_time = query.value(0).toUInt();
		query.finish();
	} else
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("scan_time").arg(query.lastQuery()).arg(query.lastError().text()));
	return scan_time;
}

void CheckSumDatabaseManager::setScanTime(uint scan_time)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT scan_time FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (scan_time, row) VALUES (:scan_time, 0)").arg(m_tableBasename));
			query.bindValue(":scan_time", scan_time);
			if ( !query.exec() )
				emitlog(tr("WARNING: failed to add '%1' to check-sum database: query = '%2', error = '%3'").arg("scan_time").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET scan_time=:scan_time WHERE row=0").arg(m_tableBasename));
			query.bindValue(":scan_time", scan_time);
			if ( !query.exec() )
				emitlog(tr("WARNING: failed to update '%1' in check-sum database: query = '%2', error = '%3'").arg("scan_time").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("scan_time").arg(query.lastQuery()).arg(query.lastError().text()));
}

qint64 CheckSumDatabaseManager::checkSumRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_tableBasename)) ) {
		if ( query.first() )
			return query.value(0).toLongLong();
		else
			return -1;
	} else {
		emitlog(tr("WARNING: failed to fetch row count from check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
	}	return -1;
}

qint64 CheckSumDatabaseManager::nextRowId(bool refreshRowIds)
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
				return m_rowIdList[0];
			}
		} else {
			emitlog(tr("WARNING: failed to fetch row IDs from check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
			return -1;
		}
	} else if ( m_lastRowId > -1 ) {
		m_lastRowId++;
		if ( m_lastRowId < m_rowIdList.count() )
			return m_rowIdList[m_lastRowId];
		else
			return -1;
	}
	return -1;
}

quint64 CheckSumDatabaseManager::databaseSize()
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

void CheckSumDatabaseManager::setCacheSize(quint64 kiloBytes)
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA cache_size = -%1").arg(kiloBytes)) )
		emitlog(tr("WARNING: failed to change the '%1' setting for the check-sum database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(query.lastError().text()));
}

void CheckSumDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( (int)syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes[syncMode])) )
		emitlog(tr("WARNING: failed to change the '%1' setting for the check-sum database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(query.lastError().text()));
}

void CheckSumDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( (int)journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes[journalMode])) )
		emitlog(tr("WARNING: failed to change the '%1' setting for the check-sum database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(query.lastError().text()));
}

bool CheckSumDatabaseManager::exists(QString sha1, QString crc, quint64 size)
{
	QSqlQuery query(m_db);
	if ( size > 0 ) {
		if ( sha1.isEmpty() ) {
			query.prepare(QString("SELECT sha1, crc, size FROM %1 WHERE crc=:crc AND size=:size LIMIT 1").arg(m_tableBasename));
			query.bindValue(":crc", crc);
			query.bindValue(":size", size);
		} else if ( crc.isEmpty() ) {
			query.prepare(QString("SELECT sha1, crc, size FROM %1 WHERE sha1=:sha1 AND size=:size LIMIT 1").arg(m_tableBasename));
			query.bindValue(":sha1", sha1);
			query.bindValue(":size", size);
		} else {
			query.prepare(QString("SELECT sha1, crc, size FROM %1 WHERE sha1=:sha1 AND crc=:crc AND size=:size LIMIT 1").arg(m_tableBasename));
			query.bindValue(":sha1", sha1);
			query.bindValue(":crc", crc);
			query.bindValue(":size", size);
		}
	} else {
		if ( sha1.isEmpty() ) {
			query.prepare(QString("SELECT sha1, crc FROM %1 WHERE crc=:crc LIMIT 1").arg(m_tableBasename));
			query.bindValue(":crc", crc);
		} else if ( crc.isEmpty() ) {
			query.prepare(QString("SELECT sha1, crc FROM %1 WHERE sha1=:sha1 LIMIT 1").arg(m_tableBasename));
			query.bindValue(":sha1", sha1);
		} else {
			query.prepare(QString("SELECT sha1, crc FROM %1 WHERE sha1=:sha1 AND crc=:crc LIMIT 1").arg(m_tableBasename));
			query.bindValue(":sha1", sha1);
			query.bindValue(":crc", crc);
		}
	}
	if ( query.exec() )
		return query.first();
	else {
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("sha1, crc").arg(query.lastQuery()).arg(query.lastError().text()));
		return false;
	}
}

void CheckSumDatabaseManager::setData(QString sha1, QString crc, quint64 size, QString path, QString member, QString type)
{
	QSqlQuery query(m_db);
	query.prepare(QString("INSERT INTO %1 (sha1, crc, size, path, member, type) VALUES (:sha1, :crc, :size, :path, :member, :type)").arg(m_tableBasename));
	query.bindValue(":sha1", sha1);
	query.bindValue(":crc", crc);
	query.bindValue(":size", size);
	query.bindValue(":path", path);
	query.bindValue(":member", member);
	query.bindValue(":type", type);
	if ( !query.exec() )
		emitlog(tr("WARNING: failed to add '%1' to check-sum database: query = '%2', error = '%3'").arg("sha1, crc, size, path, member, type").arg(query.lastQuery()).arg(query.lastError().text()));
}

bool CheckSumDatabaseManager::getData(QString sha1, QString crc, quint64 *size, QString *path, QString *member, QString *type)
{
	QSqlQuery query(m_db);
	if ( *size > 0 ) {
		if ( sha1.isEmpty() ) {
			query.prepare(QString("SELECT size, path, member, type FROM %1 WHERE crc=:crc AND size=:size").arg(m_tableBasename));
			query.bindValue(":crc", crc);
			query.bindValue(":size", *size);
		} else if ( crc.isEmpty() ) {
			query.prepare(QString("SELECT size, path, member, type FROM %1 WHERE sha1=:sha1 AND size=:size").arg(m_tableBasename));
			query.bindValue(":sha1", sha1);
			query.bindValue(":size", *size);
		} else {
			query.prepare(QString("SELECT size, path, member, type FROM %1 WHERE sha1=:sha1 AND crc=:crc AND size=:size").arg(m_tableBasename));
			query.bindValue(":sha1", sha1);
			query.bindValue(":crc", crc);
			query.bindValue(":size", *size);
		}
	} else {
		if ( sha1.isEmpty() ) {
			query.prepare(QString("SELECT size, path, member, type FROM %1 WHERE crc=:crc").arg(m_tableBasename));
			query.bindValue(":crc", crc);
		} else if ( crc.isEmpty() ) {
			query.prepare(QString("SELECT size, path, member, type FROM %1 WHERE sha1=:sha1").arg(m_tableBasename));
			query.bindValue(":sha1", sha1);
		} else {
			query.prepare(QString("SELECT size, path, member, type FROM %1 WHERE sha1=:sha1 AND crc=:crc").arg(m_tableBasename));
			query.bindValue(":sha1", sha1);
			query.bindValue(":crc", crc);
		}
	}
	if ( query.exec() ) {
		if ( query.first() ) {
			*size = query.value(0).toULongLong();
			*path = query.value(1).toString();
			*member = query.value(2).toString();
			*type = query.value(3).toString();
			return true;
		} else
			return false;
	} else {
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("path, member, type").arg(query.lastQuery()).arg(query.lastError().text()));
		return false;
	}
}

QString CheckSumDatabaseManager::getCrc(QString sha1)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT crc FROM %1 WHERE sha1=:sha1").arg(m_tableBasename));
	query.bindValue(":sha1", sha1);
	if ( query.exec() ) {
		if ( query.first() )
			return query.value(0).toString();
		else
			return QString();
	} else {
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("crc").arg(query.lastQuery()).arg(query.lastError().text()));
		return QString();
	}
}

QString CheckSumDatabaseManager::getSha1(QString crc)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT sha1 FROM %1 WHERE crc=:crc").arg(m_tableBasename));
	query.bindValue(":crc", crc);
	if ( query.exec() ) {
		if ( query.first() )
			return query.value(0).toString();
		else
			return QString();
	} else {
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("sha1").arg(query.lastQuery()).arg(query.lastError().text()));
		return QString();
	}
}

void CheckSumDatabaseManager::pathRemove(QString path)
{
	QSqlQuery query(m_db);
	query.prepare(QString("DELETE FROM %1 WHERE path=:path").arg(m_tableBasename));
	query.bindValue(":path", path);
	if ( !query.exec() )
		emitlog(tr("WARNING: failed to remove path '%1' from check-sum database: query = '%2', error = '%3'").arg(path).arg(query.lastQuery()).arg(query.lastError().text()));
}

QString CheckSumDatabaseManager::pathOfRow(qint64 row, QString *key, bool simpleKey)
{
	QSqlQuery query(m_db);
	if ( key ) {
		query.prepare(QString("SELECT path, sha1, crc, size, type FROM %1 WHERE rowid=:row").arg(m_tableBasename));
		query.bindValue(":row", row);
		if ( query.exec() ) {
			if ( query.first() ) {
				if ( !simpleKey )
					*key = QString("%1-%2-%3").arg(query.value(1).toString()).arg(query.value(2).toString()).arg(query.value(3).toString());
				else if ( query.value(4).toString() == "CHD" )
					*key = QString("%1-0-0").arg(query.value(1).toString());
				else
					*key = QString("%1-%2-%3").arg(query.value(1).toString()).arg(query.value(2).toString()).arg(query.value(3).toString());
				return query.value(0).toString();
			} else
				return QString();
		} else {
			emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("path, sha1, crc, size").arg(query.lastQuery()).arg(query.lastError().text()));
			return QString();
		}
	} else {
		query.prepare(QString("SELECT path FROM %1 WHERE rowid=:row").arg(m_tableBasename));
		query.bindValue(":row", row);
		if ( query.exec() ) {
			if ( query.first() )
				return query.value(0).toString();
			else
				return QString();
		} else {
			emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("path").arg(query.lastQuery()).arg(query.lastError().text()));
			return QString();
		}
	}
}

QString CheckSumDatabaseManager::keyOfRow(qint64 row)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT sha1, crc, size FROM %1 WHERE rowid=:row").arg(m_tableBasename));
	query.bindValue(":row", row);
	if ( query.exec() ) {
		if ( query.first() )
			return QString("%1-%2-%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
		else
			return QString();
	} else {
		emitlog(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("sha1, crc, size").arg(query.lastQuery()).arg(query.lastError().text()));
		return QString();
	}
}

void CheckSumDatabaseManager::invalidateRow(qint64 row)
{
	QSqlQuery query(m_db);
	query.prepare(QString("UPDATE %1 SET sha1='I' WHERE rowid=:row").arg(m_tableBasename));
	query.bindValue(":row", row);
	if ( !query.exec() )
		emitlog(tr("WARNING: failed to update '%1' in check-sum database: query = '%2', error = '%3'").arg("sha1").arg(query.lastQuery()).arg(query.lastError().text()));

}

void CheckSumDatabaseManager::removeInvalidatedRows()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DELETE FROM %1 WHERE sha1='I'").arg(m_tableBasename)) )
		emitlog(tr("WARNING: failed to remove invalidated rows from check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
}

int CheckSumDatabaseManager::nameToType(QString name)
{
	return m_fileTypes.indexOf(name);
}

QString CheckSumDatabaseManager::typeToName(int type)
{
	if ( type < m_fileTypes.count() )
		return m_fileTypes[type];
	else
		return QString("UNKNOWN");
}

void CheckSumDatabaseManager::vacuum()
{
	QSqlQuery query(m_db);
	query.exec("VACUUM");
	query.finish();
}

void CheckSumDatabaseManager::recreateDatabase()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_tableBasename)) ) {
		emitlog(tr("WARNING: failed to remove check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_tableBasename)) ) {
		emitlog(tr("WARNING: failed to remove check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1_metadata").arg(m_tableBasename)) ) {
		emitlog(tr("WARNING: failed to remove check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (sha1 TEXT, crc TEXT, size UNSIGNED BIG INT, path TEXT, member TEXT, type TEXT, PRIMARY KEY (sha1, crc), CONSTRAINT %1_unique_checksums UNIQUE (sha1, crc))").arg(m_tableBasename)) ) {
		emitlog(tr("WARNING: failed to create check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (sha1, crc)").arg(m_tableBasename)) ) {
		emitlog(tr("WARNING: failed to create check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1_metadata (row INTEGER PRIMARY KEY, scan_time UNSIGNED BIG INT, qmc2_version TEXT, checksum_db_version INTEGER)").arg(m_tableBasename)) ) {
		emitlog(tr("WARNING: failed to create check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	setScanTime(QDateTime::currentDateTime().toTime_t());
	setQmc2Version(XSTR(QMC2_VERSION));
	setCheckSumDbVersion(QMC2_CHECKSUM_DB_VERSION);
	emitlog(tr("check-sum database '%1' initialized").arg(databasePath()));
}

void CheckSumDatabaseManager::emitlog(QString message)
{
	emit log(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
}
