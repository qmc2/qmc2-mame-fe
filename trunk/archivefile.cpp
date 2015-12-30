#include "archivefile.h"

ArchiveFile::ArchiveFile(QString fileName, bool sequential, bool deflate, QObject *parent)
	: QObject(parent)
{
	m_fileName = fileName;
	m_sequential = sequential;
	m_deflate = deflate;
	m_archive = 0;
	m_entry = 0;
	m_openMode = QIODevice::ReadOnly;
}

ArchiveFile::~ArchiveFile()
{
	if ( isOpen() )
		close();
}

bool ArchiveFile::open(QIODevice::OpenMode openMode, QString fileName)
{
	if ( isOpen() )
		close();
	if ( !fileName.isEmpty() )
		m_fileName = fileName;
	m_openMode = openMode;
	m_entry = 0;
	int result = 0;
	switch ( m_openMode ) {
		case QIODevice::WriteOnly:
			m_archive = archive_write_new();
			archive_write_set_format_zip(m_archive);
			if ( m_deflate )
				archive_write_set_options(m_archive, "compression=deflate");
			else
				archive_write_set_options(m_archive, "compression=store");
			result = archive_write_open_filename(m_archive, m_fileName.toUtf8().constData());
			if ( result != ARCHIVE_OK) {
				archive_write_free(m_archive);
				m_archive = 0;
				return false;
			} else
				return true;
		case QIODevice::ReadOnly:
		default:
			m_archive = archive_read_new();
			archive_read_support_filter_all(m_archive);
			archive_read_support_format_all(m_archive);
			result = archive_read_open_filename(m_archive, m_fileName.toUtf8().constData(), QMC2_ARCHIVE_BLOCK_SIZE);
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
}

void ArchiveFile::close()
{
	entryList().clear();
	m_nameToIndexCache.clear();
	if ( isOpen() ) {
		switch ( m_openMode ) {
			case QIODevice::WriteOnly:
				archive_write_close(m_archive);
				archive_write_free(m_archive);
				break;
			case QIODevice::ReadOnly:
			default:
				archive_read_close(m_archive);
				archive_read_free(m_archive);
				break;
		}
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
	if ( !isOpen() || writeMode() )
		return 0;
#if defined(QMC2_OS_WIN)
	__int64 size = archive_entry_size(m_entry);
#else
	int64_t size = archive_entry_size(m_entry);
#endif
	if ( size > 0 ) {
		char *data = new char[size];
#if defined(QMC2_OS_WIN) && defined(_MSC_VER)
		SSIZE_T len = archive_read_data(m_archive, data, size);
#else
		ssize_t len = archive_read_data(m_archive, data, size);
#endif
		if ( len > 0 ) {
			buffer = QByteArray(data, len);
			delete [] data;
			return len;
		}
		delete [] data;
	}
	return 0;
}

bool ArchiveFile::createEntry(QString name, size_t size)
{
	m_entry = archive_entry_new();
	archive_entry_set_pathname(m_entry, name.toUtf8().constData());
	archive_entry_set_size(m_entry, size);
	archive_entry_set_filetype(m_entry, AE_IFREG);
	archive_entry_set_perm(m_entry, 0644);
	archive_entry_set_mtime(m_entry, QDateTime::currentDateTime().toTime_t(), 0);
	archive_write_header(m_archive, m_entry);
	return !hasError();
}

void ArchiveFile::closeEntry()
{
	archive_write_finish_entry(m_archive);
	archive_entry_free(m_entry);
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
