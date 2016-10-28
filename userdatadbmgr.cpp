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
#include "userdatadbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern MachineList *qmc2MachineList;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2HierarchyItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2CategoryItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2VersionItemHash;

UserDataDatabaseManager::UserDataDatabaseManager(QObject *parent)
	: QObject(parent)
{
	setLogActive(true);
	m_connectionName = QString("user-data-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	m_db.setDatabaseName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/UserDataDatabase", QString(Options::configPath() + "/%1-user-data.db").arg(QMC2_EMU_NAME.toLower())).toString());
	m_tableBasename = QString("%1_user_data").arg(QMC2_EMU_NAME.toLower());
	m_oldTableBasenameSL = QString("%1_software_list_visibility").arg(QMC2_EMU_NAME.toLower());
	m_tableBasenameSL = QString("%1_softlist_user_data").arg(QMC2_EMU_NAME.toLower());
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() < 2 || !tables.contains(m_tableBasename) || !tables.contains(QString("%1_metadata").arg(m_tableBasename)) )
			recreateDatabase();
		if ( tables.contains(m_oldTableBasenameSL) )
			renameSoftListTable();
		if ( tables.count() < 3 || !tables.contains(m_tableBasenameSL))
			recreateSoftListVisibilityTable();
		QStringList columns = columnNames(m_tableBasenameSL);
		if ( columns.count() < 3 || !columns.contains("favorites") )
			addSoftListFavoritesColumn();
		if ( columns.count() < 4 || !columns.contains("device_configs") )
			addSoftListDeviceConfigsColumn();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open user data database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
	m_lastRowId = -1;
}

UserDataDatabaseManager::~UserDataDatabaseManager()
{
	if ( m_db.isOpen() )
		m_db.close();
}

QString UserDataDatabaseManager::emulatorVersion()
{
	QString emu_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			emu_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return emu_version;
}

void UserDataDatabaseManager::setEmulatorVersion(QString emu_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (emu_version, row) VALUES (:emu_version, 0)").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET emu_version=:emu_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

QString UserDataDatabaseManager::qmc2Version()
{
	QString qmc2_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			qmc2_version = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return qmc2_version;
}

void UserDataDatabaseManager::setQmc2Version(QString qmc2_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (qmc2_version, row) VALUES (:qmc2_version, 0)").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET qmc2_version=:qmc2_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

int UserDataDatabaseManager::userDataVersion()
{
	int userdata_version = -1;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT userdata_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			userdata_version = query.value(0).toInt();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("userdata_version").arg(query.lastQuery()).arg(query.lastError().text()));
	return userdata_version;
}

void UserDataDatabaseManager::setUserDataVersion(int userdata_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT userdata_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (userdata_version, row) VALUES (:userdata_version, 0)").arg(m_tableBasename));
			query.bindValue(":userdata_version", userdata_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("userdata_version").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET userdata_version=:userdata_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":userdata_version", userdata_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("userdata_version").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("userdata_version").arg(query.lastQuery()).arg(query.lastError().text()));
}

int UserDataDatabaseManager::rank(QString id)
{
	if ( m_rankCache.contains(id) )
		return m_rankCache[id];

	int rank = 0;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT rank FROM %1 WHERE id=:id").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			rank = query.value(0).toInt();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(query.lastError().text()));
	return rank;
}

int UserDataDatabaseManager::rank(int rowid)
{
	int rank = 0;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT rank FROM %1 WHERE rowid=:rowid").arg(m_tableBasename));
	query.bindValue(":rowid", rowid);
	if ( query.exec() ) {
		if ( query.first() )
			rank = query.value(0).toInt();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(query.lastError().text()));
	return rank;
}

void UserDataDatabaseManager::setRank(QString id, int rank)
{
	if ( rank == 0 && comment(id).isEmpty() ) {
		remove(id);
		return;
	}

	QSqlQuery query(m_db);
	query.prepare(QString("SELECT rank FROM %1 WHERE id=:id").arg(m_tableBasename));
	query.bindValue(":id", id);
	m_rankCache[id] = rank;
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, rank) VALUES (:id, :rank)").arg(m_tableBasename));
			query.bindValue(":id", id);
			query.bindValue(":rank", rank);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET rank=:rank WHERE id=:id").arg(m_tableBasename));
			query.bindValue(":id", id);
			query.bindValue(":rank", rank);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(query.lastError().text()));
}

