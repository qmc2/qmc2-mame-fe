#ifndef MACHINELISTVIEWER_H
#define MACHINELISTVIEWER_H

#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QStringList>
#include <QTimer>

#include "machinelistmodel.h"
#include "filterconfigurationdialog.h"
#include "visiblecolumnsetup.h"
#include "rankitemwidget.h"
#include "ui_machinelistviewer.h"

class MachineListViewer : public QWidget, public Ui::MachineListViewer
{
	Q_OBJECT

       	public:
		explicit MachineListViewer(QWidget *parent = 0);
		~MachineListViewer();

		MachineListModel *model() { return m_model; }
		FilterConfigurationDialog *filterConfigurationDialog() { return m_filterConfigurationDialog; }
		VisibleColumnSetup *visibleColumnSetup() { return m_visibleColumnSetup; }
		QStringList &headers() { return model()->headers(); }
		QHeaderView *headerView() { return treeView->header(); }
		QList<int> &pages() { return model()->pages(); }

	public slots:
		void init();
		void adjustIconSizes();
		void on_toolButtonToggleMenu_clicked();
		void on_toolButtonConfigureFilters_clicked();
		void on_toolButtonVisibleColumns_clicked();
		void on_toolButtonEditQuery_clicked();
		void on_toolButtonUpdateView_clicked();
		void currentChanged(const QModelIndex &, const QModelIndex &);
		void currentChangedDelayed();
		void romStatusChanged(const QString &, char);
		void mainTagChanged(const QString &, bool);
		void mainSelectionChanged(const QString &);
		void treeViewVerticalScrollChanged(int);
		void treeViewUpdateRanks();
		void treeViewSectionMoved(int, int, int);
		void on_treeView_customContextMenuRequested(const QPoint &);
		void on_treeView_activated(const QModelIndex &);
		void on_treeView_entered(const QModelIndex &);
		void on_treeView_clicked(const QModelIndex &);

	signals:
		void selectionChanged(const QString &);
		void tagChanged(const QString &, bool);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void resizeEvent(QResizeEvent *);

	private:
		MachineListModel *m_model;
		QString m_currentId;
		bool m_ignoreSelectionChange;
		QTimer m_rankUpdateTimer;
		QTimer m_selectionUpdateTimer;
		FilterConfigurationDialog *m_filterConfigurationDialog;
		VisibleColumnSetup *m_visibleColumnSetup;
};

#endif
