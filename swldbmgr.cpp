#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QUuid>

#include "macros.h"
#include "qmc2main.h"
#include "machinelist.h"
#include "settings.h"
#include "swldbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern MachineList *qmc2MachineList;

SoftwareListXmlDatabaseManager::SoftwareListXmlDatabaseManager(QObject *parent)
	: QObject(parent)
{
	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	m_listIterationQuery = 0;
	m_connectionName = QString("swl-cache-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	m_db.setDatabaseName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareListCacheDatabase", QString(userScopePath + "/%1-swl-cache.db").arg(QMC2_EMU_NAME.toLower())).toString());
	m_tableBasename = QString("%1_swl_cache").arg(QMC2_EMU_NAME.toLower());
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() != 2 || !tables.contains(m_tableBasename) || !tables.contains(QString("%1_metadata").arg(m_tableBasename)) )
			recreateDatabase();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open software-list XML cache database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
}

SoftwareListXmlDatabaseManager::~SoftwareListXmlDatabaseManager()
{
	if ( m_db.isOpen() )
		m_db.close();
}

QString SoftwareListXmlDatabaseManager::emulatorVersion()
{
	QString emu_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			emu_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return emu_version;
}

void SoftwareListXmlDatabaseManager::setEmulatorVersion(QString emu_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (emu_version, row) VALUES (:emu_version, 0)").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to software-list XML cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET emu_version=:emu_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in software-list XML cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

QString SoftwareListXmlDatabaseManager::qmc2Version()
{
	QString qmc2_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			qmc2_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return qmc2_version;
}

void SoftwareListXmlDatabaseManager::setQmc2Version(QString qmc2_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (qmc2_version, row) VALUES (:qmc2_version, 0)").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to software-list XML cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET qmc2_version=:qmc2_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in software-list XML cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

int SoftwareListXmlDatabaseManager::swlCacheVersion()
{
	int swlcache_version = -1;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT swlcache_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			swlcache_version = query.value(0).toInt();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("swlcache_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return swlcache_version;
}

void SoftwareListXmlDatabaseManager::setSwlCacheVersion(int swlcache_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT swlcache_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (swlcache_version, row) VALUES (:swlcache_version, 0)").arg(m_tableBasename));
			query.bindValue(":swlcache_version", swlcache_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to software-list XML cache database: query = '%2', error = '%3'").arg("swlcache_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET swlcache_version=:swlcache_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":swlcache_version", swlcache_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in software-list XML cache database: query = '%2', error = '%3'").arg("swlcache_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("swlcache_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

QString SoftwareListXmlDatabaseManager::dtd()
{
	QString dtd;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT dtd FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			dtd = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("dtd").arg(query.lastQuery()).arg(query.lastError().text()));
	return dtd;
}

void SoftwareListXmlDatabaseManager::setDtd(QString dtd)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT dtd FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (dtd, row) VALUES (:dtd, 0)").arg(m_tableBasename));
			query.bindValue(":dtd", dtd);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to software-list XML cache database: query = '%2', error = '%3'").arg("dtd").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET dtd=:dtd WHERE row=0").arg(m_tableBasename));
			query.bindValue(":dtd", dtd);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in software-list XML cache database: query = '%2', error = '%3'").arg("dtd").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("dtd").arg(query.lastQuery()).arg(query.lastError().text()));
}

QStringList SoftwareListXmlDatabaseManager::uniqueSoftwareLists()
{
	static QStringList uniqueListNames;
	uniqueListNames.clear();
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT DISTINCT list FROM %1").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() ) {
			uniqueListNames << query.value(0).toString();
			while ( query.next() )
				uniqueListNames << query.value(0).toString();
		}
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("list").arg(query.lastQuery()).arg(query.lastError().text()));
	return uniqueListNames;
}

QStringList SoftwareListXmlDatabaseManager::uniqueSoftwareSets(QString list)
{
	static QStringList uniqueSets;
	uniqueSets.clear();
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT DISTINCT id FROM %1 WHERE list=:list").arg(m_tableBasename));
	query.bindValue(":list", list);
	if ( query.exec() ) {
		if ( query.first() ) {
			uniqueSets << query.value(0).toString();
			while ( query.next() )
				uniqueSets << query.value(0).toString();
		}
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(query.lastError().text()));
	return uniqueSets;
}

QString SoftwareListXmlDatabaseManager::xml(QString list, QString id)
{
	QString xml;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT xml FROM %1 WHERE list=:list AND id=:id").arg(m_tableBasename));
	query.bindValue(":list", list);
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			xml = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(query.lastError().text()));
	return xml;
}

QString SoftwareListXmlDatabaseManager::xml(QString setKey)
{
	QStringList setKeyTokens = setKey.split(":", QString::SkipEmptyParts);
	if ( setKeyTokens.count() < 2 )
		return QString();
	return xml(setKeyTokens[0], setKeyTokens[1]);
}

QString SoftwareListXmlDatabaseManager::xml(int rowid)
{
	QString xml;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT xml FROM %1 WHERE rowid=:rowid").arg(m_tableBasename));
	query.bindValue(":rowid", rowid);
	if ( query.exec() ) {
		if ( query.first() )
			xml = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(query.lastError().text()));
	return xml;
}

QString SoftwareListXmlDatabaseManager::list(int rowid)
{
	QString list;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT list FROM %1 WHERE rowid=:rowid").arg(m_tableBasename));
	query.bindValue(":rowid", rowid);
	if ( query.exec() ) {
		if ( query.first() )
			list = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("list").arg(query.lastQuery()).arg(query.lastError().text()));
	return list;
}

