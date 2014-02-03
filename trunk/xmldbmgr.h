#ifndef _XMLDBMGR_H_
#define _XMLDBMGR_H_

#include <QObject>
#include <QSqlDatabase>

class XmlDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit XmlDatabaseManager(QObject *parent);
		~XmlDatabaseManager();

		QString emulatorVersion();
		void setEmulatorVersion(QString);
		QString qmc2Version();
		void setQmc2Version(QString);
		int xmlCacheVersion();
		void setXmlCacheVersion(int);
		QString dtd();
		void setDtd(QString);

	public slots:
		void recreateDatabase();

	private:
		mutable QSqlDatabase m_db;
		QString m_tableBasename;
};

#endif