QString UserDataDatabaseManager::comment(QString id)
{
	if ( m_commentCache.contains(id) )
		return m_commentCache[id];

	QString comment;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT comment FROM %1 WHERE id=:id").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			comment = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(query.lastError().text()));
	return comment;
}

QString UserDataDatabaseManager::comment(int rowid)
{
	QString comment;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT comment FROM %1 WHERE rowid=:rowid").arg(m_tableBasename));
	query.bindValue(":rowid", rowid);
	if ( query.exec() ) {
		if ( query.first() )
			comment = query.value(0).toString();
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(query.lastError().text()));
	return comment;
}

void UserDataDatabaseManager::setComment(QString id, QString comment)
{
	if ( comment.isEmpty() && rank(id) == 0 ) {
		remove(id);
		return;
	}

	QSqlQuery query(m_db);
	query.prepare(QString("SELECT comment FROM %1 WHERE id=:id").arg(m_tableBasename));
	query.bindValue(":id", id);
	m_commentCache[id] = comment;
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, comment) VALUES (:id, :comment)").arg(m_tableBasename));
			query.bindValue(":id", id);
			query.bindValue(":comment", comment);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET comment=:comment WHERE id=:id").arg(m_tableBasename));
			query.bindValue(":id", id);
			query.bindValue(":comment", comment);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(query.lastError().text()));
}

qint64 UserDataDatabaseManager::userDataRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_tableBasename)) ) {
		if ( query.first() )
			return query.value(0).toLongLong();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return -1;
	}
}

qint64 UserDataDatabaseManager::nextRowId(bool refreshRowIds)
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
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row IDs from user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
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

QString UserDataDatabaseManager::id(int rowid)
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(query.lastError().text()));
		return QString();
	}
}

bool UserDataDatabaseManager::exists(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE id=:id LIMIT 1").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( query.exec() )
		return query.first();
	else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(query.lastError().text()));
		return false;
	}
}

void UserDataDatabaseManager::cleanUp()
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("cleaning up user data database '%1'").arg(m_db.databaseName()));

	qint64 row = nextRowId(true);
	beginTransaction();
	while ( row > 0 ) {
		QString idOfCurrentRow = id(row);
		if ( !idOfCurrentRow.isEmpty() ) {
			if ( !qmc2MachineList->xmlDb()->exists(idOfCurrentRow) ) {
				QSqlQuery query(m_db);
				query.prepare(QString("DELETE FROM %1 WHERE rowid=:row").arg(m_tableBasename));
				query.bindValue(":row", row);
				if ( query.exec() ) {
					query.finish();
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("removed obsolete ID '%1'").arg(idOfCurrentRow));
				} else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(idOfCurrentRow).arg(query.lastQuery()).arg(query.lastError().text()));
			} else if ( rank(idOfCurrentRow) == 0 && comment(idOfCurrentRow).isEmpty() ) {
				QSqlQuery query(m_db);
				query.prepare(QString("DELETE FROM %1 WHERE rowid=:row").arg(m_tableBasename));
				query.bindValue(":row", row);
				if ( query.exec() ) {
					query.finish();
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("removed ID '%1' due to no data").arg(idOfCurrentRow));
				} else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(idOfCurrentRow).arg(query.lastQuery()).arg(query.lastError().text()));
			}
		}
		qApp->processEvents();
		row = nextRowId();
	}
	QSqlQuery query(m_db);
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	commitTransaction();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (cleaning up user data database '%1')").arg(m_db.databaseName()));
}

void UserDataDatabaseManager::remove(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("DELETE FROM %1 WHERE id=:id").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( !query.exec() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(id).arg(query.lastQuery()).arg(query.lastError().text()));
	m_rankCache.remove(id);
	m_commentCache.remove(id);
}

quint64 UserDataDatabaseManager::databaseSize()
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

void UserDataDatabaseManager::setCacheSize(quint64 kiloBytes)
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA cache_size = -%1").arg(kiloBytes)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the user data database: query = '%2', error = '%3'").arg("cache_size").arg(query.lastQuery()).arg(query.lastError().text()));
}

