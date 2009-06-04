#ifndef _DOWNLOADITEM_H_
#define _DOWNLOADITEM_H_

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QProgressBar>
#include <QNetworkReply>
#include <QString>
#include <QFile>
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
    QNetworkReply *networkReply;
    QProgressBar *progressWidget;
    QString localPath;
    QFile localFile;
    DownloadItem *downloadItem;

    ItemDownloader(QNetworkReply *, QString, QProgressBar *, DownloadItem *);
    ~ItemDownloader();

  public slots:
    void readyRead();
    void error(QNetworkReply::NetworkError);
    void downloadProgress(qint64, qint64);
    void metaDataChanged();
    void finished();
};

#endif
