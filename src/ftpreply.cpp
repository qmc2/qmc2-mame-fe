#include <QTextDocument>
#include <QtNetwork>

#include "macros.h"
#include "romalyzer.h"
#include "ftpreply.h"

FtpReply::FtpReply(const QUrl &url)
	: QNetworkReply()
{
	ftp = new QFtp(this);
	connect(ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(processListInfo(QUrlInfo)));
	connect(ftp, SIGNAL(readyRead()), this, SLOT(processData()));
	connect(ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(processCommand(int, bool)));
	offset = 0;
	setUrl(url);
	ftp->connectToHost(url.host());
}

void FtpReply::processCommand(int, bool err)
{
	if ( err ) {
		setError(ContentNotFoundError, "Unknown command");
		emit error(ContentNotFoundError);
		return;
	}

	switch ( ftp->currentCommand() ) {
		case QFtp::ConnectToHost:
			ftp->login();
			break;

		case QFtp::Login:
			ftp->list(url().path());
			break;

		case QFtp::List:
			if ( items.size() == 1 )
				ftp->get(url().path());
			else
				setListContent();
			break;

		case QFtp::Get:
			setContent();

		default:
			break;
	}
}

void FtpReply::processListInfo(const QUrlInfo &urlInfo)
{
	items.append(urlInfo);

	QUrl u = url();
	if ( !u.path().endsWith("/") )
		u.setPath(u.path() + "/");
	QUrl fileUrl = u.resolved(QUrl(urlInfo.name()));
	fileSizeMap[fileUrl.toString()] = urlInfo.size();
}

void FtpReply::processData()
{
	content += ftp->readAll();
	emit readyRead();
}

void FtpReply::setContent()
{
	setOperation(QNetworkAccessManager::GetOperation);
	open(ReadOnly | Unbuffered);
	setHeader(QNetworkRequest::ContentLengthHeader, QVariant(content.size()));
	emit readyRead();
	emit finished();
	ftp->close();
}

void FtpReply::setListContent()
{
	QUrl u = url();
	if ( !u.path().endsWith("/") )
		u.setPath(u.path() + "/");

	QString base_url = url().toString();
	QString base_path = u.path();

	open(ReadOnly | Unbuffered);

	QString content("<html><head>\n"
#if QT_VERSION < 0x050000
			"<title>" + Qt::escape(base_url) + "</title>\n"
#else
			"<title>" + QString(base_url).toHtmlEscaped() + "</title>\n"
#endif
			"<style type=\"text/css\">\n"
			"th { background-color: #aaaaaa; color: black; }\n"
			"table { border: solid 1px #aaaaaa; }\n"
			"tr.odd { background-color: #dddddd; color: black; }\n"
			"tr.even { background-color: white; color: black; }\n"
			"</style>\n"
			"</head><body>\n"
			"<h1>" + tr("FTP directory listing for %1").arg(base_path) + "</h1>\n"
			"<table align=\"center\" cellspacing=\"0\" width=\"100%\">\n"
			"<tr><th align=\"left\">" + tr("Name") + "</th><th align=\"left\">" + tr("Type") + "</th><th align=\"left\">" + tr("Size") + "</th></tr>\n");

	QUrl parent = u.resolved(QUrl(".."));

	if ( parent.isParentOf(u) )
		content += QString("<tr><td><strong><a href=\"" + parent.toString() + "\">" + tr("Parent directory") + "</a></strong></td><td></td></tr>\n");

	int i = 0;
	foreach (const QUrlInfo &item, items) {
		if ( item.name() == "." || item.name() == ".." )
			continue;
		QUrl child = u.resolved(QUrl(item.name()));
		if ( i == 0 )
			content += QString("<tr class=\"odd\">");
		else
			content += QString("<tr class=\"even\">");
#if QT_VERSION < 0x050000
		content += QString("<td><a href=\"" + child.toString() + "\">" + Qt::escape(item.name()) + "</a></td>");
#else
		content += QString("<td><a href=\"" + child.toString() + "\">" + QString(item.name()).toHtmlEscaped() + "</a></td>");
#endif
		if ( item.isFile() )
			content += QString("<td>" + tr("File") + "</td><td>" + ROMAlyzer::humanReadable(item.size(), 2) + "</td></tr>\n");
		else if ( item.isDir() )
			content += QString("<td>" + tr("Folder") + "<td></td></tr>\n");
		else
			content += QString("<td>" + tr("Unknown") + "<td></td></tr>\n");
		i = 1 - i;
	}

	content += QString("</table>\n</body></html>\n");

	this->content = content.toUtf8();

	setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/html; charset=UTF-8"));
	setHeader(QNetworkRequest::ContentLengthHeader, QVariant(this->content.size()));
	emit readyRead();
	emit finished();
	ftp->close();
}

void FtpReply::abort()
{
	// NOP :)
}

qint64 FtpReply::bytesAvailable() const
{
	return content.size() - offset + QIODevice::bytesAvailable();
}

bool FtpReply::isSequential() const
{
	return true;
}

qint64 FtpReply::readData(char *data, qint64 maxSize)
{
	if ( offset < content.size() ) {
		qint64 number = qMin(maxSize, content.size() - offset);
		memcpy(data, content.constData() + offset, number);
		offset += number;
		return number;
	} else
		return -1;
}

qint64 FtpReply::totalSize(QString fileUrl)
{
	if ( fileSizeMap.isEmpty() )
		ftp->list(url().path());

	if ( fileSizeMap.contains(fileUrl) )
		return fileSizeMap[fileUrl];
	else
		return -1;
}