void UserDataDatabaseManager::setSyncMode(uint syncMode)
{
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	if ( (int)syncMode > dbSyncModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA synchronous = %1").arg(dbSyncModes[syncMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the user data database: query = '%2', error = '%3'").arg("synchronous").arg(query.lastQuery()).arg(query.lastError().text()));
}

void UserDataDatabaseManager::setJournalMode(uint journalMode)
{
	static QStringList dbJournalModes = QStringList() << "DELETE" << "TRUNCATE" << "PERSIST" << "MEMORY" << "WAL" << "OFF";
	if ( (int)journalMode > dbJournalModes.count() - 1 )
		return;
	QSqlQuery query(m_db);
	if ( !query.exec(QString("PRAGMA journal_mode = %1").arg(dbJournalModes[journalMode])) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to change the '%1' setting for the user data database: query = '%2', error = '%3'").arg("journal_mode").arg(query.lastQuery()).arg(query.lastError().text()));
}

void UserDataDatabaseManager::recreateDatabase()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1_metadata").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT PRIMARY KEY, rank INTEGER, comment TEXT, CONSTRAINT %1_unique_id UNIQUE (id))").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1_metadata (row INTEGER PRIMARY KEY, emu_version TEXT, qmc2_version TEXT, userdata_version INTEGER)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	recreateSoftListVisibilityTable();
	if ( logActive() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("user data database '%1' initialized").arg(m_db.databaseName()));
}

void UserDataDatabaseManager::fillUpRankCache()
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("filling up rank cache from user data database '%1'").arg(m_db.databaseName()));
	qint64 row = nextRowId(true);
	while ( row > 0 ) {
		QString idString = id(row);
		if ( !m_rankCache.contains(idString) ) {
			int rankInt = rank(row);
			m_rankCache.insert(idString, rankInt);
			QString rankString(QString::number(rankInt));
			QTreeWidgetItem *item = qmc2MachineListItemHash.value(idString);
			item->setWhatsThis(QMC2_MACHINELIST_COLUMN_RANK, rankString);
			item = qmc2HierarchyItemHash.value(idString);
			item->setWhatsThis(QMC2_MACHINELIST_COLUMN_RANK, rankString);
			item = qmc2CategoryItemHash.value(idString);
			if ( item )
				item->setWhatsThis(QMC2_MACHINELIST_COLUMN_RANK, rankString);
			item = qmc2VersionItemHash.value(idString);
			if ( item )
				item->setWhatsThis(QMC2_MACHINELIST_COLUMN_RANK, rankString);
		}
		if ( row % QMC2_RANKCACHE_FILLUP_RESPONSE == 0 )
			qApp->processEvents();
		row = nextRowId();
	}
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (filling up rank cache from user data database '%1')").arg(m_db.databaseName()));
}

void UserDataDatabaseManager::fillUpCommentCache()
{
	// FIXME
}

void UserDataDatabaseManager::setHiddenLists(QString id, QStringList hidden_lists)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT hidden_lists FROM %1 WHERE id=:id").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, hidden_lists) VALUES (:id, :hidden_lists)").arg(m_tableBasenameSL));
			query.bindValue(":id", id);
			query.bindValue(":hidden_lists", hidden_lists.join(","));
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("hidden_lists").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET hidden_lists=:hidden_lists WHERE id=:id").arg(m_tableBasenameSL));
			query.bindValue(":id", id);
			query.bindValue(":hidden_lists", hidden_lists.join(","));
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("hidden_lists").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("hidden_lists").arg(query.lastQuery()).arg(query.lastError().text()));

	if ( hidden_lists.isEmpty() )
		removeHiddenLists(id);
}

QStringList UserDataDatabaseManager::hiddenLists(QString id)
{
	QStringList hidden_lists;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT hidden_lists FROM %1 WHERE id=:id").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			hidden_lists = query.value(0).toString().split(",", QString::SkipEmptyParts);
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("hidden_lists").arg(query.lastQuery()).arg(query.lastError().text()));
	return hidden_lists;
}

void UserDataDatabaseManager::removeHiddenLists(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("DELETE FROM %1 WHERE id=:id AND (favorites IS NULL OR favorites = '') AND (device_configs IS NULL OR device_configs='')").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( !query.exec() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(id).arg(query.lastQuery()).arg(query.lastError().text()));
}

