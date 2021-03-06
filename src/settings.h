#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef QSETTINGS_H
#include <QtCore/QSettings>
#endif
#ifndef QSTRING_H
#include <QtCore/QString>
#endif
#ifndef QSTRING_H
#include <QtCore/QStringList>
#endif
#ifndef QREGEXP_H
#include <QtCore/QRegExp>
#endif

class Settings : public QSettings
{
	Q_OBJECT

	public:
#ifndef QT_NO_QOBJECT
		Settings(Format format, Scope scope, const QString &organization, const QString &application = QString(), QObject *parent = 0) : QSettings(format, scope, organization, application, parent) {} ;
#else
		Settings(Format format, Scope scope, const QString &organization, const QString &application = QString()) : QSettings(format, scope, organization, &application) {} ;
#endif
		virtual QVariant value(const QString & key, const QVariant & defaultValue = QVariant()) const;
		static QStringList stResolve(const QStringList& qstr);
		static QString stResolve(const QString& qstr);
};

#endif
