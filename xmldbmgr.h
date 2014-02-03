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

	public slots:
		void recreateDatabase();

	private:
		mutable QSqlDatabase m_db;
};

#endif
