#ifndef _FILEEDITWIDGET_H_
#define _FILEEDITWIDGET_H_

#include "ui_fileeditwidget.h"

class FileEditWidget : public QWidget, public Ui::FileEditWidget
{
	Q_OBJECT

       	public:
		QString browserFilter;
		QString browserPart;

		FileEditWidget(QString, QString, QString part = "", QWidget *parent = 0, bool showClearButton = false);
		~FileEditWidget();

	public slots:
		void on_toolButtonBrowse_clicked();
		void on_lineEditFile_textChanged(const QString &) { emit dataChanged(this); }

	signals:
		void dataChanged(QWidget *);
};

#endif
