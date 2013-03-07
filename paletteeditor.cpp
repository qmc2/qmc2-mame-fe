#include <QSettings>
#include "paletteeditor.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

PaletteEditor::PaletteEditor(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

PaletteEditor::~PaletteEditor()
{
}

void PaletteEditor::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeMiddle = iconSize + QSize(2, 2);
	QSize iconSizeLarge = iconSize + QSize(4, 4);

	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	pushButtonPreview->setIconSize(iconSize);
}

void PaletteEditor::on_pushButtonOk_clicked()
{
	accept();
}

void PaletteEditor::on_pushButtonCancel_clicked()
{
	reject();
}

void PaletteEditor::on_pushButtonPreview_clicked()
{
}

void PaletteEditor::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	adjustSize();
	QDialog::showEvent(e);
}
