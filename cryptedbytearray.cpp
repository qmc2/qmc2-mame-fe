#include "cryptedbytearray.h"

CryptedByteArray::CryptedByteArray() :
	QByteArray()
{
	setKey(QString());
	reset();
}

CryptedByteArray::CryptedByteArray(QByteArray ba, QString key) :
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
	uint keyValue = 0;
	for (int i = 0; i < m_key.length(); i++)
		keyValue += m_key[i].unicode() * m_key.length();
	return keyValue;
}
