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
		void on_toolButtonOk_clicked();
		void on_toolButtonCancel_clicked();
		void on_toolButtonPreview_clicked();
};

#endif
