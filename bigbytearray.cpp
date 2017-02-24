#include <QDebug>

#include "bigbytearray.h"
#include "macros.h"

#define BBA_CHUNK_SIZE	quint64(QMC2_1G)

BigByteArray::BigByteArray(const BigByteArray &bba)
{
	for (int i = 0; i < bba.chunks(); i++)
		m_concatByteArrays.append(bba.chunk(i));
}

BigByteArray::BigByteArray(const char *rawData, quint64 len)
{
	int c = int(len / BBA_CHUNK_SIZE) + 1;
	for (int i = 0; i < c; i++) {
		int l = QMC2_MIN(BBA_CHUNK_SIZE, len);
		m_concatByteArrays.append(QByteArray(rawData + i * BBA_CHUNK_SIZE, l));
		len -= l;
	}
}

void BigByteArray::append(const QByteArray &ba)
{
	if ( m_concatByteArrays.isEmpty() )
		m_concatByteArrays.append(ba);
	else {
		QByteArray baCopy(ba);
		quint64 len = baCopy.size();
		while ( len > 0 ) {
			int l = QMC2_MIN(BBA_CHUNK_SIZE, len);
			if ( quint64(l + m_concatByteArrays.last().size()) > BBA_CHUNK_SIZE )
				l = BBA_CHUNK_SIZE - m_concatByteArrays.last().size();
			m_concatByteArrays.last().append(baCopy, l);
			baCopy.remove(0, l);
			len = baCopy.size();
			if ( len > 0 )
				m_concatByteArrays.append(QByteArray());
		}

	}
}

QByteArray &BigByteArray::mid(quint64 index, int len)
{
	m_tempArray.clear();
	int c = int(index / BBA_CHUNK_SIZE);
	if ( c < m_concatByteArrays.count() ) {
		int lc = int((index + len) / BBA_CHUNK_SIZE);
		if ( lc < m_concatByteArrays.count() ) {
			int i = index - c * BBA_CHUNK_SIZE;
			int l = QMC2_MIN(m_concatByteArrays.at(c).size() - i, len);
			m_tempArray.append(m_concatByteArrays.at(c).mid(i, l));
			if ( l < len )
				m_tempArray.append(m_concatByteArrays.at(lc).left(len - l));
		} else
			qWarning() << "BigByteArray::mid(): length out of range";
	} else
		qWarning() << "BigByteArray::mid(): index out of range";
	return m_tempArray;
}

quint64 BigByteArray::size()
{
	quint64 s = 0;
	for (int i = 0; i < chunks(); i++)
		s += chunk(i).size();
	return s;
}

char BigByteArray::at(quint64 index)
{
	int c = int(index / BBA_CHUNK_SIZE);
	if ( c < m_concatByteArrays.count() ) {
		int i = index - c * BBA_CHUNK_SIZE;
		if ( i < m_concatByteArrays.at(c).size() )
			return m_concatByteArrays.at(c).at(i);
		else {
			qWarning() << "BigByteArray::at(): index out of range";
			return (char)0;
		}
	} else {
		qWarning() << "BigByteArray::at(): index out of range";
		return (char)0;
	}
}
