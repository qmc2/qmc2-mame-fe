#include <archive_entry.h>
#include "archivefile.h"

ArchiveFile::ArchiveFile(QString fileName, bool sequential, QObject *parent)
	: QObject(parent)
{
	m_fileName = fileName;
	m_sequential = sequential;
	m_archive = 0;
	m_entry = 0;
}

ArchiveFile::~ArchiveFile()
{
	if ( isOpen() )
		close();
}

bool ArchiveFile::open(QString fileName)
{
	if ( isOpen() )
		close();
	if ( !fileName.isEmpty() )
		m_fileName = fileName;
	m_archive = archive_read_new();
	archive_read_support_filter_all(m_archive);
	archive_read_support_format_all(m_archive);
	m_entry = 0;
	int result = archive_read_open_filename(m_archive, m_fileName.toUtf8().constData(), QMC2_ARCHIVE_BLOCK_SIZE);
	if ( result != ARCHIVE_OK) {
		archive_read_free(m_archive);
		m_archive = 0;
		return false;
	} else {
		if ( !m_sequential )
			createEntryList();
		return true;
	}
}

void ArchiveFile::close()
{
	entryList().clear();
	m_nameToIndexCache.clear();
	if ( isOpen() ) {
		archive_read_free(m_archive);
		m_archive = 0;
		m_entry = 0;
	}
}

bool ArchiveFile::seekNextEntry(ArchiveEntryMetaData *metaData, bool *reset)
{
	if ( !isOpen() )
		return false;
	if ( !m_sequential ) {
		if ( reset != 0 && *reset ) {
			archive_read_free(m_archive);
			m_archive = archive_read_new();
			archive_read_support_filter_all(m_archive);
			archive_read_support_format_all(m_archive);
			archive_read_open_filename(m_archive, m_fileName.toUtf8().constData(), QMC2_ARCHIVE_BLOCK_SIZE);
			*reset = false;
		}
	}
	if ( archive_read_next_header(m_archive, &m_entry) == ARCHIVE_OK ) {
		*metaData = ArchiveEntryMetaData(archive_entry_pathname(m_entry), archive_entry_size(m_entry), QDateTime::fromTime_t(archive_entry_mtime(m_entry)));
		return true;
	} else
		return false;
}

bool ArchiveFile::seekEntry(uint index)
{
	if ( !isOpen() )
		return false;
	ArchiveEntryMetaData metadata = entryList()[index];
	archive_read_free(m_archive);
	m_archive = archive_read_new();
	archive_read_support_filter_all(m_archive);
	archive_read_support_format_all(m_archive);
	archive_read_open_filename(m_archive, m_fileName.toUtf8().constData(), QMC2_ARCHIVE_BLOCK_SIZE);
	while ( archive_read_next_header(m_archive, &m_entry) == ARCHIVE_OK )
		if ( metadata.name().compare(archive_entry_pathname(m_entry), Qt::CaseSensitive) == 0 )
			return true;
	return false;
}

qint64 ArchiveFile::readEntry(QByteArray &buffer)
{
	if ( !isOpen() )
		return 0;
#if defined(QMC2_OS_WIN)
	__int64 size = archive_entry_size(m_entry);
#else
	int64_t size = archive_entry_size(m_entry);
#endif
	if ( size > 0 ) {
		char *data = new char[size];
		ssize_t len = archive_read_data(m_archive, data, size);
		if ( len > 0 ) {
			buffer = QByteArray(data, len);
			delete [] data;
			return len;
		}
		delete [] data;
	}
	return 0;
}

void ArchiveFile::createEntryList()
{
	entryList().clear();
	m_nameToIndexCache.clear();
	if ( !isOpen() )
		return;
	struct archive_entry *entry;
	int counter = 0;
	while ( archive_read_next_header(m_archive, &entry) == ARCHIVE_OK ) {
		QString entryName(archive_entry_pathname(entry));
		entryList() << ArchiveEntryMetaData(entryName, archive_entry_size(entry), QDateTime::fromTime_t(archive_entry_mtime(entry)));
		m_nameToIndexCache[entryName] = counter++;
		archive_read_data_skip(m_archive);
	}
}
