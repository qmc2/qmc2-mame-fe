#ifndef MACHINELISTVIEWER_H
#define MACHINELISTVIEWER_H

#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QStringList>
#include <QAction>
#include <QTimer>
#include <QMenu>

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
		QString &name() { return m_name; }

		void settingsSaveView();
		void settingsRemoveView();
		void loadView(const QString &);
		void saveView();

		static QStringList &savedViews() { return m_savedViews; }
		static QStringList &attachedViews() { return m_attachedViews; }
		static void loadSavedViews();
		static void setSavedViewsLoaded(bool loaded = true) { m_savedViewsLoaded = loaded; }
		static bool savedViewsLoaded() { return m_savedViewsLoaded; }
		static void setViewSelectSeparatorIndex(int index) { m_viewSelectSeparatorIndex = index; }
		static int viewSelectSeparatorIndex() { return m_viewSelectSeparatorIndex; }
		static int totalViews() { return m_viewSelectSeparatorIndex + m_attachedViews.count(); }
		void connectToDb() { model()->connectToDb(); }
		void disconnectFromDb() { model()->disconnectFromDb(); }

	public slots:
		void init();
		void adjustIconSizes();
		void currentChanged(const QModelIndex &, const QModelIndex &);
		void currentChangedDelayed();
		void romStatusChanged(const QString &, char);
		void mainTagChanged(const QString &, bool);
		void mainSelectionChanged(const QString &);
		void treeViewVerticalScrollChanged(int);
		void treeViewUpdateRanks();
		void treeViewSectionMoved(int, int, int);
		void saveViewAction_triggered(bool);
		void removeViewAction_triggered(bool);
		void attachViewAction_triggered(bool);
		void detachViewAction_triggered(bool);
		void cloneViewAction_triggered(bool);
		void lineEdit_textChanged(const QString &);
		void updateCurrentView();

		// automatically connected slots
		void on_toolButtonToggleMenu_clicked();
		void on_toolButtonConfigureFilters_clicked();
		void on_toolButtonVisibleColumns_clicked();
		void on_toolButtonUpdateView_clicked();
		void on_treeView_customContextMenuRequested(const QPoint &);
		void on_treeView_activated(const QModelIndex &);
		void on_treeView_entered(const QModelIndex &);
		void on_treeView_clicked(const QModelIndex &);
		void on_comboBoxViewName_activated(const QString &);

	signals:
		void selectionChanged(const QString &);
		void tagChanged(const QString &, bool);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void resizeEvent(QResizeEvent *);

	private:
		static QStringList m_savedViews;
		static QStringList m_attachedViews;
		static bool m_savedViewsLoaded;
		static int m_viewSelectSeparatorIndex;

		MachineListModel *m_model;
		QString m_currentId;
		bool m_ignoreSelectionChange;
		bool m_initCalled;
		QTimer m_rankUpdateTimer;
		QTimer m_selectionUpdateTimer;
		FilterConfigurationDialog *m_filterConfigurationDialog;
		VisibleColumnSetup *m_visibleColumnSetup;
		QMenu *m_toolsMenu;
		QAction *m_saveViewAction;
		QAction *m_removeViewAction;
		QAction *m_attachViewAction;
		QAction *m_detachViewAction;
		QAction *m_cloneViewAction;
		QString m_name;
};

#endif
