#include <QApplication>
#include <QFileDialog>
#include <QColorDialog>

#include "brusheditor.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QString qmc2FileEditStartPath;

BrushEditor::BrushEditor(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

BrushEditor::~BrushEditor()
{
}

void BrushEditor::on_toolButtonOk_clicked()
{
}

void BrushEditor::on_toolButtonCancel_clicked()
{
}
