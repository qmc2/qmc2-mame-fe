#include "settings.h"
#ifndef IOSTREAM_H
#ifdef QMC2_DEBUG_ENV_RESOLVE
#include <iostream>
#endif
#endif

QStringList Settings::stResolve(const QStringList& qstrl) {
	QString qstr;
	QStringList qstrl2;
	foreach (qstr, qstrl)
		qstrl2 << stResolve(qstr);
	return qstrl2;
}

QString Settings::stResolve(const QString& qstr) {
	QByteArray qbaBuf;
	QString qstrEnv, qstrFinal;

#if defined(QMC2_OS_WIN)
	QRegExp qrx("(\\%(.*)\\%)", Qt::CaseInsensitive, QRegExp::RegExp2);
#else
	QRegExp qrx("(\\$\\{(.*)\\})", Qt::CaseSensitive, QRegExp::RegExp2);
#endif
	qrx.setMinimal(true);

	int pos = 0;
	int posLastEnd = -1;
	while ((pos = qrx.indexIn(qstr, pos)) != -1) {
		if (pos > posLastEnd)
			qstrFinal += qstr.midRef(posLastEnd + 1, pos - (posLastEnd + 1));

		qbaBuf = qrx.cap(2).toUtf8();
		qbaBuf = qgetenv(qbaBuf.constData());
		if (!qbaBuf.isNull()) {
#ifdef QMC2_DEBUG_ENV_RESOLVE
			std::cout << "match: '" << qrx.cap(1).toStdString() << "', resolve: '" << qbaBuf.constData() << "'" << std::endl;
#endif
			qstrFinal += QString::fromLocal8Bit(qbaBuf.constData());
		} else
			qstrFinal += qrx.cap(1);  // unresolved, so put it back untouched
    
		pos += qrx.matchedLength();
		posLastEnd = pos - 1;
	}
	if (posLastEnd < qstr.length())
		qstrFinal += qstr.midRef(posLastEnd + 1, qstr.length());
#ifdef QMC2_DEBUG_ENV_RESOLVE
	std::cout << "input: '" << qstr.toStdString() << "' output: '" << qstrFinal.toStdString() << "'" << std::endl;
#endif
	return qstrFinal;
}

QVariant Settings::value(const QString& key, const QVariant& defaultValue) const
{
	QVariant v = QSettings::value(key, defaultValue);
	if (QString(v.typeName()) == QString("QString") && v.toString().contains("${")) {
		v = QVariant(stResolve(v.toString()));
		return v;
	} else
		return v;
}
