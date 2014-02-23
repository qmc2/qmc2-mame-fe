#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

class NetworkAccessManager : public QNetworkAccessManager
{
	Q_OBJECT

	public:
		NetworkAccessManager(QNetworkAccessManager *, QObject *parent = 0);

	protected:
		QNetworkReply *createRequest(Operation, const QNetworkRequest &, QIODevice *);
};

#endif
