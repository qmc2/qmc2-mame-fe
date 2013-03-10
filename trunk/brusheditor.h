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
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_toolButtonBrowseImageFile_clicked();
		void on_toolButtonBrowseTextureImage_clicked();
		void on_toolButtonAddColorStop_clicked();
		void on_toolButtonRemoveColorStop_clicked();
		void on_comboBoxPatternType_currentIndexChanged(int);
		void on_comboBoxGradientType_currentIndexChanged(int);

	protected:
		void showEvent(QShowEvent *);
};

#endif
