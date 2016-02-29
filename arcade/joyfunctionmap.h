#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)

#ifndef JOYFUNCTIONMAP_H
#define JOYFUNCTIONMAP_H

#include <QObject>
#include <QMap>
#include <QStringList>

class JoyFunctionMap : public QObject
{
	Q_OBJECT
public:
	explicit JoyFunctionMap(QStringList keySequences, QObject *parent = 0);

public slots:
	void setKeySequences(QStringList keySequences);
	void loadJoyFunctionMap();
	QString mapJoyFunction(QString joyFunction);

private:
	QStringList mNativeKeySequences;
	QMap<QString, QString> mJoyFunctionMap;
};

#endif

#endif
