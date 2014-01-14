#include "sevenzipfile.h"

SevenZipFile::SevenZipFile(QString fileName, QObject *parent) :
    QObject(parent)
{
    m_fileName = fileName;
    m_isOpen = false;
    m_allocImp.Alloc = SzAlloc;
    m_allocImp.Free = SzFree;
    m_allocTempImp.Alloc = SzAllocTemp;
    m_allocTempImp.Free = SzFreeTemp;
    m_readBlockIndex = 0xFFFFFFFF;
    m_readBuffer = 0;
    m_readBufferSize = 0;
}

SevenZipFile::~SevenZipFile()
{
    IAlloc_Free(&m_allocImp, m_readBuffer);
}

quint64 SevenZipFile::read(QString name, QByteArray *buffer)
{
    m_lastError.clear();

    int index = indexOfFile(name);
    if ( index >= 0 )
        return read(index, buffer);
    else {
        m_lastError = tr("file name '%1' not found").arg(name);
        emit error(lastError());
        return 0;
    }
}

quint64 SevenZipFile::read(uint index, QByteArray *buffer)
{
    m_lastError.clear();

    if ( !isOpen() ) {
        m_lastError = tr("archive not open");
        emit error(lastError());
        return 0;
    }

    if ( !buffer ) {
        m_lastError = tr("null-buffer not allowed");
        emit error(lastError());
        return 0;
    }

    buffer->clear();
    size_t sizeProcessed = 0;
    SRes result = SzArEx_Extract(db(), &m_lookStream.s, index, &m_readBlockIndex, &m_readBuffer, &m_readBufferSize, &m_readOffsetMap[index], &sizeProcessed, &m_allocImp, &m_allocTempImp);
    if ( result == SZ_OK )
        buffer->setRawData((const char *)(m_readBuffer + m_readOffsetMap[index]), sizeProcessed);
    else {
        m_lastError = errorCodeToString(result);
        emit error(lastError());
        sizeProcessed = -1;
    }
    return sizeProcessed;
}

quint64 SevenZipFile::write(QString name, QByteArray *buffer)
{
    m_lastError.clear();

    int index = indexOfFile(name);
    if ( index >= 0 )
        return write(index, buffer);
    else {
        m_lastError = tr("file name '%1' not found").arg(name);
        emit error(lastError());
        return 0;
    }
}

quint64 SevenZipFile::write(uint /*index*/, QByteArray *buffer)
{
    m_lastError.clear();

    if ( !isOpen() ) {
        m_lastError = tr("archive not open");
        emit error(lastError());
        return 0;
    }

    if ( !buffer ) {
        m_lastError = tr("null-buffer not allowed");
        emit error(lastError());
        return 0;
    }

    // FIXME
    m_lastError = tr("7z encoding not supported yet");
    emit error(lastError());
    return 0;
}

QString SevenZipFile::errorCodeToString(SRes errorCode)
{
    switch ( errorCode ) {
    case SZ_OK:
        return tr("no error");
    case SZ_ERROR_DATA:
        return tr("incorrect data");
    case SZ_ERROR_MEM:
        return tr("out of memory");
    case SZ_ERROR_CRC:
        return tr("incorrect CRC");
    case SZ_ERROR_UNSUPPORTED:
        return tr("unsupported compression");
    case SZ_ERROR_PARAM:
        return tr("incorrect properties");
    case SZ_ERROR_INPUT_EOF:
        return tr("premature end-of-file (input)");
    case SZ_ERROR_OUTPUT_EOF:
        return tr("premature end-of-file (output)");
    case SZ_ERROR_READ:
        return tr("failed reading");
    case SZ_ERROR_WRITE:
        return tr("failed writing");
    case SZ_ERROR_PROGRESS:
        return tr("failed signalling progress");
    case SZ_ERROR_FAIL:
        return tr("fatal error");
    case SZ_ERROR_THREAD:
        return tr("thread error");
    case SZ_ERROR_ARCHIVE:
        return tr("invalid archive structure");
    case SZ_ERROR_NO_ARCHIVE:
        return tr("invalid header structure");
    default:
        return tr("unknown error");
    }
}

