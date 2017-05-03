#ifndef BIGBYTEARRAY_H
#define BIGBYTEARRAY_H

#include <QList>
#include <QString>
#include <QByteArray>
#include "macros.h"

#define QMC2_BBA_CHUNK_SIZE	quint64(QMC2_1G)

class BigByteArray
{
	public:
		explicit BigByteArray() { ; }
		explicit BigByteArray(const BigByteArray &bba);
		explicit BigByteArray(const char *rawData, quint64 len);
		~BigByteArray() { clear(); }

		// QByteArray-like API
		void clear() { m_concatByteArrays.clear(); }
		void setRawData(const char *rawData, quint64 len) { clear(); *this = BigByteArray(rawData, len); }
		void append(const QByteArray &ba);
		char at(quint64 index);
		quint64 size();
		quint64 length() { return size(); }
		QByteArray &mid(quint64 index, int len);

		// BigByteArray-specific
		int chunks() const { return m_concatByteArrays.count(); }
		const QByteArray &chunk(int index) const { return m_concatByteArrays.at(index); }
		quint64 chunkSize() { return QMC2_BBA_CHUNK_SIZE; }
		QString crc32();
		QString sha1();
		QString md5();

	private:
		QList<QByteArray> m_concatByteArrays;
		QByteArray m_tempArray;
};

#endif // BIGBYTEARRAY_H