QString SoftwareListXmlDatabaseManager::nextXml(QString list, QString *id, bool start)
{
	if ( start || m_listIterationQuery == 0 ) {
		if ( m_listIterationQuery )
			delete m_listIterationQuery;
		m_listIterationQuery = new QSqlQuery(m_db);
		m_listIterationQuery->prepare("SELECT id, xml FROM %1 WHERE list=:list");
		m_listIterationQuery->bindValue(":list", list);
		if ( m_listIterationQuery->exec() ) {
			if ( m_listIterationQuery->first() ) {
				if ( id )
					*id = m_listIterationQuery->value(0).toString();
				return m_listIterationQuery->value(1).toString();
			}
		}
	} else {
		if ( m_listIterationQuery->next() ) {
			if ( id )
				*id = m_listIterationQuery->value(0).toString();
			return m_listIterationQuery->value(1).toString();
		} else {
			delete m_listIterationQuery;
			m_listIterationQuery = 0;
			if ( id )
				id->clear();
			return QString();
		}
	}
	return QString();
}

QString SoftwareListXmlDatabaseManager::allXml(QString list)
{
	static QString softwareListBuffer;
	softwareListBuffer.clear();

	QSqlQuery query(m_db);
	query.prepare(QString("SELECT xml FROM %1 WHERE list=:list").arg(m_tableBasename));
	query.bindValue(":list", list);
	if ( query.exec() ) {
		if ( query.first() ) {
			softwareListBuffer = QString("<softwarelist name=\"%1\">\n").arg(list);
			softwareListBuffer += query.value(0).toString();
			while ( query.next() )
				softwareListBuffer += query.value(0).toString();
			softwareListBuffer += "</softwarelist>";
		}
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(query.lastError().text()));

	return softwareListBuffer;
}

void SoftwareListXmlDatabaseManager::setXml(QString list, QString id, QString xml)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT xml FROM %1 WHERE list=:list AND id=:id").arg(m_tableBasename));
	query.bindValue(":list", list);
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (list, id, xml) VALUES (:list, :id, :xml)").arg(m_tableBasename));
			query.bindValue(":list", list);
			query.bindValue(":id", id);
			query.bindValue(":xml", xml);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to software-list XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET xml=:xml WHERE list=:list AND id=:id").arg(m_tableBasename));
			query.bindValue(":list", list);
			query.bindValue(":id", id);
			query.bindValue(":xml", xml);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in software-list XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(query.lastError().text()));
}

qint64 SoftwareListXmlDatabaseManager::swlRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_tableBasename)) ) {
		if ( query.first() )
			return query.value(0).toLongLong();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from software-list XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return -1;
	}
}

qint64 SoftwareListXmlDatabaseManager::nextRowId(bool refreshRowIds)
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
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row IDs from software-list XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
	} else if ( m_lastRowId > -1 ) {
		m_lastRowId++;
		if ( m_lastRowId < m_rowIdList.count() )
			return m_rowIdList[m_lastRowId];
	}
	return -1;
}

QString SoftwareListXmlDatabaseManager::idAtIndex(int index)
{
	if ( index < 0 ) {
		m_idAtIndexCache.clear();
		QSqlQuery query(m_db);
		if ( query.exec(QString("SELECT list, id FROM %1 ORDER BY rowid").arg(m_tableBasename)) ) {
			if ( query.first() ) {
				do {
					m_idAtIndexCache << query.value(0).toString() + ":" + query.value(1).toString();
				} while ( query.next() );
				return m_idAtIndexCache[0];
			} else
				return QString();
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("list, id").arg(query.lastQuery()).arg(query.lastError().text()));
			return QString();
		}
	} else {
		if ( index < m_idAtIndexCache.count() )
			return m_idAtIndexCache[index];
		else
			return QString();
	}
}

bool SoftwareListXmlDatabaseManager::exists(QString list, QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT list, id FROM %1 WHERE list=:list AND id=:id LIMIT 1").arg(m_tableBasename));
	query.bindValue(":list", list);
	query.bindValue(":id", id);
	if ( query.exec() )
		return query.first();
	else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from software-list XML cache database: query = '%2', error = '%3'").arg("list, id").arg(query.lastQuery()).arg(query.lastError().text()));
		return false;
	}
}

quint64 SoftwareListXmlDatabaseManager::databaseSize()
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

void SoftwareListXmlDatabaseManager::setCacheSize(quint64 kiloBytes)
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA cache_size = -%1").arg(kiloBytes)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the software-list XML cache database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(query.lastError().text()));
}

void SoftwareListXmlDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( (int)syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes[syncMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the software-list XML cache database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(query.lastError().text()));
}

void SoftwareListXmlDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( (int)journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes[journalMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the software-list XML cache database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(query.lastError().text()));
}

void SoftwareListXmlDatabaseManager::recreateDatabase(bool quiet)
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove software-list XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove software-list XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1_metadata").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove software-list XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (list TEXT, id TEXT, xml TEXT, PRIMARY KEY (list, id))").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create software-list XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (list, id)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create software-list XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1_metadata (row INTEGER PRIMARY KEY, dtd TEXT, emu_version TEXT, qmc2_version TEXT, swlcache_version INTEGER)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create software-list XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	setQmc2Version(XSTR(QMC2_VERSION));
	setSwlCacheVersion(QMC2_SWLCACHE_VERSION);
	setEmulatorVersion(qmc2MachineList->emulatorVersion);
	if ( !quiet )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("software-list XML cache database '%1' initialized").arg(m_db.databaseName()));
}
