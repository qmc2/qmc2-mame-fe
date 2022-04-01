#include <QApplication>
#include <QNetworkAccessManager>
#include <QToolTip>

#include "settings.h"
#include "qmc2main.h"
#include "macros.h"
#include "ftpreply.h"
#include "downloaditem.h"
#include "networkaccessmanager.h"
#include "romalyzer.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern NetworkAccessManager *qmc2NetworkAccessManager;
extern Settings *qmc2Config;

DownloadItem::DownloadItem(QNetworkReply *reply, QString file, QTreeWidget *parent)
	: QTreeWidgetItem(parent)
{
	progressWidget = 0;
	itemDownloader = 0;
	treeWidget = parent;

	progressWidget = new QProgressBar(0);
	QFont f(qApp->font());
	f.setPointSize(f.pointSize() - 2);
	progressWidget->setFont(f);
	progressWidget->setFormat(reply->url().toString());
	treeWidget->setItemWidget(this, QMC2_DOWNLOAD_COLUMN_PROGRESS, progressWidget);
	setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/download.png")));
	setWhatsThis(0, "initializing");
	treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
	progressWidget->reset();
	progressWidget->setRange(-1, -1);
	progressWidget->setValue(-1);
	progressWidget->show();

	itemDownloader = new ItemDownloader(reply, file, progressWidget, this);
}

DownloadItem::~DownloadItem()
{
	if ( progressWidget )
		delete progressWidget;

	if ( itemDownloader )
		delete itemDownloader;
}

ItemDownloader::ItemDownloader(QNetworkReply *reply, QString file, QProgressBar *progress, DownloadItem *parent)
{
	networkReply = reply;
	localPath = file;
	progressWidget = progress;
	downloadItem = parent;
	retryCount = 0;
	downloadPercent = 0.0;

	connect(&errorCheckTimer, SIGNAL(timeout()), this, SLOT(checkError()));
	QTimer::singleShot(0, this, SLOT(reload()));
}

void ItemDownloader::init()
{
	if ( !networkReply )
		return;

	if ( !networkReply->open(QIODevice::ReadOnly) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open network reply for reading"));
		finished();
		return;
	}

	if ( localFile.isOpen() )
		localFile.close();

	localFile.setFileName(localPath);

	if ( localFile.exists() )
		localFile.remove();

	if ( !localFile.open(QIODevice::WriteOnly) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open '%1' for writing").arg(localPath));
		finished();
		return;
	}

	dataReceived = downloadBytesTotal = 0;
	downloadPercent = 0.0;
	retryCount++;
	progressWidget->reset();
	progressWidget->setRange(-1, -1);
	progressWidget->setValue(-1);
	progressWidget->setFormat(networkReply->url().toString());
	downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/download.png")));
	downloadItem->progressWidget->setEnabled(true);
	downloadItem->setWhatsThis(0, "downloading");
	downloadItem->setToolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS,
			tr("Source URL: %1").arg(networkReply->url().toString()) + "\n" +
			tr("Local path: %2").arg(localPath) + "\n" +
			tr("Status: %1").arg(tr("initializing download")) + "\n" +
			tr("Total size: %1").arg(tr("unknown")) + "\n" +
			tr("Downloaded: %1 (%2%)").arg(0).arg(downloadPercent, 0, 'f', 2));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download started: URL = %1, local path = %2, reply ID = %3")
			.arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));

	networkReply->disconnect(this);
	connect(networkReply, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
	connect(networkReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
	connect(networkReply, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
	connect(networkReply, SIGNAL(finished()), this, SLOT(finished()));
	qmc2NetworkAccessManager->disconnect(this);
	connect(qmc2NetworkAccessManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(managerFinished(QNetworkReply *)));

	errorCheckTimer.start(QMC2_DOWNLOAD_CHECK_TIMEOUT);
}

void ItemDownloader::checkError()
{
	if ( networkReply->error() != QNetworkReply::NoError ) {
		error(networkReply->error());
		finished();
	}

	if ( dataReceived == 0 ) {
		if ( retryCount < QMC2_DOWNLOAD_OPCANCEL_RETRY )
			QTimer::singleShot((qrand() % 6 + 5) * QMC2_DOWNLOAD_RETRY_DELAY, this, SLOT(reload()));
		else {
			error(QNetworkReply::TimeoutError);
			finished();
		}
	}
}

void ItemDownloader::readyRead()
{
	QByteArray buffer = networkReply->readAll();
	if ( buffer.length() > 0 ) {
		dataReceived += buffer.length();
		downloadBytesTotal = networkReply->size();
		if ( localFile.isOpen() )
			localFile.write(buffer);
		retryCount = 0;
	} else if ( networkReply->url().scheme() == "ftp" ) {
		dataReceived = networkReply->bytesAvailable();
		downloadBytesTotal = ((FtpReply *)networkReply)->totalSize(networkReply->url().toString());
		if ( downloadBytesTotal == -1 )
			downloadBytesTotal = dataReceived;
		downloadProgress(dataReceived, downloadBytesTotal);
		retryCount = 0;
	}
}

void ItemDownloader::error(QNetworkReply::NetworkError code)
{
	if ( code == QNetworkReply::OperationCanceledError )
		downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/stop_browser.png")));
	else {
		progressWidget->setFormat(tr("Error #%1: ").arg(code) + networkReply->url().toString());
		downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/warning.png")));
	}
	downloadItem->treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
	progressWidget->setEnabled(false);

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download aborted: reason = %1, URL = %2, local path = %3, reply ID = %4")
			.arg(networkReply->errorString()).arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));
	downloadItem->setWhatsThis(0, "aborted");
	errorCheckTimer.stop();

	if ( localFile.isOpen() )
		localFile.close();

	QString idString = tr("Source URL: %1").arg(networkReply->url().toString()) + "\n" + tr("Local path: %1").arg(localPath);
	downloadItem->setToolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS,
			idString + "\n" +
			tr("Status: %1").arg(tr("download aborted")) + "\n" +
			tr("Total size: %1").arg(ROMAlyzer::humanReadable(downloadBytesTotal, 2)) + "\n" +
			tr("Downloaded: %1 (%2%)").arg(ROMAlyzer::humanReadable(dataReceived, 2)).arg(downloadPercent, 0, 'f', 2));

	if ( QToolTip::isVisible() && QToolTip::text().startsWith(idString) && downloadItem->treeWidget->visualItemRect(downloadItem).contains(downloadItem->treeWidget->viewport()->mapFromGlobal(QCursor::pos())) )
		QToolTip::showText(QCursor::pos(), downloadItem->toolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS));
}

void ItemDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if ( bytesTotal <= 0 )
		return;

	downloadPercent = ((qreal)bytesReceived / (qreal)bytesTotal) * 100.0;

	downloadBytesTotal = bytesTotal;
	dataReceived = bytesReceived;

	progressWidget->setRange(0, 100);
	progressWidget->setValue(int(downloadPercent));

	QString idString = tr("Source URL: %1").arg(networkReply->url().toString()) + "\n" + tr("Local path: %1").arg(localPath);
	downloadItem->setToolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS,
			idString + "\n" +
			tr("Status: %1").arg(tr("downloading")) + "\n" +
			tr("Total size: %1").arg(ROMAlyzer::humanReadable(bytesTotal, 2)) + "\n" +
			tr("Downloaded: %1 (%2%)").arg(ROMAlyzer::humanReadable(bytesReceived, 2)).arg(downloadPercent, 0, 'f', 2));

	if ( QToolTip::isVisible() && QToolTip::text().startsWith(idString) && downloadItem->treeWidget->visualItemRect(downloadItem).contains(downloadItem->treeWidget->viewport()->mapFromGlobal(QCursor::pos())) )
		QToolTip::showText(QCursor::pos(), downloadItem->toolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS));
}

void ItemDownloader::metaDataChanged()
{
    // NOP
}

void ItemDownloader::finished()
{
	errorCheckTimer.stop();

	if ( downloadItem->whatsThis(0) != "aborted" && downloadItem->whatsThis(0) != "finished" ) {
		downloadItem->setWhatsThis(0, "finished");
		progressWidget->setValue(progressWidget->maximum());
		downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/ok.png")));
		downloadItem->treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download finished: URL = %1, local path = %2, reply ID = %3")
				.arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));
		QString idString = tr("Source URL: %1").arg(networkReply->url().toString()) + "\n" + tr("Local path: %1").arg(localPath);
		if ( networkReply->url().scheme() == "ftp" ) {
			dataReceived = dataReceived / 2;
			downloadBytesTotal = dataReceived;
		} else
			dataReceived = downloadBytesTotal;
		downloadItem->setToolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS,
				idString + "\n" +
				tr("Status: %1").arg(tr("download finished")) + "\n" +
				tr("Total size: %1").arg(ROMAlyzer::humanReadable(downloadBytesTotal, 2)) + "\n" +
				tr("Downloaded: %1 (%2%)").arg(ROMAlyzer::humanReadable(dataReceived, 2)).arg(downloadPercent, 0, 'f', 2));
		if ( QToolTip::isVisible() && QToolTip::text().startsWith(idString) && downloadItem->treeWidget->visualItemRect(downloadItem).contains(downloadItem->treeWidget->viewport()->mapFromGlobal(QCursor::pos())) )
			QToolTip::showText(QCursor::pos(), downloadItem->toolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS));
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Downloads/RemoveFinished", false).toBool() )
			QTimer::singleShot(QMC2_DOWNLOAD_CLEANUP_DELAY, qmc2MainWindow, SLOT(on_pushButtonClearFinishedDownloads_clicked()));
	}
	progressWidget->setEnabled(false);

	// close connection and file
	networkReply->close();
	if ( localFile.isOpen() )
		localFile.close();
}

void ItemDownloader::managerFinished(QNetworkReply *reply)
{
	if ( reply == networkReply )
		finished();
}

void ItemDownloader::reload()
{
	networkReply->close();
	errorCheckTimer.stop();
	networkReply = qmc2NetworkAccessManager->get(QNetworkRequest(networkReply->url()));
	init();
}

