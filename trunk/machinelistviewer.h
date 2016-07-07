#ifndef _MACHINELISTVIEWER_H_
#define _MACHINELISTVIEWER_H_

#include <QStyledItemDelegate>

#include "machinelistmodel.h"
#include "ui_machinelistviewer.h"

class MachineListViewer : public QWidget, public Ui::MachineListViewer
{
	Q_OBJECT

       	public:
		explicit MachineListViewer(QWidget *parent = 0);
		~MachineListViewer();

		MachineListModel *model() { return m_model; }

	public slots:
		void init();
		void adjustIconSizes();
		void on_toolButtonToggleMenu_clicked();

		void currentChanged(const QModelIndex &current, const QModelIndex &previous);
		void mainSelectionChanged(const QString &id);

	signals:
		void selectionChanged(const QString &);

	protected:
		void showEvent(QShowEvent *e);
		void hideEvent(QHideEvent *e);

	private:
		MachineListModel *m_model;
		QString m_currentId;
		bool m_ignoreSelectionChange;
};

#endif
