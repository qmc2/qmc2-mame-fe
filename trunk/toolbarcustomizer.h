#ifndef _TOOLBARCUSTOMIZER_H_
#define _TOOLBARCUSTOMIZER_H_

#include <QList>
#include <QMap>
#include <QAction>
#include <QIcon>

#include "ui_toolbarcustomizer.h"

class ToolBarCustomizer : public QDialog, public Ui::ToolBarCustomizer
{
	Q_OBJECT

	public:
		QMap<QListWidgetItem *, QAction *> availableToolBarActions;
		QMap<QListWidgetItem *, QAction *> activeToolBarActions;
		QMap<QString, QAction *> availableActionsByName;
		QStringList defaultToolBarActions;
		QStringList activeActions;
		QStringList appliedActions;
		QAction *separatorAction;
		bool resetToDefault;
		bool firstRefresh;

		ToolBarCustomizer(QWidget *parent = 0);

	public slots:
		void adjustIconSizes();
		void refreshAvailableActions();
		void refreshActiveActions();
		void on_pushButtonOk_clicked();
		void on_pushButtonApply_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonActivateActions_clicked();
		void on_pushButtonDeactivateActions_clicked();
		void on_pushButtonActionUp_clicked();
		void on_pushButtonActionDown_clicked();
		void on_pushButtonInsertSeparator_clicked();
		void on_pushButtonDefault_clicked();
		void on_listWidgetAvailableActions_itemSelectionChanged();
		void on_listWidgetActiveActions_itemSelectionChanged();

	protected:
		void showEvent(QShowEvent *);

	private:
		QIcon m_bookIcon;
		QIcon m_findIcon;
};

#endif
