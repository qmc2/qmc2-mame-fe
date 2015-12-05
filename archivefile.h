#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QByteArray>

#include <archive.h>

class ArchiveItemMetaData
{
	public:
		explicit ArchiveItemMetaData(QString name = QString(), quint64 size = 0, QDateTime date = QDateTime(), QString crc = QString())
		{
			setName(name);
			setSize(size);
			setDate(date);
			setCrc(crc);
		}

		void setName(QString name) { m_name = name; }
		QString name() { return m_name; }
		void setSize(quint64 size) { m_size = size; }
		quint64 size() { return m_size; }
		void setDate(QDateTime date) { m_date = date; }
		QDateTime date() { return m_date; }
		void setCrc(QString crc) { m_crc = crc; }
		QString crc() { return m_crc; }

	private:
		QString m_name;
		quint64 m_size;
		QDateTime m_date;
		QString m_crc;
};

class ArchiveFile : public QObject
{
	Q_OBJECT

	public:
		explicit ArchiveFile(QString fileName = QString(), QObject *parent = 0);

		QString fileName() { return m_fileName; }
		QString lastError() { return m_lastError; }
		QList<ArchiveItemMetaData> &itemList() { return m_itemList; }
		bool hasError() { return !m_lastError.isEmpty(); }
		bool isOpen() { return m_isOpen; }
		bool open(QString fileName = QString());
		void close();
		bool openItem(QString name);
		bool openItem(uint index);
		void closeItem();
		int indexOfName(QString name);
		int indexOfCrc(QString crc);
		quint64 readItem(uint len, QByteArray *buffer);

	signals:
		void opened();
		void closed();
		void itemOpened(uint);
		void itemClosed(uint);
		void error(QString);

	private:
		void createItemList();
		QString errorCodeToString(int errorCode);

		QList<ArchiveItemMetaData> m_itemList;
		QString m_fileName;
		QString m_lastError;
		bool m_isOpen;
};

#endif
