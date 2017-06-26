#ifndef GRADIENTSTOPACTIONS_H
#define GRADIENTSTOPACTIONS_H

#include <QTreeWidgetItem>
#include "ui_gradientstopactions.h"

class GradientStopActions : public QWidget, public Ui::GradientStopActions
{
	Q_OBJECT

       	public:
		GradientStopActions(QTreeWidgetItem *, QWidget *parent = 0);
		~GradientStopActions();

		QTreeWidgetItem *parentItem;

	public slots:
		void adjustIconSizes();
		void on_toolButtonRemove_clicked();
		void on_toolButtonUp_clicked();
		void on_toolButtonDown_clicked();

	signals:
		void removeRequested(QTreeWidgetItem *);
		void upRequested(QTreeWidgetItem *);
		void downRequested(QTreeWidgetItem *);
};

#endif
