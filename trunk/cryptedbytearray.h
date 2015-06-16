#ifndef CRYPTEDBYTEARRAY_H
#define CRYPTEDBYTEARRAY_H

#include <QByteArray>
#include <QString>

class CryptedByteArray : public QByteArray
{
	public:
		explicit CryptedByteArray();
		explicit CryptedByteArray(QByteArray ba, QString key = QString());
		explicit CryptedByteArray(const char *rawData, const char *key = 0);

		void setKey(QString key) { m_key = key; }
		QString key() { return m_key; }
		QByteArray &encryptedData();
		QByteArray &decryptedData() { return encryptedData(); }
		void reset() { m_cryptoData.clear(); }

	private:
		void crypt();
		uint keyToValue();
		QString m_key;
		QByteArray m_cryptoData;
};

#endif
