#ifndef SEVENZIPFILE_H
#define SEVENZIPFILE_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QString>
#include <QDateTime>
#include <QByteArray>

extern "C" {
#include "lzma/7z.h"
#include "lzma/7zAlloc.h"
#include "lzma/7zCrc.h"
#include "lzma/7zFile.h"
}

class SevenZipMetaData
{
public:
    explicit SevenZipMetaData(QString name = QString(), quint64 size = 0, QDateTime date = QDateTime(), QString crc = QString())
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

class SevenZipFile : public QObject
{
    Q_OBJECT

public:
    explicit SevenZipFile(QString fileName = QString(), QObject *parent = 0);
    ~SevenZipFile();

    QString fileName() { return m_fileName; }
    QString lastError() { return m_lastError; }
    bool hasError() { return !m_lastError.isEmpty(); }
    bool isOpen() { return m_isOpen; }
    QList<SevenZipMetaData> &itemList() { return m_itemList; }
    quint64 read(QString name, QByteArray *buffer);
    quint64 read(uint index, QByteArray *buffer);
    quint64 write(QString name, QByteArray *buffer);
    quint64 write(uint index, QByteArray *buffer);

signals:
    void opened();
    void closed();
    void error(QString);

public slots:
    bool open(QString fileName = QString());
    void close();

private:
    QDateTime convertFileTime(const CNtfsFileTime *ft);
    void createItemList();
    int indexOfFile(QString);
    QString errorCodeToString(SRes errorCode);
    CSzArEx *db() { return &m_db; }

    CSzArEx m_db;
    ISzAlloc m_allocImp;
    ISzAlloc m_allocTempImp;
    CFileInStream m_archiveStream;
    CLookToRead m_lookStream;
    UInt32 m_readBlockIndex;
    size_t m_readBufferSize;
    Byte *m_readBuffer;
    QMap<uint, size_t> m_readOffsetMap;
    QList<SevenZipMetaData> m_itemList;
    QString m_fileName;
    QString m_lastError;
    bool m_isOpen;
};

#endif // SEVENZIPFILE_H
