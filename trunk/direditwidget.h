#ifndef _DIREDITWIDGET_H_
#define _DIREDITWIDGET_H_

#include <QTreeWidget>
#include "ui_direditwidget.h"

class DirectoryEditWidget : public QWidget, public Ui::DirectoryEditWidget
{
	Q_OBJECT

	public:
		QTreeWidget *myTreeWidget;

		DirectoryEditWidget(QString, QWidget *parent = 0, QTreeWidget *treeWidget = 0);
		~DirectoryEditWidget();

	public slots:
		void on_toolButtonBrowse_clicked();
		void on_lineEditDirectory_textChanged(const QString &) { emit dataChanged(this); }

	signals:
		void dataChanged(QWidget *);
};

#endif
