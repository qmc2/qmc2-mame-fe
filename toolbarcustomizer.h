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
		QMap<QString, QAction *> availableActionsByName;
		QStringList defaultToolBarActions;
		QAction *separatorAction;

		ToolBarCustomizer(QWidget *parent = 0);
		~ToolBarCustomizer();

	public slots:
		void adjustIconSizes();
		void refreshAvailableActions();
		void refreshActiveActions();
		void on_pushButtonOk_clicked();
		void on_pushButtonApply_clicked();
		void on_pushButtonCancel_clicked();

	protected:
		void showEvent(QShowEvent *);
};

#endif
