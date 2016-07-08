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
		FilterConfigurationDialog *filterConfig() { return m_filterConfig; }

	public slots:
		void init();
		void adjustIconSizes();
		void on_toolButtonToggleMenu_clicked();
		void on_toolButtonConfigureFilters_clicked();
		void currentChanged(const QModelIndex &current, const QModelIndex &previous);
		void mainSelectionChanged(const QString &id);
		void treeViewVerticalScrollChanged(int);
		void treeViewUpdateRanks();

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
		FilterConfigurationDialog *m_filterConfig;
};

#endif
