#include <archive_entry.h>

#include "archivefile.h"

ArchiveFile::ArchiveFile(QString fileName, QObject *parent)
	: QObject(parent)
{
	m_fileName = fileName;
	m_isOpen = false;
}

bool ArchiveFile::open(QString fileName)
{
	// FIXME
	return false;
}

void ArchiveFile::close()
{
	// FIXME
}

bool ArchiveFile::openItem(QString name)
{
	// FIXME
	return false;
}

bool ArchiveFile::openItem(uint index)
{
	// FIXME
	return false;
}

void ArchiveFile::closeItem()
{
	// FIXME
}

int ArchiveFile::indexOfName(QString name)
{
	// FIXME
	return -1;
}

int ArchiveFile::indexOfCrc(QString crc)
{
	// FIXME
	return -1;
}

quint64 ArchiveFile::readItem(uint len, QByteArray *buffer)
{
	// FIXME
	return 0;
}

void ArchiveFile::createItemList()
{
	itemList().clear();
	if ( !isOpen() )
		return;
	// FIXME
}

QString ArchiveFile::errorCodeToString(int errorCode)
{
	switch ( errorCode ) {
		case ARCHIVE_EOF:
			return tr("end of file");
		case ARCHIVE_OK:
			return tr("operation completed successfully");
		case ARCHIVE_WARN:
			return tr("operation completed with warnings");
		case ARCHIVE_FAILED:
			return tr("operation failed");
		case ARCHIVE_FATAL:
			return tr("fatal error");
		default:
			return tr("unknown error");
	}
}
