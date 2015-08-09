#include <QDir>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QMultiMap>
#include <QUuid>

#include "settings.h"
#include "options.h"
#include "cookiejar.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

CookieJar::CookieJar(QObject *parent) : QNetworkCookieJar(parent)
{
	QString userScopePath = Options::configPath();
	db = QSqlDatabase::addDatabase("QSQLITE", "cookie-db-connection-" + QUuid::createUuid().toString());
	db.setDatabaseName(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/CookieDatabase", userScopePath + "/qmc2-" + QMC2_EMU_NAME_VARIANT.toLower() + "-cookies.db").toString());
	if ( !db.open() ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to open cookie database: error = '%1'").arg(db.lastError().text()));
		return;
	}
	QStringList tables = db.driver()->tables(QSql::Tables);
	if ( tables.count() != 1 || !tables.contains("qmc2_cookies") )
		recreateDatabase();
	static QStringList dbSyncModes = QStringList() << "OFF" << "NORMAL" << "FULL";
	QSqlQuery query(db);
	query.exec("PRAGMA synchronous = OFF");
	query.exec("PRAGMA journal_mode = MEMORY");
}

CookieJar::~CookieJar()
{
	if ( db.isOpen() ) {
		saveCookies();
		db.close();
	}
}

void CookieJar::recreateDatabase()
{
	if ( !db.isOpen() )
		return;

	QSqlQuery query(db);
	if ( !query.exec("DROP TABLE IF EXISTS qmc2_cookies") ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove cookie database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(db.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec("CREATE TABLE qmc2_cookies (id INTEGER PRIMARY KEY, domain TEXT, name TEXT, value TEXT, path TEXT, expiry INTEGER, secure INTEGER, http_only INTEGER, CONSTRAINT qmc2_uniqueid UNIQUE (name, domain, path))") )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to create cookie database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(db.lastError().text()));
	cookieMap.clear();
	setAllCookies(QList<QNetworkCookie>());
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
	QString domain = url.host();
	QString path = url.path();
	QString defaultPath = path.left(path.lastIndexOf(QLatin1Char('/')) + 1);
	if ( defaultPath.isEmpty() )
		defaultPath = QLatin1Char('/');
	QList<QNetworkCookie> cookieList;
	if ( loadCookies(cookieList, domain, defaultPath) )
		return cookieList;
	else
		return QNetworkCookieJar::cookiesForUrl(url);
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
	QString domain = url.host();
	QString path = url.path();
	QString defaultPath = path.left(path.lastIndexOf(QLatin1Char('/')) + 1);
	if ( defaultPath.isEmpty() )
		defaultPath = QLatin1Char('/');
	for (int i = 0; i < cookieList.count(); i++) {
		QNetworkCookie cookie = cookieList[i];
		cookie.setDomain(domain);
		cookie.setPath(defaultPath);
		cookieMap.insertMulti(domain + defaultPath, cookie);
	}
	return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
}

void CookieJar::saveCookies()
{
	if ( !db.isOpen() )
		return;

	QSqlQuery query(db);
	QDateTime now = QDateTime::currentDateTime();

	QMapIterator<QString, QNetworkCookie> it(cookieMap);
	QStringList cookieKeysProcessed;
	db.driver()->beginTransaction();
	while ( it.hasNext() ) {
		it.next();
		QNetworkCookie cookie = it.value();
		QString cookieKey = cookie.domain() + cookie.path() + cookie.name();
		if ( cookieKeysProcessed.contains(cookieKey) )
			continue;
		query.prepare("SELECT domain, name, path FROM qmc2_cookies WHERE domain=:domain AND path=:path AND name=:name");
		query.bindValue(":domain", cookie.domain());
		query.bindValue(":path", cookie.path());
		query.bindValue(":name", cookie.name());
		if ( query.exec() ) {
			if ( query.next() ) {
				query.finish();
				if ( cookie.value().isEmpty() ) {
					query.prepare("DELETE FROM qmc2_cookies WHERE domain=:domain AND path=:path AND name=:name");
					query.bindValue(":domain", cookie.domain());
					query.bindValue(":path", cookie.path());
					query.bindValue(":name", cookie.name());
					if ( !query.exec() )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove expired cookie from database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(db.lastError().text()));
					query.finish();
					cookieKeysProcessed << cookieKey;
				} else if ( cookie.expirationDate() < now ) {
					query.prepare("DELETE FROM qmc2_cookies WHERE domain=:domain AND path=:path AND name=:name");
					query.bindValue(":domain", cookie.domain());
					query.bindValue(":path", cookie.path());
					query.bindValue(":name", cookie.name());
					if ( !query.exec() )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove expired cookie from database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(db.lastError().text()));
					query.finish();
					cookieKeysProcessed << cookieKey;
				} else if ( !cookie.isSessionCookie() ) {
					query.prepare("UPDATE qmc2_cookies SET value=:value, expiry=" + QString::number(cookie.expirationDate().toTime_t()) + " WHERE domain=:domain AND path=:path AND name=:name");
					query.bindValue(":value", cookie.value());
					query.bindValue(":domain", cookie.domain());
					query.bindValue(":path", cookie.path());
					query.bindValue(":name", cookie.name());
					if ( !query.exec() )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to update cookie in database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(db.lastError().text()));
					query.finish();
					cookieKeysProcessed << cookieKey;
				}
			} else {
				query.finish();
				if ( cookie.expirationDate() > now && !cookie.isSessionCookie() ) {
					query.prepare("INSERT INTO qmc2_cookies (domain, name, value, path, expiry, secure, http_only) VALUES (:domain, :name, :value, :path, " + QString::number(cookie.expirationDate().toTime_t()) + ", " + QString(cookie.isSecure() ? "1" : "0") + ", " + QString(cookie.isHttpOnly() ? "1" : "0") + ")");
					query.bindValue(":value", cookie.value());
					query.bindValue(":domain", cookie.domain());
					query.bindValue(":path", cookie.path());
					query.bindValue(":name", cookie.name());
					if ( !query.exec() )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to add cookie to database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(db.lastError().text()));
					query.finish();
					cookieKeysProcessed << cookieKey;
				}
			}
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to query cookie database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(db.lastError().text()));
	}
	db.driver()->commitTransaction();
}

