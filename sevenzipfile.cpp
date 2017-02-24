#include <QCoreApplication>
#include <QTest>

#include "sevenzipfile.h"

SevenZipFile::SevenZipFile(QString fileName, QObject *parent) :
	QObject(parent),
	m_blockIndex(0xFFFFFFFF),
	m_bufferSize(0),
	m_sizeProcessed(0),
	m_buffer(0),
	m_fileName(fileName),
	m_extractor(0),
	m_isOpen(false),
	m_firstExtraction(true),
	m_fillingDictionary(false)
{
	m_allocImp.Alloc = SzAlloc;
	m_allocImp.Free = SzFree;
	m_allocTempImp.Alloc = SzAllocTemp;
	m_allocTempImp.Free = SzFreeTemp;
}

SevenZipFile::~SevenZipFile()
{
	IAlloc_Free(&m_allocImp, m_buffer);
}

quint64 SevenZipFile::read(QString name, QByteArray *buffer)
{
	m_lastError.clear();
	int index = indexOfName(name);
	if ( index >= 0 )
		return read(index, buffer);
	else {
		m_lastError = tr("file name '%1' not found").arg(name);
		emit error(lastError());
		return 0;
	}
}

quint64 SevenZipFile::read(uint index, QByteArray *buffer, bool *async)
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
	if ( m_firstExtraction ) {
		// do the first extraction in a separate thread because 7z decompresses the *complete* LZMA stream at once (and returns pointers to already decompressed data on subsequent extractions)
		if ( !m_extractor ) {
			m_extractor = new SevenZipExtractorThread(this);
			connect(m_extractor, SIGNAL(extracted(uint)), this, SLOT(asyncExtractionFinished(uint)));
			connect(m_extractor, SIGNAL(failed(uint)), this, SLOT(asyncExtractionFinished(uint)));
			m_extractor->setParams(db(), &m_lookStream.s, index, &m_blockIndex, &m_buffer, &m_bufferSize, &m_offsetMap[index], &m_sizeProcessed, &m_allocImp, &m_allocTempImp);
		}
		if ( async && *async ) {
			if ( m_extractor->isActive() )
				return 0;
			if ( m_extractor->fileCount() == 0 ) {
				buffer->clear();
				m_sizeProcessed = 0;
				m_fillingDictionary = true;
				m_extractor->waitCondition().wakeAll();
			}
			if ( isFillingDictionary() )
				return 0;
			if ( m_extractor->result() == SZ_OK )
				buffer->setRawData((const char *)(m_buffer + m_offsetMap[index]), m_sizeProcessed);
			else {
				m_lastError = tr("extraction of file '%1' (index %2) failed").arg(entryList()[index].name()).arg(index) +  " - " + errorCodeToString(m_extractor->result());
				emit error(lastError());
				m_sizeProcessed = 0;
			}
			delete m_extractor;
			m_extractor = 0;
			m_firstExtraction = false;
			*async = false;
			return m_sizeProcessed;
		} else {
			m_fillingDictionary = true;
			buffer->clear();
			m_sizeProcessed = 0;
			while ( isFillingDictionary() ) {
				m_extractor->waitCondition().wakeAll();
				QTest::qWait(25);
				qApp->processEvents();
			}
			if ( m_extractor->result() == SZ_OK )
				buffer->setRawData((const char *)(m_buffer + m_offsetMap[index]), m_sizeProcessed);
			else {
				m_lastError = tr("extraction of file '%1' (index %2) failed").arg(entryList()[index].name()).arg(index) +  " - " + errorCodeToString(m_extractor->result());
				emit error(lastError());
				m_sizeProcessed = 0;
			}
			delete m_extractor;
			m_extractor = 0;
			m_firstExtraction = false;
			if ( async )
				*async = false;
			return m_sizeProcessed;
		}
	} else {
		buffer->clear();
		m_sizeProcessed = 0;
		SRes result = SzArEx_Extract(db(), &m_lookStream.s, index, &m_blockIndex, &m_buffer, &m_bufferSize, &m_offsetMap[index], &m_sizeProcessed, &m_allocImp, &m_allocTempImp);
		if ( result == SZ_OK )
			buffer->setRawData((const char *)(m_buffer + m_offsetMap[index]), m_sizeProcessed);
		else {
			m_lastError = tr("extraction of file '%1' (index %2) failed").arg(entryList()[index].name()).arg(index) +  " - " + errorCodeToString(result);
			emit error(lastError());
			m_sizeProcessed = 0;
		}
		if ( async )
			*async = false;
		return m_sizeProcessed;
	}
}

