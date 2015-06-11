#ifndef _FILEEDITWIDGET_H_
#define _FILEEDITWIDGET_H_

#include <QTreeWidget>
#include "ui_fileeditwidget.h"

class FileEditWidget : public QWidget, public Ui::FileEditWidget
{
	Q_OBJECT

       	public:
		QString browserFilter;
		QString browserPart;
		QString relativeToFolderOption;
		QTreeWidget *myTreeWidget;

		FileEditWidget(QString, QString, QString part = "", QWidget *parent = 0, bool showClearButton = false, QString relativeTo = QString(), QTreeWidget *treeWidget = 0, bool cbMode = false);

		bool returnRelativePath() { return !relativeToFolderOption.isEmpty(); }
		QString relativeToPath();

		bool comboBoxMode() { return m_comboBoxMode; } 

	public slots:
		void on_toolButtonBrowse_clicked();
		void on_toolButtonClear_clicked();
		void on_lineEditFile_textChanged(const QString &) { emit dataChanged(this); }
		void on_comboBox_editTextChanged(const QString &) { emit dataChanged(this); }

	signals:
		void dataChanged(QWidget *);

	private:
		bool m_comboBoxMode;
};

#endif
