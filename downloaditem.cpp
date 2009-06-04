#include <QApplication>

#include "qmc2main.h"
#include "macros.h"
#include "downloaditem.h"

extern MainWindow *qmc2MainWindow;

#define QMC2_DEBUG

DownloadItem::DownloadItem(QNetworkReply *reply, QString file, QTreeWidget *parent)
  : QTreeWidgetItem(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DownloadItem::DownloadItem(QNetworkReply *reply = %1, QString file = %2, QTreeWidget *parent = %3)")
                      .arg((qulonglong)reply).arg(file).arg((qulonglong)parent));
#endif

  progressWidget = NULL;
  itemDownloader = NULL;
  treeWidget = parent;

  progressWidget = new QProgressBar(0);
  QFont f(qApp->font());
  f.setPointSize(f.pointSize() - 2);
  progressWidget->setFont(f);
  progressWidget->setFormat(reply->url().toString());
  treeWidget->setItemWidget(this, QMC2_DOWNLOAD_COLUMN_PROGRESS, progressWidget);
  setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/download.png")));
  treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
  progressWidget->reset();
  progressWidget->setRange(-1, -1);
  progressWidget->setValue(-1);
  progressWidget->show();

  itemDownloader = new ItemDownloader(reply, file, progressWidget, this);
}

DownloadItem::~DownloadItem()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DownloadItem::~DownloadItem()");
#endif

  if ( progressWidget )
    delete progressWidget;
  if ( itemDownloader )
    delete itemDownloader;
}

ItemDownloader::ItemDownloader(QNetworkReply *reply, QString file, QProgressBar *progress, DownloadItem *parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::ItemDownloader(QNetworkReply *reply = %1, QString file = %2, QProgressBar *progress = %3, DownloadItem *parent = %4)")
                      .arg((qulonglong)reply).arg(file).arg((qulonglong)progress).arg((qulonglong)parent));
#endif

  networkReply = reply;
  localPath = file;
  progressWidget = progress;
  downloadItem = parent;

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download started: URL = %1, local path = %2, reply ID = %3")
                      .arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));

  connect(networkReply, SIGNAL(readyRead()), this, SLOT(readyRead()));
  connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
  connect(networkReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
  connect(networkReply, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
  connect(networkReply, SIGNAL(finished()), this, SLOT(finished()));
}

ItemDownloader::~ItemDownloader()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ItemDownloader::~ItemDownloader()");
#endif

}

void ItemDownloader::readyRead()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::readyRead(): networkReply = %1").arg((qulonglong)networkReply));
#endif

  QByteArray buffer = networkReply->readAll();

  // FIXME: do something with the data...
}

void ItemDownloader::error(QNetworkReply::NetworkError code)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::error(QNetworkReply::NetworkError code = %1): networkReply = %2").arg(code).arg((qulonglong)networkReply));
#endif

  progressWidget->setFormat(tr("Error #%1: ").arg(code) + networkReply->url().toString());
  downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/warning.png")));
  downloadItem->treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
  progressWidget->setEnabled(FALSE);

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download aborted: reason = %1, URL = %2, local path = %3, reply ID = %4")
                      .arg(networkReply->errorString()).arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));
}

void ItemDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::downloadProgress(qint64 bytesReceived = %1, qint64 bytesTotal = %2): networkReply = %3").arg(bytesReceived).arg(bytesTotal).arg((qulonglong)networkReply));
#endif

  progressWidget->setRange(0, bytesTotal); // FIXME: may overflow!?
  progressWidget->setValue(bytesReceived);
}

void ItemDownloader::metaDataChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::metaDataChanged(): networkReply = %1").arg((qulonglong)networkReply));
#endif

}

void ItemDownloader::finished()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::finished(): networkReply = %1").arg((qulonglong)networkReply));
#endif

  progressWidget->setValue(progressWidget->maximum());
  downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/ok.png")));
  downloadItem->treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
  progressWidget->setEnabled(FALSE);

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download finished: URL = %1, local path = %2, reply ID = %3")
                      .arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));
}