quint64 SevenZipFile::readBig(uint index, BigByteArray *buffer, bool *async)
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
	if ( m_firstExtraction ) {
		// do the first extraction in a separate thread because 7z decompresses the *complete* LZMA stream at once (and returns pointers to already decompressed data on subsequent extractions)
		if ( !m_extractor ) {
			m_extractor = new SevenZipExtractorThread(this);
			connect(m_extractor, SIGNAL(extracted(uint)), this, SLOT(asyncExtractionFinished(uint)));
			connect(m_extractor, SIGNAL(failed(uint)), this, SLOT(asyncExtractionFinished(uint)));
			m_extractor->setParams(db(), &m_lookStream.s, index, &m_blockIndex, &m_buffer, &m_bufferSize, &m_offsetMap[index], &m_sizeProcessed, &m_allocImp, &m_allocTempImp);
		}
		if ( async && *async ) {
			if ( m_extractor->isActive() )
				return 0;
			if ( m_extractor->fileCount() == 0 ) {
				buffer->clear();
				m_sizeProcessed = 0;
				m_fillingDictionary = true;
				m_extractor->waitCondition().wakeAll();
			}
			if ( isFillingDictionary() )
				return 0;
			if ( m_extractor->result() == SZ_OK )
				buffer->setRawData((const char *)(m_buffer + m_offsetMap[index]), m_sizeProcessed);
			else {
				m_lastError = tr("extraction of file '%1' (index %2) failed").arg(entryList()[index].name()).arg(index) +  " - " + errorCodeToString(m_extractor->result());
				emit error(lastError());
				m_sizeProcessed = 0;
			}
			delete m_extractor;
			m_extractor = 0;
			m_firstExtraction = false;
			*async = false;
			return m_sizeProcessed;
		} else {
			m_fillingDictionary = true;
			buffer->clear();
			m_sizeProcessed = 0;
			while ( isFillingDictionary() ) {
				m_extractor->waitCondition().wakeAll();
				QTest::qWait(25);
				qApp->processEvents();
			}
			if ( m_extractor->result() == SZ_OK )
				buffer->setRawData((const char *)(m_buffer + m_offsetMap[index]), m_sizeProcessed);
			else {
				m_lastError = tr("extraction of file '%1' (index %2) failed").arg(entryList()[index].name()).arg(index) +  " - " + errorCodeToString(m_extractor->result());
				emit error(lastError());
				m_sizeProcessed = 0;
			}
			delete m_extractor;
			m_extractor = 0;
			m_firstExtraction = false;
			if ( async )
				*async = false;
			return m_sizeProcessed;
		}
	} else {
		buffer->clear();
		m_sizeProcessed = 0;
		SRes result = SzArEx_Extract(db(), &m_lookStream.s, index, &m_blockIndex, &m_buffer, &m_bufferSize, &m_offsetMap[index], &m_sizeProcessed, &m_allocImp, &m_allocTempImp);
		if ( result == SZ_OK )
			buffer->setRawData((const char *)(m_buffer + m_offsetMap[index]), m_sizeProcessed);
		else {
			m_lastError = tr("extraction of file '%1' (index %2) failed").arg(entryList()[index].name()).arg(index) +  " - " + errorCodeToString(result);
			emit error(lastError());
			m_sizeProcessed = 0;
		}
		if ( async )
			*async = false;
		return m_sizeProcessed;
	}
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

	if ( InFile_Open(&m_archiveStream.file, m_fileName.toUtf8().constData()) ) {
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
		createEntryList();
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
		File_Close(&m_archiveStream.file);
		SzArEx_Free(db(), &m_allocImp);
		emit closed();
	}

	entryList().clear();
	m_nameToIndexCache.clear();
	m_crcToIndexCache.clear();
	m_isOpen = false;
	m_firstExtraction = true;
}