bool CookieJar::loadCookies(QList<QNetworkCookie> &cookieList, QString domain, QString path) const
{
	cookieList.clear();

	if ( cookieMap.contains(domain + path) ) {
		cookieList = cookieMap.values(domain + path);
		return !cookieList.isEmpty();
	}

	if ( !db.isOpen() )
		return false;

	QSqlQuery query(db);
	query.prepare("SELECT domain, name, value, path, expiry, secure, http_only FROM qmc2_cookies WHERE domain=:domain AND path=:path");
	query.bindValue(":domain", domain);
	query.bindValue(":path", path);
	if ( !query.exec() ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to fetch cookies from database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(db.lastError().text()));
		return false;
	}
	QDateTime now = QDateTime::currentDateTime();
	QDateTime dt;
	db.driver()->beginTransaction();
	while ( query.next() ) {
		QNetworkCookie cookie;
		cookie.setDomain(query.value(0).toString());
		cookie.setName(query.value(1).toByteArray());
		cookie.setValue(query.value(2).toByteArray());
		cookie.setPath(query.value(3).toString());
		dt.setTime_t((uint) query.value(4).toULongLong());
		cookie.setExpirationDate(dt);
		cookie.setSecure(query.value(5).toBool());
		cookie.setHttpOnly(query.value(6).toBool());
		if ( dt < now ) {
			QSqlQuery delquery(db);
			delquery.prepare("DELETE FROM qmc2_cookies WHERE domain=:domain AND path=:path AND name=:name");
			delquery.bindValue(":domain", cookie.domain());
			delquery.bindValue(":path", cookie.path());
			delquery.bindValue(":name", cookie.name());
			if ( !delquery.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove expired cookie from database: query = '%1', error = '%2'").arg(delquery.lastQuery()).arg(db.lastError().text()));
		} else {
			cookieList << cookie;
			cookieMap.insertMulti(domain + path, cookie);
		}
	}
	db.driver()->commitTransaction();
	return !cookieList.isEmpty();
}