void UserDataDatabaseManager::setListFavorites(QString id, QStringList favorites)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT favorites FROM %1 WHERE id=:id").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, favorites) VALUES (:id, :favorites)").arg(m_tableBasenameSL));
			query.bindValue(":id", id);
			query.bindValue(":favorites", favorites.join(","));
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("favorites").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET favorites=:favorites WHERE id=:id").arg(m_tableBasenameSL));
			query.bindValue(":id", id);
			query.bindValue(":favorites", favorites.join(","));
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("favorites").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("favorites").arg(query.lastQuery()).arg(query.lastError().text()));

	if ( favorites.isEmpty() )
		removeListFavorites(id);
}

QStringList UserDataDatabaseManager::listFavorites(QString id)
{
	QStringList favorites;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT favorites FROM %1 WHERE id=:id").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			favorites = query.value(0).toString().split(",", QString::SkipEmptyParts);
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("favorites").arg(query.lastQuery()).arg(query.lastError().text()));
	return favorites;
}

void UserDataDatabaseManager::removeListFavorites(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("DELETE FROM %1 WHERE id=:id AND (hidden_lists IS NULL OR hidden_lists='') AND (device_configs IS NULL OR device_configs='')").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( !query.exec() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(id).arg(query.lastQuery()).arg(query.lastError().text()));
}

void UserDataDatabaseManager::setDeviceConfigs(QString id, QStringList device_configs)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT device_configs FROM %1 WHERE id=:id").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, device_configs) VALUES (:id, :device_configs)").arg(m_tableBasenameSL));
			query.bindValue(":id", id);
			query.bindValue(":device_configs", device_configs.join(","));
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("device_configs").arg(query.lastQuery()).arg(query.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET device_configs=:device_configs WHERE id=:id").arg(m_tableBasenameSL));
			query.bindValue(":id", id);
			query.bindValue(":device_configs", device_configs.join(","));
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("device_configs").arg(query.lastQuery()).arg(query.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("device_configs").arg(query.lastQuery()).arg(query.lastError().text()));

	if ( device_configs.isEmpty() )
		removeDeviceConfigs(id);
}

QStringList UserDataDatabaseManager::deviceConfigs(QString id)
{
	QStringList device_configs;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT device_configs FROM %1 WHERE id=:id").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			device_configs = query.value(0).toString().split(",");
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("device_configs").arg(query.lastQuery()).arg(query.lastError().text()));
	return device_configs;
}

void UserDataDatabaseManager::removeDeviceConfigs(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("DELETE FROM %1 WHERE id=:id AND (hidden_lists IS NULL OR hidden_lists='') AND (favorites IS NULL OR favorites='')").arg(m_tableBasenameSL));
	query.bindValue(":id", id);
	if ( !query.exec() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(id).arg(query.lastQuery()).arg(query.lastError().text()));
}

void UserDataDatabaseManager::recreateSoftListVisibilityTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_tableBasenameSL)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_tableBasenameSL)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT PRIMARY KEY, hidden_lists TEXT, favorites TEXT, device_configs TEXT, CONSTRAINT %1_unique_id UNIQUE (id))").arg(m_tableBasenameSL)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_tableBasenameSL)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
		return;
	}
}

void UserDataDatabaseManager::addSoftListFavoritesColumn()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("ALTER TABLE %1 ADD COLUMN favorites TEXT").arg(m_tableBasenameSL)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
}

void UserDataDatabaseManager::addSoftListDeviceConfigsColumn()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("ALTER TABLE %1 ADD COLUMN device_configs TEXT").arg(m_tableBasenameSL)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
}

void UserDataDatabaseManager::renameSoftListTable()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("ALTER TABLE %1 RENAME TO %2").arg(m_oldTableBasenameSL).arg(m_tableBasenameSL)) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
}

QStringList UserDataDatabaseManager::columnNames(QString tableName)
{
	QStringList column_names;
	QSqlQuery query(m_db);
	query.prepare(QString("PRAGMA table_info(%1)").arg(tableName));
	if ( query.exec() ) {
		if ( query.first() ) {
			do {
				column_names << query.value(1).toString();
			} while ( query.next() );
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg(QString("table_info(%1)").arg(tableName)).arg(query.lastQuery()).arg(query.lastError().text()));
	return column_names;
}
