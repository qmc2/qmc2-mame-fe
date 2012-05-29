#ifndef _TOOLBARCUSTOMIZER_H_
#define _TOOLBARCUSTOMIZER_H_

#include <QList>
#include <QAction>

#include "ui_toolbarcustomizer.h"

class ToolBarCustomizer : public QDialog, public Ui::ToolBarCustomizer
{
	Q_OBJECT

	public:
		QMap<QListWidgetItem *, QAction *> availableToolBarActions;
		QMap<QListWidgetItem *, QAction *> activeToolBarActions;

		ToolBarCustomizer(QWidget *parent = 0);
		~ToolBarCustomizer();

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonApply_clicked();
		void on_pushButtonCancel_clicked();

	protected:
		void showEvent(QShowEvent *);
};

#endif
