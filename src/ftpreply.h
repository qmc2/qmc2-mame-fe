#ifndef FTPREPLY_H
#define FTPREPLY_H

#include <QNetworkReply>
#include <QStringList>
#include <QMap>

#include "qftp/qftp.h"
#include "qftp/qurlinfo.h"

class FtpReply : public QNetworkReply
{
	Q_OBJECT

	public:
		FtpReply(const QUrl &);
		void abort();
		qint64 bytesAvailable() const;
		bool isSequential() const;

		qint64 totalSize(QString);

	protected:
		qint64 readData(char *, qint64);

	private slots:
		void processCommand(int, bool);
		void processListInfo(const QUrlInfo &);
		void processData();

	private:
		void setContent();
		void setListContent();

		QFtp *ftp;
		QList<QUrlInfo> items;
		QByteArray content;
		qint64 offset;
		QMap<QString, qint64> fileSizeMap;
};    

#endif