void SevenZipFile::asyncExtractionFinished(uint /*index*/)
{
	m_fillingDictionary = false;
	emit dataReady();
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
	UInt32_7z v;
	sec = (unsigned)(v64 % 60);
	v64 /= 60;
	min = (unsigned)(v64 % 60);
	v64 /= 60;
	hour = (unsigned)(v64 % 24);
	v64 /= 24;
	v = (UInt32_7z)v64;
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
		if ( v < s )
			break;
		v -= s;
	}
	day = (unsigned)v + 1;
	QDateTime dateTime;
	dateTime.setDate(QDate(year, mon, day));
	dateTime.setTime(QTime(hour, min, sec));
	return dateTime;
}

void SevenZipFile::createEntryList()
{
	entryList().clear();
	m_nameToIndexCache.clear();
	m_crcToIndexCache.clear();
	m_crcDuplicates.clear();
	if ( !isOpen() )
		return;
	for (uint i = 0; i < db()->NumFiles; i++)
	{
		if ( SzArEx_IsDir(db(), i) )
			continue;
		int fileItemLength = SzArEx_GetFileNameUtf16(db(), i, 0);
		UInt16 *tempFileName = (UInt16 *)SzAlloc(0, fileItemLength * sizeof(UInt16));
		SzArEx_GetFileNameUtf16(db(), i, tempFileName);
		QString fileItemName(QString::fromUtf16(tempFileName, fileItemLength - 1));
		m_nameToIndexCache.insert(fileItemName, i);
		SzFree(0, tempFileName);
		QDateTime dateTime;
		if ( SzBitWithVals_Check(&db()->MTime, i) )
			dateTime = convertFileTime(&db()->MTime.Vals[i]);
		QString crc("00000000");
		if ( SzBitWithVals_Check(&db()->CRCs, i) ) {
			crc = QString::number(db()->CRCs.Vals[i], 16).rightJustified(8, '0');
			m_crcToIndexCache.insert(crc, i);
			m_crcDuplicates.insert(crc, m_crcDuplicates.contains(crc));
		}
		entryList() << SevenZipMetaData(fileItemName, SzArEx_GetFileSize(db(), i), dateTime, crc);
	}
}

SevenZipExtractorThread::SevenZipExtractorThread(QObject *parent) :
	QThread(parent),
	m_quitFlag(false),
	m_active(false),
	m_fileCount(0),
	m_result(SZ_OK)
{
	start();
}

SevenZipExtractorThread::~SevenZipExtractorThread()
{
	setQuitFlag(true);
	waitCondition().wakeAll();
	wait();
	quit();
}

void SevenZipExtractorThread::setParams(CSzArEx *db, ILookInStream *lookInStream, uint fileIndex, UInt32_7z *blockIndex, Byte **buffer, size_t *bufferSize, size_t *offset, size_t *sizeProcessed, ISzAlloc *allocImp, ISzAlloc *allocTempImp)
{
	m_db = db;
	m_lookInStream = lookInStream;
	m_fileIndex = fileIndex;
	m_blockIndex = blockIndex;
	m_buffer = buffer;
	m_bufferSize = bufferSize;
	m_offset = offset;
	m_sizeProcessed = sizeProcessed;
	m_allocImp = allocImp;
	m_allocTempImp = allocTempImp;
}

void SevenZipExtractorThread::run()
{
	while ( !quitFlag() ) {
		m_active = false;
		waitMutex().lock();
		waitCondition().wait(&m_waitMutex);
		waitMutex().unlock();
		m_active = true;
		m_fileCount++;
		if ( !quitFlag() ) {
			// we assume that all 7z params are set when we are triggered to go on!
			m_result = SzArEx_Extract(m_db, m_lookInStream, m_fileIndex, m_blockIndex, m_buffer, m_bufferSize, m_offset, m_sizeProcessed, m_allocImp, m_allocTempImp);
			if ( result() == SZ_OK )
				emit extracted(m_fileIndex);
			else
				emit failed(m_fileIndex);
		}
	}
}
