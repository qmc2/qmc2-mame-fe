#ifndef _BRUSHEDITOR_H_
#define _BRUSHEDITOR_H_

#include "ui_brusheditor.h"

class BrushEditor : public QDialog, public Ui::BrushEditor
{
	Q_OBJECT

       	public:
		BrushEditor(QWidget *parent = 0);
		~BrushEditor();

	public slots:
		void on_toolButtonOk_clicked();
		void on_toolButtonCancel_clicked();
};

#endif
