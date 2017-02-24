#ifndef BIGBYTEARRAY_H
#define BIGBYTEARRAY_H

#include <QList>
#include <QByteArray>

class BigByteArray
{
	public:
		explicit BigByteArray() { ; }
		explicit BigByteArray(const BigByteArray &bba);
		explicit BigByteArray(const char *rawData, quint64 len);
		~BigByteArray() { clear(); }

		void setRawData(const char *rawData, quint64 len) { *this = BigByteArray(rawData, len); }
		void clear() { m_concatByteArrays.clear(); }
		int chunks() const { return m_concatByteArrays.count(); }
		const QByteArray &chunk(int index) const { return m_concatByteArrays.at(index); }
		void append(const QByteArray &ba);
		char at(quint64 index);
		quint64 size();
		quint64 length() { return size(); }
		QByteArray &mid(quint64 index, int len);

	private:
		QList<QByteArray> m_concatByteArrays;
		QByteArray m_tempArray;
};

#endif // BIGBYTEARRAY_H
