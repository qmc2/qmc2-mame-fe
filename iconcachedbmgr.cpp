#include <QApplication>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QUuid>

#include "macros.h"
#include "qmc2main.h"
#include "settings.h"
#include "options.h"
#include "iconcachedbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

IconCacheDatabaseManager::IconCacheDatabaseManager(QObject *parent) :
	QObject(parent),
	m_logActive(false),
	m_resetRowCount(true),
	m_lastRowCount(-1),
	m_query(0)
{
	m_connectionName = QString("icon-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	m_db.setDatabaseName(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/IconCacheDatabase", QString(Options::configPath() + "/%1-icon-cache.db").arg(QMC2_EMU_NAME.toLower())).toString());
	m_tableBasename = QString("%1_icon_cache").arg(QMC2_EMU_NAME.toLower());
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() < 2 || !tables.contains(m_tableBasename) || !tables.contains(QString("%1_metadata").arg(m_tableBasename)) )
			recreateDatabase();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open icon cache database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
}

IconCacheDatabaseManager::~IconCacheDatabaseManager()
{
	if ( m_query )
		delete m_query;
	if ( m_db.isOpen() )
		m_db.close();
}

QString IconCacheDatabaseManager::emulatorVersion()
{
	QString emu_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			emu_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from icon cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return emu_version;
}

void IconCacheDatabaseManager::setEmulatorVersion(QString emu_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (emu_version, row) VALUES (:emu_version, 0)").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to icon cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET emu_version=:emu_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in icon cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from icon cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

QString IconCacheDatabaseManager::qmc2Version()
{
	QString qmc2_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			qmc2_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from icon cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return qmc2_version;
}

void IconCacheDatabaseManager::setQmc2Version(QString qmc2_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (qmc2_version, row) VALUES (:qmc2_version, 0)").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to icon cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET qmc2_version=:qmc2_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in icon cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from icon cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

int IconCacheDatabaseManager::iconCacheVersion()
{
	int icon_cache_version = -1;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT icon_cache_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			icon_cache_version = query.value(0).toInt();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from icon cache database: query = '%2', error = '%3'").arg("icon_cache_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return icon_cache_version;
}

void IconCacheDatabaseManager::setIconCacheVersion(int icon_cache_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT icon_cache_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (icon_cache_version, row) VALUES (:icon_cache_version, 0)").arg(m_tableBasename));
			query.bindValue(":icon_cache_version", icon_cache_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to icon cache database: query = '%2', error = '%3'").arg("icon_cache_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET icon_cache_version=:icon_cache_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":icon_cache_version", icon_cache_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in icon cache database: query = '%2', error = '%3'").arg("icon_cache_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from icon cache database: query = '%2', error = '%3'").arg("icon_cache_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

qint64 IconCacheDatabaseManager::iconCacheRowCount(bool reset)
{
	m_resetRowCount |= reset;
	if ( m_resetRowCount ) {
		QSqlQuery query(m_db);
		if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_tableBasename)) ) {
			if ( query.first() )
				m_lastRowCount = query.value(0).toLongLong();
			else
				m_lastRowCount = -1;
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from icon cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
			m_lastRowCount = -1;
		}
		m_resetRowCount = false;
	}
	return m_lastRowCount;
}

bool IconCacheDatabaseManager::isEmpty()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT * FROM %1 LIMIT 1").arg(m_tableBasename)) )
		return !query.first();
	else
		return true;
}

bool IconCacheDatabaseManager::importRequired(const QStringList &pathList)
{
	QStringList importPaths(qmc2Config->value(QMC2_EMULATOR_PREFIX + "IconCacheDatabase/ImportPaths", QStringList()).toStringList());
	QStringList importDates(qmc2Config->value(QMC2_EMULATOR_PREFIX + "IconCacheDatabase/ImportDates", QStringList()).toStringList());
	if ( importPaths.isEmpty() || importDates.isEmpty() )
		return true;
	if ( importPaths.count() != importDates.count() )
		return true;
	bool importPathsChanged = false;
	foreach (QString path, pathList) {
		if ( !importPaths.contains(path) )
			importPathsChanged = true;
		if ( importPathsChanged )
			break;
	}
	if ( !importPathsChanged ) {
		bool datesChanged = false;
		foreach (QString path, pathList) {
			QFileInfo fi(path);
			uint dtImport = importDates.at(importPaths.indexOf(path)).toUInt();
			if ( dtImport < fi.lastModified().toTime_t() )
				datesChanged = true;
			if ( datesChanged )
				break;
		}
		if ( datesChanged )
			return true;
		else
			return iconCacheRowCount() == 0;
	} else
		return true;
}

void IconCacheDatabaseManager::queryIconData()
{
	if ( !m_query )
		m_query = new QSqlQuery(m_db);
	m_query->clear();
	m_query->prepare(QString("SELECT id, icon_data FROM %1").arg(m_tableBasename));
	m_query->exec();
}

bool IconCacheDatabaseManager::nextIconData(QString *id, QByteArray *icon_data)
{
	if ( m_query->next() ) {
		*id = m_query->value(QMC2_ICDB_INDEX_ID).toString();
		*icon_data = m_query->value(QMC2_ICDB_INDEX_ICON_DATA).toByteArray();
		return true;
	} else
		return false;
}

void IconCacheDatabaseManager::setIconData(const QString &id, const QByteArray &icon_data)
{
	QSqlQuery query(m_db);
	query.prepare(QString("INSERT INTO %1 (id, icon_data) VALUES (:id, :icon_data)").arg(m_tableBasename));
	query.bindValue(":id", id);
	query.bindValue(":icon_data", icon_data);
	if ( !query.exec() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to icon cache database: query = '%2', error = '%3'").arg(id).arg(query.lastQuery()).arg(query.lastError().text()));
}

bool IconCacheDatabaseManager::exists(const QString &id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE id=:id LIMIT 1").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( query.exec() )
		return query.first();
	else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from icon cache database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(query.lastError().text()));
		return false;
	}
}

quint64 IconCacheDatabaseManager::databaseSize()
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

void IconCacheDatabaseManager::setCacheSize(quint64 kiloBytes)
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA cache_size = -%1").arg(kiloBytes)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the icon cache database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(query.lastError().text()));
}

void IconCacheDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( (int)syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes.at(syncMode))) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the icon cache database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(query.lastError().text()));
}

void IconCacheDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( (int)journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes.at(journalMode))) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the icon cache database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(query.lastError().text()));
}

void IconCacheDatabaseManager::recreateDatabase()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove icon cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove icon cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1_metadata").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove icon cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT PRIMARY KEY, icon_data BLOB, CONSTRAINT %1_unique_id UNIQUE (id))").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create icon cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create icon cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1_metadata (row INTEGER PRIMARY KEY, emu_version TEXT, qmc2_version TEXT, icon_cache_version INTEGER)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create icon cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	if ( logActive() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("icon cache database '%1' initialized").arg(m_db.databaseName()));
}
