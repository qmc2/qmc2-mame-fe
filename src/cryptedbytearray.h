#ifndef CRYPTEDBYTEARRAY_H
#define CRYPTEDBYTEARRAY_H

#include <QByteArray>

class CryptedByteArray : public QByteArray
{
	public:
		explicit CryptedByteArray();
		explicit CryptedByteArray(const QByteArray &ba, const QByteArray &key = QByteArray());
		explicit CryptedByteArray(const char *rawData, const char *key = 0);

		void setKey(const QByteArray &key) { m_key = key;  m_keyValueValid = false; }
		QByteArray &key() { return m_key; }
		QByteArray &encryptedData();
		QByteArray &decryptedData() { return encryptedData(); }
		void reset() { m_cryptoData.clear(); }

	private:
		void crypt();
		uint keyToValue();
		uint m_keyValue;
		bool m_keyValueValid;
		QByteArray m_key;
		QByteArray m_cryptoData;
};

#endif // CRYPTEDBYTEARRAY_H