bool SevenZipFile::open(QString fileName)
{
    m_lastError.clear();

    if ( isOpen() )
        close();

    if ( !fileName.isEmpty() )
        m_fileName = fileName;

    if ( m_fileName.isEmpty() ) {
        m_lastError = tr("no file name specified");
        emit error(lastError());
        return false;
    }

    if ( InFile_Open(&m_archiveStream.file, m_fileName.toLocal8Bit().constData()) )
    {
        m_lastError = tr("can't open archive '%1'").arg(m_fileName);
        emit error(lastError());
        return false;
    }

    FileInStream_CreateVTable(&m_archiveStream);
    LookToRead_CreateVTable(&m_lookStream, false);
    m_lookStream.realStream = &m_archiveStream.s;
    LookToRead_Init(&m_lookStream);
    CrcGenerateTable();
    SzArEx_Init(db());

    SRes result = SzArEx_Open(db(), &m_lookStream.s, &m_allocImp, &m_allocTempImp);

    if ( result == SZ_OK ) {
        m_isOpen = true;
        createItemList();
        emit opened();
        return true;
    } else {
        m_lastError = tr("can't open archive '%1'").arg(m_fileName) + ", " + errorCodeToString(result);
        emit error(lastError());
        return false;
    }
}

void SevenZipFile::close()
{
    m_lastError.clear();

    if ( isOpen() ) {
        SzArEx_Free(db(), &m_allocImp);
        File_Close(&m_archiveStream.file);
        m_isOpen = false;
        emit closed();
    }
}

#define QMC2_SEVENZIP_PERIOD_4       (4 * 365 + 1)
#define QMC2_SEVENZIP_PERIOD_100     (QMC2_SEVENZIP_PERIOD_4 * 25 - 1)
#define QMC2_SEVENZIP_PERIOD_400     (QMC2_SEVENZIP_PERIOD_100 * 4 + 1)

QDateTime SevenZipFile::convertFileTime(const CNtfsFileTime *ft)
{
    unsigned year, mon, day, hour, min, sec;
    UInt64 v64 = (ft->Low | ((UInt64)ft->High << 32)) / 10000000;
    Byte ms[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    unsigned t;
    UInt32 v;
    sec = (unsigned)(v64 % 60);
    v64 /= 60;
    min = (unsigned)(v64 % 60);
    v64 /= 60;
    hour = (unsigned)(v64 % 24);
    v64 /= 24;
    v = (UInt32)v64;
    year = (unsigned)(1601 + v / QMC2_SEVENZIP_PERIOD_400 * 400);
    v %= QMC2_SEVENZIP_PERIOD_400;
    t = v / QMC2_SEVENZIP_PERIOD_100;
    if ( t == 4 )
        t = 3;
    year += t * 100;
    v -= t * QMC2_SEVENZIP_PERIOD_100;
    t = v / QMC2_SEVENZIP_PERIOD_4;
    if ( t == 25 )
        t = 24;
    year += t * 4;
    v -= t * QMC2_SEVENZIP_PERIOD_4;
    t = v / 365;
    if ( t == 4 )
        t =  3;
    year += t;
    v -= t * 365;
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
      ms[1] = 29;
    for (mon = 1; mon <= 12; mon++) {
        unsigned s = ms[mon - 1];
        if (v < s)
            break;
        v -= s;
    }
    day = (unsigned)v + 1;
    QDateTime dateTime;
    dateTime.setDate(QDate(year, mon, day));
    dateTime.setTime(QTime(hour, min, sec));
    return dateTime;
}

void SevenZipFile::createItemList()
{
    itemList().clear();

    if ( !isOpen() )
        return;

    for (uint i = 0; i < db()->db.NumFiles; i++)
    {
        const CSzFileItem *fileItem = db()->db.Files + i;
        if ( fileItem->IsDir )
            continue;
        int fileItemLength = SzArEx_GetFileNameUtf16(db(), i, NULL);
        UInt16 *tempFileName = (UInt16 *)SzAlloc(NULL, fileItemLength * sizeof(UInt16));
        SzArEx_GetFileNameUtf16(db(), i, tempFileName);
        QString fileItemName = QString::fromUtf16(tempFileName, fileItemLength - 1);
        SzFree(NULL, tempFileName);
        QDateTime dateTime;
        if ( fileItem->MTimeDefined )
            dateTime = convertFileTime(&fileItem->MTime);
        QString crc = "00000000";
        if ( fileItem->CrcDefined )
            crc = QString::number(fileItem->Crc, 16).rightJustified(8, '0');
        itemList() << SevenZipMetaData(fileItemName, fileItem->Size, dateTime, crc);
    }
}

int SevenZipFile::indexOfFile(QString name)
{
    for (int i = 0; i < itemList().count(); i++)
        if ( itemList()[i].name() == name )
            return i;
    return -1;
}
