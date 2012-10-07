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
		mutable QSqlDatabase db;
		mutable QMultiMap<QString, QNetworkCookie> cookieMap;

		CookieJar(QObject *parent = 0);
		~CookieJar();

		virtual QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
		virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

		void saveCookies();
		bool loadCookies(QList<QNetworkCookie> &, QString, QString) const;

		void recreateDatabase();
};

#endif
