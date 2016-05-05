#include "cryptedbytearray.h"

CryptedByteArray::CryptedByteArray() :
	QByteArray(),
	m_keyValue(0),
	m_keyValueValid(false)
{
	setKey(QByteArray());
	reset();
}

CryptedByteArray::CryptedByteArray(const QByteArray &ba, const QByteArray &key) :
	QByteArray(ba)
{
	setKey(key);
	reset();
	crypt();
}

CryptedByteArray::CryptedByteArray(const char *rawData, const char *key) :
	QByteArray(rawData)
{
	setKey(key);
	reset();
	crypt();
}

QByteArray &CryptedByteArray::encryptedData()
{
	if ( m_cryptoData.isEmpty() )
		crypt();
	return m_cryptoData;
}

void CryptedByteArray::crypt()
{
	reset();
	qsrand(keyToValue());
	for (int i = 0; i < length(); i++) {
		char randChar = qrand() % 256;
		m_cryptoData.append(at(i) ^ randChar);
	}
}

uint CryptedByteArray::keyToValue()
{
	if ( !m_keyValueValid ) {
		m_keyValue = 0;
		for (int i = 0; i < m_key.length(); i++)
			m_keyValue += m_key.at(i) * m_key.length();
		m_keyValueValid = true;
	}
	return m_keyValue;
}
