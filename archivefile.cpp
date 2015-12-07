#include <archive_entry.h>

#include "archivefile.h"

ArchiveFile::ArchiveFile(QString fileName, QObject *parent)
    : QObject(parent)
{
    m_fileName = fileName;
    m_archive = 0;
    m_fd = -1;
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
    m_file.setFileName(m_fileName);
    if ( !m_file.open(QIODevice::ReadOnly) )
        return false;
    m_fd = m_file.handle();
    m_archive = archive_read_new();
    archive_read_support_filter_all(m_archive);
    archive_read_support_format_all(m_archive);
    int result = archive_read_open_fd(m_archive, m_fd, QMC2_ARCHIVE_BLOCK_SIZE);
    if ( result != ARCHIVE_OK) {
        archive_read_free(m_archive);
        m_archive = 0;
        m_file.close();
        m_fd = -1;
        return false;
    } else {
        createItemList();
        return true;
    }
}

void ArchiveFile::close()
{
    m_entryList.clear();
    if ( isOpen() ) {
        archive_read_free(m_archive);
        m_file.close();
    }
    m_archive = 0;
    m_fd = -1;
}

bool ArchiveFile::seekNextEntry(ArchiveEntryMetaData *metaData, bool *reset)
{
    if ( !isOpen() )
        return false;
    if ( *reset ) {
        archive_read_free(m_archive);
        m_file.close();
        m_file.open(QIODevice::ReadOnly);
        m_fd = m_file.handle();
        m_archive = archive_read_new();
        archive_read_support_filter_all(m_archive);
        archive_read_support_format_all(m_archive);
        archive_read_open_fd(m_archive, m_fd, QMC2_ARCHIVE_BLOCK_SIZE);
        *reset = false;
    }
    struct archive_entry *entry;
    if ( archive_read_next_header(m_archive, &entry) == ARCHIVE_OK ) {
        *metaData = ArchiveEntryMetaData(archive_entry_pathname(entry), archive_entry_size(entry), QDateTime::fromTime_t(archive_entry_mtime(entry)));
        return true;
    } else
        return false;
}

bool ArchiveFile::seekEntry(QString name)
{
    return seekEntry(indexOfName(name));
}

bool ArchiveFile::seekEntry(uint index)
{
    if ( !isOpen() )
        return false;
    struct archive_entry *entry;
    ArchiveEntryMetaData metadata = entryList()[index];
    archive_read_free(m_archive);
    lseek(m_fd, 0, SEEK_SET);
    m_archive = archive_read_new();
    archive_read_support_filter_all(m_archive);
    archive_read_support_format_all(m_archive);
    archive_read_open_fd(m_archive, m_fd, QMC2_ARCHIVE_BLOCK_SIZE);
    while ( archive_read_next_header(m_archive, &entry) == ARCHIVE_OK )
        if ( metadata.name().compare(archive_entry_pathname(entry), Qt::CaseSensitive) == 0 )
            return true;
    return false;
}

qint64 ArchiveFile::readBlock(QByteArray *buffer)
{
    if ( !isOpen() )
        return 0;
    off_t offset;
    size_t size;
    int result = archive_read_data_block(m_archive, &m_buffer, &size, &offset);
    if ( result == ARCHIVE_EOF || result == ARCHIVE_OK ) {
        buffer->setRawData((const char *)m_buffer, size);
        if ( result == ARCHIVE_EOF )
            return -1;
        else
            return size;
    } else
        return -1;
}

QString ArchiveFile::errorString()
{
    if ( isOpen() )
        return QString(archive_error_string(m_archive));
    else
        return QString();
}

int ArchiveFile::errorCode()
{
    if ( isOpen() )
        return archive_errno(m_archive);
    else
        return ARCHIVE_OK;
}

void ArchiveFile::createItemList()
{
    entryList().clear();
    if ( !isOpen() )
        return;
    struct archive_entry *entry;
    while ( archive_read_next_header(m_archive, &entry) == ARCHIVE_OK ) {
        entryList() << ArchiveEntryMetaData(archive_entry_pathname(entry), archive_entry_size(entry), QDateTime::fromTime_t(archive_entry_mtime(entry)));
        archive_read_data_skip(m_archive);
    }
}

int ArchiveFile::indexOfName(QString name)
{
    for (int i = 0; i < entryList().count(); i++)
            if ( entryList()[i].name() == name )
                    return i;
    return -1;
}
