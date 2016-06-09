#ifndef SEVENZIPFILE_H
#define SEVENZIPFILE_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QHash>
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>

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

class SevenZipExtractorThread : public QThread
{
	Q_OBJECT

	public:
		explicit SevenZipExtractorThread(QObject *parent = 0);
		~SevenZipExtractorThread();

		bool quitFlag() { return m_quitFlag; }
		void setQuitFlag(bool flag) { m_quitFlag = flag; }
		bool isActive() { return m_active; }
		int fileCount() { return m_fileCount; }
		void setParams(CSzArEx *db, ILookInStream *lookInStream, uint fileIndex, UInt32_7z *blockIndex, Byte **buffer, size_t *bufferSize, size_t *offset, size_t *sizeProcessed, ISzAlloc *allocImp, ISzAlloc *allocTempImp);
		SRes result() { return m_result; }
		QMutex &waitMutex() { return m_waitMutex; }
		QWaitCondition &waitCondition() { return m_waitCondition; }

	public slots:

	protected:
		void run();

	signals:
		void extracted(uint);
		void failed(uint);

	private:
		bool m_quitFlag;
		bool m_active;
		int m_fileCount;
		CSzArEx *m_db;
		ILookInStream *m_lookInStream;
		uint m_fileIndex;
		UInt32_7z *m_blockIndex;
		Byte **m_buffer;
		size_t *m_bufferSize;
		size_t *m_offset;
		size_t *m_sizeProcessed;
		ISzAlloc *m_allocImp;
		ISzAlloc *m_allocTempImp;
		SRes m_result;
		QMutex m_waitMutex;
		QWaitCondition m_waitCondition;
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
		bool isFillingDictionary() { return m_fillingDictionary; }
		QList<SevenZipMetaData> &entryList() { return m_entryList; }
		quint64 read(QString name, QByteArray *buffer);
		quint64 read(uint index, QByteArray *buffer, bool *async = 0);
		int indexOfName(QString name) { if ( m_nameToIndexCache.contains(name) ) return m_nameToIndexCache[name]; else return -1; }
		int indexOfCrc(QString crc) { if ( m_crcToIndexCache.contains(crc) ) return m_crcToIndexCache[crc]; else return -1; }
		QString userData() { return m_userData; }
		void setUserData(QString data) { m_userData = data; }

	signals:
		void opened();
		void closed();
		void error(QString);
		void dataReady();

	public slots:
		bool open(QString fileName = QString());
		void close();

	private slots:
		void asyncExtractionFinished(uint index);

	private:
		QDateTime convertFileTime(const CNtfsFileTime *ft);
		void createEntryList();
		QString errorCodeToString(SRes errorCode);
		CSzArEx *db() { return &m_db; }

		CSzArEx m_db;
		ISzAlloc m_allocImp;
		ISzAlloc m_allocTempImp;
		CFileInStream m_archiveStream;
		CLookToRead m_lookStream;
		UInt32_7z m_blockIndex;
		size_t m_bufferSize;
		size_t m_sizeProcessed;
		Byte *m_buffer;
		QMap<uint, size_t> m_offsetMap;
		QList<SevenZipMetaData> m_entryList;
		QString m_fileName;
		QString m_lastError;
		SevenZipExtractorThread *m_extractor;
		bool m_isOpen;
		bool m_firstExtraction;
		bool m_fillingDictionary;
		QString m_userData;
		QHash<QString, int> m_nameToIndexCache;
		QHash<QString, int> m_crcToIndexCache;
};

#endif
