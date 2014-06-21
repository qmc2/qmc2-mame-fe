#ifndef _DOWNLOADITEM_H_
#define _DOWNLOADITEM_H_

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QProgressBar>
#include <QNetworkReply>
#include <QString>
#include <QFile>
#include <QTimer>
#include "macros.h"

class ItemDownloader;

class DownloadItem : public QTreeWidgetItem
{
	public:
		QProgressBar *progressWidget;
		ItemDownloader *itemDownloader;
		QTreeWidget *treeWidget;

		DownloadItem(QNetworkReply *, QString, QTreeWidget *);
		~DownloadItem();
};

class ItemDownloader : public QObject
{
	Q_OBJECT

	public:
		qint64 dataReceived;
		qint64 downloadBytesTotal;
		qreal downloadPercent;
		int retryCount;
		QNetworkReply *networkReply;
		QProgressBar *progressWidget;
		QString localPath;
		QFile localFile;
		DownloadItem *downloadItem;
		QTimer errorCheckTimer;

		ItemDownloader(QNetworkReply *, QString, QProgressBar *, DownloadItem *);

	public slots:
		void init();
		void readyRead();
		void error(QNetworkReply::NetworkError);
		void downloadProgress(qint64, qint64);
		void metaDataChanged();
		void finished();
		void managerFinished(QNetworkReply *);
		void reload();
		void checkError();
};

#endif
