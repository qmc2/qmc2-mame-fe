#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QByteArray>

#include <archive.h>

#define QMC2_ARCHIVE_BLOCK_SIZE	65536

class ArchiveEntryMetaData
{
public:
	explicit ArchiveEntryMetaData(QString name = QString(), quint64 size = 0, QDateTime date = QDateTime())
	{
		setName(name);
		setSize(size);
		setDate(date);
	}

	void setName(QString name) { m_name = name; }
	QString name() { return m_name; }
	void setSize(quint64 size) { m_size = size; }
	quint64 size() { return m_size; }
	void setDate(QDateTime date) { m_date = date; }
	QDateTime date() { return m_date; }

private:
	QString m_name;
	quint64 m_size;
	QDateTime m_date;
};

class ArchiveFile : public QObject
{
	Q_OBJECT

public:
	explicit ArchiveFile(QString fileName = QString(), QObject *parent = 0);
	~ArchiveFile();

	QString fileName() { return m_fileName; }
	QList<ArchiveEntryMetaData> &entryList() { return m_entryList; }
	bool isOpen() { return m_fd >= 0; }
	bool open(QString fileName = QString());
	void close();
	bool seekNextEntry(ArchiveEntryMetaData *metaData, bool *reset);
	bool seekEntry(QString name);
	bool seekEntry(uint index);
	qint64 readBlock(QByteArray *buffer);
	QString errorString();
	int errorCode();

private:
	void createItemList();
	int indexOfName(QString name);

	struct archive *m_archive;
	int m_fd;
	const void *m_buffer;
	QFile m_file;
	QList<ArchiveEntryMetaData> m_entryList;
	QString m_fileName;
};

#endif
