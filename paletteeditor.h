#ifndef _PALETTEEDITOR_H_
#define _PALETTEEDITOR_H_

#include "colorwidget.h"
#include "ui_paletteeditor.h"

class PaletteEditor : public QDialog, public Ui::PaletteEditor
{
	Q_OBJECT

       	public:
		PaletteEditor(QWidget *parent = 0);
		~PaletteEditor();

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonPreview_clicked();

	protected:
		void showEvent(QShowEvent *);
};

#endif
