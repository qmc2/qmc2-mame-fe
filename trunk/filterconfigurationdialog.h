#ifndef _FILTERCONFIGURATIONDIALOG_H_
#define _FILTERCONFIGURATIONDIALOG_H_

#include <QHash>
#include <QToolButton>
#include <QTreeWidgetItem>

#include "ui_filterconfigurationdialog.h"

class MachineListViewer;

#define QMC2_FCDLG_COLUMN_ACTION	0
#define QMC2_FCDLG_COLUMN_NAME		1

#define QMC2_FCDLG_PAGE_DEFAULT		0
#define QMC2_FCDLG_PAGE_STRING		1
#define QMC2_FCDLG_PAGE_BOOL		0
#define QMC2_FCDLG_PAGE_ICON		0
#define QMC2_FCDLG_PAGE_ROMSTATUS	0
#define QMC2_FCDLG_PAGE_DRVSTATUS	0
#define QMC2_FCDLG_PAGE_RANK		0
#define QMC2_FCDLG_PAGE_INT		0

class FilterConfigurationDialog : public QDialog, public Ui::FilterConfigurationDialog
{
	Q_OBJECT

       	public:
		FilterConfigurationDialog(MachineListViewer *viewer, QWidget *parent = 0);

		MachineListViewer *viewer() { return m_viewer; }
		int buttonToPage(QToolButton *tb) { return m_buttonToPageHash.value(tb); }
		QTreeWidgetItem *buttonToItem(QToolButton *tb) { return m_buttonToItemHash.value(tb); }

	public slots:
		void init();
		void adjustIconSizes();
		void addFilterClicked();
		void removeFilterClicked();
		void on_pushButtonOk_clicked();
		void on_pushButtonApply_clicked();
		void on_pushButtonCancel_clicked();

	protected:
		void showEvent(QShowEvent *e);
		void hideEvent(QHideEvent *e);

	private:
		MachineListViewer *m_viewer;
		QHash<QToolButton *, int> m_buttonToPageHash;
		QHash<QToolButton *, QTreeWidgetItem *> m_buttonToItemHash;
		QHash<QToolButton *, QTreeWidgetItem *> m_removeButtonToItemHash;
};

#endif
