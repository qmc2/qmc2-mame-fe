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
	m_tableBasename = QString("%1_xml_cache").arg(QMC2_EMU_NAME.toLower());
	if ( m_db.open() ) {
		QSqlQuery query(m_db);
		query.exec(QString("SELECT name FROM sqlite_master WHERE type='table' AND (name='%1' OR name='%1_metadata')").arg(m_tableBasename));
		if ( query.size() != 2 )
			recreateDatabase();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open XML cache database: error = '%1'").arg(m_db.lastError().text()));
}

XmlDatabaseManager::~XmlDatabaseManager()
{
	if ( m_db.isOpen() )
		m_db.close();
}

QString XmlDatabaseManager::emulatorVersion()
{
	QString emu_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.next() )
			emu_version = query.value(0).toString();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	query.finish();
	return emu_version;
}

void XmlDatabaseManager::setEmulatorVersion(QString emu_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT emu_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (emu_version, row) VALUES (:emu_version, 0)").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to XML cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET emu_version=:emu_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":emu_version", emu_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in XML cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		}
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("emu_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

QString XmlDatabaseManager::qmc2Version()
{
	QString qmc2_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.next() )
			qmc2_version = query.value(0).toString();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	query.finish();
	return qmc2_version;
}

void XmlDatabaseManager::setQmc2Version(QString qmc2_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (qmc2_version, row) VALUES (:qmc2_version, 0)").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to XML cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET qmc2_version=:qmc2_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in XML cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		}
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

int XmlDatabaseManager::xmlCacheVersion()
{
	int xmlcache_version = -1;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT xmlcache_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.next() )
			xmlcache_version = query.value(0).toInt();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("xmlcache_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	query.finish();
	return xmlcache_version;
}

void XmlDatabaseManager::setXmlCacheVersion(int xmlcache_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT xmlcache_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (xmlcache_version, row) VALUES (:xmlcache_version, 0)").arg(m_tableBasename));
			query.bindValue(":xmlcache_version", xmlcache_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to XML cache database: query = '%2', error = '%3'").arg("xmlcache_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET xmlcache_version=:xmlcache_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":xmlcache_version", xmlcache_version);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in XML cache database: query = '%2', error = '%3'").arg("xmlcache_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		}
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("xmlcache_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

QString XmlDatabaseManager::dtd()
{
	QString dtd;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT dtd FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.next() )
			dtd = query.value(0).toString();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("dtd").arg(query.lastQuery()).arg(m_db.lastError().text()));
	query.finish();
	return dtd;
}

void XmlDatabaseManager::setDtd(QString dtd)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT dtd FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (dtd, row) VALUES (:dtd, 0)").arg(m_tableBasename));
			query.bindValue(":dtd", dtd);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to XML cache database: query = '%2', error = '%3'").arg("dtd").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET dtd=:dtd WHERE row=0").arg(m_tableBasename));
			query.bindValue(":dtd", dtd);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in XML cache database: query = '%2', error = '%3'").arg("dtd").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		}
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("dtd").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

QString XmlDatabaseManager::xml(QString id)
{
	QString xml;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT xml FROM %1 WHERE id=:id").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( query.next() )
			xml = query.value(0).toString();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(m_db.lastError().text()));
	query.finish();
	return xml;
}

void XmlDatabaseManager::setXml(QString id, QString xml)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT xml FROM %1 WHERE id=:id").arg(m_tableBasename));
	query.bindValue(":id", id);
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1 (id, xml) VALUES (:id, :xml)").arg(m_tableBasename));
			query.bindValue(":id", id);
			query.bindValue(":xml", xml);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add '%1' to XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET xml=:xml WHERE id=:id").arg(m_tableBasename));
			query.bindValue(":id", id);
			query.bindValue(":xml", xml);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update '%1' in XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(m_db.lastError().text()));
			query.finish();
		}
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch '%1' from XML cache database: query = '%2', error = '%3'").arg("xml").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

void XmlDatabaseManager::recreateDatabase()
{
	QSqlQuery query(m_db);
	bool success = true;
	query.exec(QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1'").arg(m_tableBasename));
	if ( query.next() ) {
		query.finish();
		if ( !query.exec(QString("DROP TABLE %1").arg(m_tableBasename)) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
			success = false;
		}
	}
	if ( success ) {
		query.exec(QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1_metadata'").arg(m_tableBasename));
		if ( query.next() ) {
			query.finish();
			if ( !query.exec(QString("DROP TABLE %1_metadata").arg(m_tableBasename)) ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
				success = false;
			}
		}
		if ( success ) {
			if ( !query.exec(QString("CREATE TABLE %1 (id TEXT PRIMARY KEY, xml TEXT, CONSTRAINT %1_unique_id UNIQUE (id))").arg(m_tableBasename)) ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
				success = false;
			}
			query.finish();
			if ( success ) {
				if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (id)").arg(m_tableBasename)) ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
					success = false;
				}
				query.finish();
				if ( success ) {
					if ( !query.exec(QString("CREATE TABLE %1_metadata (row INTEGER PRIMARY KEY, dtd TEXT, emu_version TEXT, qmc2_version TEXT, xmlcache_version INTEGER)").arg(m_tableBasename)) ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create XML cache database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
						success = false;
					}
					query.finish();
				}
			}
		}
	}
}
