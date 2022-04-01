#ifndef _COOKIEMANAGER_H
#define _COOKIEMANAGER_H

#include <QSqlQueryModel>
#include "ui_cookiemanager.h"

class CookieManager : public QDialog, public Ui::CookieManager
{
	Q_OBJECT

       	public:
		QSqlQueryModel *sqlTableModel;

		CookieManager(QWidget *parent = 0);
		~CookieManager();

	public slots:
		void adjustIconSizes();
		void tableViewCookies_selectionChanged(const QItemSelection &, const QItemSelection &);
		void on_pushButtonRemove_clicked();
};

#endif
