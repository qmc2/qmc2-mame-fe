#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>

#include "macros.h"
#include "qmc2main.h"
#include "settings.h"
#include "xmldbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

XmlDatabaseManager::XmlDatabaseManager(QObject *parent)
	: QObject(parent)
{
	static quint64 connectionNumber = 0;
	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	m_db = QSqlDatabase::addDatabase("QSQLITE", "xmlcachedb-" + QString::number(connectionNumber++));
	m_db.setDatabaseName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/XmlCacheDatabase", QString(userScopePath + "/%1-xml-cache.db").arg(QMC2_EMU_NAME.toLower())).toString());
	if ( m_db.open() ) {
		QSqlQuery query(m_db);
		query.exec(QString("SELECT name FROM sqlite_master WHERE type='table' AND (name='%1' OR name='%1_dtd' OR name='%1_version')").arg(QString("qmc2_%1_xml_cache").arg(QMC2_EMU_NAME.toLower())));
		if ( query.size() != 3 )
			recreateDatabase();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open XML cache database: error = '%1'").arg(m_db.lastError().text()));
}

XmlDatabaseManager::~XmlDatabaseManager()
{
	if ( m_db.isOpen() )
		m_db.close();
}

void XmlDatabaseManager::recreateDatabase()
{
	if ( !m_db.isOpen() )
		return;

	QSqlQuery query(m_db);
	QString tableName = QString("qmc2_%1_xml_cache").arg(QMC2_EMU_NAME.toLower());
	bool success = true;
	query.exec(QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1'").arg(tableName));
	if ( query.next() ) {
		query.finish();
		if ( !query.exec(QString("DROP TABLE %1").arg(tableName)) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
			success = false;
		}
	}
	if ( success ) {
		query.exec(QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1_dtd'").arg(tableName));
		if ( query.next() ) {
			query.finish();
			if ( !query.exec(QString("DROP TABLE %1_dtd").arg(tableName)) ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
				success = false;
			}
		}
		if ( success ) {
			query.exec(QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1_version'").arg(tableName));
			if ( query.next() ) {
				query.finish();
				if ( !query.exec(QString("DROP TABLE %1_version").arg(tableName)) ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
					success = false;
				}
			}
			if ( success ) {
				if ( !query.exec(QString("CREATE TABLE %1 (id TEXT PRIMARY KEY, xml TEXT, CONSTRAINT qmc2_uniqueid UNIQUE (id))").arg(tableName)) ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
					success = false;
				}
				query.finish();
				if ( success ) {
					if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(tableName)) ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
						success = false;
					}
					query.finish();
					if ( success ) {
						if ( !query.exec(QString("CREATE TABLE %1_dtd (dtd TEXT)").arg(tableName)) ) {
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
							success = false;
						}
						query.finish();
						if ( success ) {
							if ( !query.exec(QString("CREATE TABLE %1_version (emu_version TEXT, qmc2_version TEXT, xmlcache_version INTEGER)").arg(tableName)) ) {
								qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
								success = false;
							}
							query.finish();
						}
					}
				}
			}
		}
	}
}
