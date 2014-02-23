#include <QtNetwork>
#include "networkaccessmanager.h"
#include "ftpreply.h"

NetworkAccessManager::NetworkAccessManager(QNetworkAccessManager *manager, QObject *parent)
	: QNetworkAccessManager(parent)
{
	if ( manager ) {
		setCache(manager->cache());
		setCookieJar(manager->cookieJar());
		setProxy(manager->proxy());
		setProxyFactory(manager->proxyFactory());
	}
}

QNetworkReply *NetworkAccessManager::createRequest(QNetworkAccessManager::Operation operation, const QNetworkRequest &request, QIODevice *device)
{
	if ( request.url().scheme() != "ftp" )
		return QNetworkAccessManager::createRequest(operation, request, device);

	if ( operation == GetOperation )
		return new FtpReply(request.url());
	else
		return QNetworkAccessManager::createRequest(operation, request, device);
}
