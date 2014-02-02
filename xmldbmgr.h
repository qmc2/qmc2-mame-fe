#ifndef _XMLDBMGR_H_
#define _XMLDBMGR_H_

#include <QSqlQueryModel>

class XmlDatabaseManager : public QObject
{
	Q_OBJECT

	public:
		explicit XmlDatabaseManager(QObject *parent);
		~XmlDatabaseManager();

	public slots:

	private:
};

#endif
