#ifndef KEYMAP_H
#define KEYMAP_H

#include <QObject>
#include <QMap>
#include <QStringList>

class KeySequenceMap : public QObject
{
	Q_OBJECT

public:
	explicit KeySequenceMap(QStringList keySequences, QObject *parent = 0);

public slots:
	void setKeySequences(QStringList keySequences);
	void loadKeySequenceMap();
	QString mapKeySequence(QString keySeq);

private:
	QStringList mNativeKeySequences;
	QMap<QString, QString> mKeySequenceMap;
};

#endif // KEYMAP_H
