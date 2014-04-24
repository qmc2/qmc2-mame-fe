#include <QApplication>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QTreeWidgetItem>
#include <QDir>
#include <QUuid>
#include <QMap>

#include "macros.h"
#include "qmc2main.h"
#include "settings.h"
#include "gamelist.h"
#include "userdatadbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Gamelist *qmc2Gamelist;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif

UserDataDatabaseManager::UserDataDatabaseManager(QObject *parent)
	: QObject(parent)
{
	setLogActive(true);
	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	m_db = QSqlDatabase::addDatabase("QSQLITE", "user-data-db-connection-" + QUuid::createUuid().toString());
	m_db.setDatabaseName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/UserDataDatabase", QString(userScopePath + "/%1-user-data.db").arg(QMC2_EMU_NAME.toLower())).toString());
	m_tableBasename = QString("%1_user_data").arg(QMC2_EMU_NAME.toLower());
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() != 2 || !tables.contains(m_tableBasename) || !tables.contains(QString("%1_metadata").arg(m_tableBasename)) )
			recreateDatabase();
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET emu_version=:emu_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET qmc2_version=:qmc2_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("userdata_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("userdata_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET userdata_version=:userdata_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":userdata_version", userdata_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("userdata_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("userdata_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET rank=:rank WHERE id=:id").arg(m_tableBasename));
			query.bindValue(":id", id);
			query.bindValue(":rank", rank);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("rank").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(m_db.lastError().text()));
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
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1 SET comment=:comment WHERE id=:id").arg(m_tableBasename));
			query.bindValue(":id", id);
			query.bindValue(":comment", comment);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("comment").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

int UserDataDatabaseManager::userDataRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_tableBasename)) ) {
		if ( query.first() )
			return query.value(0).toInt();
		else
			return -1;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row count from user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}	return -1;
}

int UserDataDatabaseManager::nextRowId(bool refreshRowIds)
{
	if ( refreshRowIds ) {
		m_rowIdList.clear();
		m_lastRowId = -1;
		// FIXME: get list of row IDs and return the first one
		QSqlQuery query(m_db);
		if ( query.exec(QString("SELECT rowid FROM %1").arg(m_tableBasename)) ) {
			if ( query.first() ) {
				do {
					m_rowIdList << query.value(0).toInt();
				} while ( query.next() );
				m_lastRowId = 0;
				return m_rowIdList[0];
			}
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch row IDs from user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
			return -1;
		}
	} else if ( m_lastRowId > -1 ) {
		m_lastRowId++;
		if ( m_lastRowId < m_rowIdList.count() )
			return m_rowIdList[m_lastRowId];
		else
			return -1;
	} else
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
			return false;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return false;
	}
}

bool UserDataDatabaseManager::exists(QString id)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT id FROM %1 WHERE id=:id").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.first() )
			return query.value(0).toString() == id;
		else
			return false;
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from user data database: query = '%2', error = '%3'").arg("id").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return false;
	}
}

void UserDataDatabaseManager::cleanUp()
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("cleaning up user data database '%1'").arg(m_db.databaseName()));

	int row = nextRowId(true);
	beginTransaction();
	while ( row > 0 ) {
		QString idOfCurrentRow = id(row);
		if ( !idOfCurrentRow.isEmpty() ) {
			if ( !qmc2Gamelist->xmlDb()->exists(idOfCurrentRow) ) {
				QSqlQuery query(m_db);
				query.prepare(QString("DELETE FROM %1 WHERE rowid=:row").arg(m_tableBasename));
				query.bindValue(":row", row);
				if ( query.exec() ) {
					query.finish();
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("removed obsolete ID '%1'").arg(idOfCurrentRow));
				} else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(idOfCurrentRow).arg(query.lastQuery()).arg(m_db.lastError().text()));
			} else if ( rank(idOfCurrentRow) == 0 && comment(idOfCurrentRow).isEmpty() ) {
				QSqlQuery query(m_db);
				query.prepare(QString("DELETE FROM %1 WHERE rowid=:row").arg(m_tableBasename));
				query.bindValue(":row", row);
				if ( query.exec() ) {
					query.finish();
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("removed ID '%1' due to no data").arg(idOfCurrentRow));
				} else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(idOfCurrentRow).arg(query.lastQuery()).arg(m_db.lastError().text()));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove '%1' from user data database: query = '%2', error = '%3'").arg(id).arg(query.lastQuery()).arg(m_db.lastError().text()));
	m_rankCache.remove(id);
	m_commentCache.remove(id);
}

void UserDataDatabaseManager::recreateDatabase()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1_metadata").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (id TEXT PRIMARY KEY, rank INTEGER, comment TEXT, CONSTRAINT %1_unique_id UNIQUE (id))").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1_metadata (row INTEGER PRIMARY KEY, emu_version TEXT, qmc2_version TEXT, userdata_version INTEGER)").arg(m_tableBasename)) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create user data database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	if ( logActive() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("user data database '%1' initialized").arg(m_db.databaseName()));
}

void UserDataDatabaseManager::fillUpRankCache()
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("filling up rank cache from user data database '%1'").arg(m_db.databaseName()));
	int row = nextRowId(true);
	while ( row > 0 ) {
		QString idString = id(row);
		if ( !m_rankCache.contains(idString) ) {
			int rankInt = rank(row);
			m_rankCache[idString] = rankInt;
			QString rankString = QString::number(rankInt);
			QTreeWidgetItem *item = qmc2GamelistItemMap[idString];
			if ( item )
				item->setWhatsThis(QMC2_GAMELIST_COLUMN_RANK, rankString);
			item = qmc2HierarchyItemMap[idString];
			if ( item)
				item->setWhatsThis(QMC2_GAMELIST_COLUMN_RANK, rankString);
			item = qmc2CategoryItemMap[idString];
			if ( item )
				item->setWhatsThis(QMC2_GAMELIST_COLUMN_RANK, rankString);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
			item = qmc2VersionItemMap[idString];
			if ( item )
				item->setWhatsThis(QMC2_GAMELIST_COLUMN_RANK, rankString);
#endif
		}
		if ( row % 25 == 0 )
			qApp->processEvents();
		row = nextRowId();
	}
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (filling up rank cache from user data database '%1')").arg(m_db.databaseName()));
}

void UserDataDatabaseManager::fillUpCommentCache()
{
	// FIXME
}
