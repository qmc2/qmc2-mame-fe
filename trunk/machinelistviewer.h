#ifndef _MACHINELISTVIEWER_H_
#define _MACHINELISTVIEWER_H_

#include <QStyledItemDelegate>
#include <QTimer>

#include "machinelistmodel.h"
#include "filterconfigurationdialog.h"
#include "ui_machinelistviewer.h"

class MachineListViewer : public QWidget, public Ui::MachineListViewer
{
	Q_OBJECT

       	public:
		explicit MachineListViewer(QWidget *parent = 0);
		~MachineListViewer();

		MachineListModel *model() { return m_model; }
		FilterConfigurationDialog *filterConfigurationDialog() { return m_filterConfigurationDialog; }

	public slots:
		void init();
		void adjustIconSizes();
		void on_toolButtonToggleMenu_clicked();
		void on_toolButtonConfigureFilters_clicked();
		void on_toolButtonUpdateView_clicked();
		void currentChanged(const QModelIndex &current, const QModelIndex &previous);
		void romStatusChanged(const QString &id, char status);
		void mainSelectionChanged(const QString &id);
		void treeViewVerticalScrollChanged(int);
		void treeViewUpdateRanks();
		void on_treeView_customContextMenuRequested(const QPoint &p);
		void on_treeView_activated(const QModelIndex &);

	signals:
		void selectionChanged(const QString &);

	protected:
		void showEvent(QShowEvent *e);
		void hideEvent(QHideEvent *e);
		void resizeEvent(QResizeEvent *e);

	private:
		MachineListModel *m_model;
		QString m_currentId;
		bool m_ignoreSelectionChange;
		QTimer m_rankUpdateTimer;
		FilterConfigurationDialog *m_filterConfigurationDialog;
};

#endif
