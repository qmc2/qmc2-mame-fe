#include "paletteeditor.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;

PaletteEditor::PaletteEditor(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

PaletteEditor::~PaletteEditor()
{
}

void PaletteEditor::on_toolButtonOk_clicked()
{
}

void PaletteEditor::on_toolButtonCancel_clicked()
{
}

void PaletteEditor::on_toolButtonPreview_clicked()
{
}
