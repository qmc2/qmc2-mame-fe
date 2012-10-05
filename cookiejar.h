#ifndef _COOKIEJAR_H_
#define _COOKIEJAR_H_

#include <QString>
#include <QSqlDatabase>
#include <QMultiMap>
#include <QNetworkCookieJar>

class CookieJar : public QNetworkCookieJar
{
	Q_OBJECT

       	public:
		QSqlDatabase db;
		mutable QMultiMap<QString, QNetworkCookie> cookieMap;

		CookieJar(QObject *parent = 0);
		~CookieJar();

		virtual QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;

		bool saveCookies();
		bool loadCookies(QList<QNetworkCookie> &, QString, QString) const;

		void recreateDatabase();
};

#endif
