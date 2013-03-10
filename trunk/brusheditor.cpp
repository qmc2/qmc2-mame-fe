#include <QSettings>
#include <QApplication>
#include <QFileDialog>
#include <QColorDialog>

#include "brusheditor.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

BrushEditor::BrushEditor(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

BrushEditor::~BrushEditor()
{
}

void BrushEditor::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	toolButtonBrowseImageFile->setIconSize(iconSize);
	toolButtonBrowseTextureImage->setIconSize(iconSize);
	toolButtonAddColorStop->setIconSize(iconSize);
	toolButtonRemoveColorStop->setIconSize(iconSize);
}

void BrushEditor::on_pushButtonOk_clicked()
{
	// FIXME
	accept();
}

void BrushEditor::on_pushButtonCancel_clicked()
{
	// FIXME
	reject();
}

void BrushEditor::on_toolButtonBrowseImageFile_clicked()
{
	// FIXME
}

void BrushEditor::on_toolButtonBrowseTextureImage_clicked()
{
	// FIXME
}

void BrushEditor::on_toolButtonAddColorStop_clicked()
{
	// FIXME
}

void BrushEditor::on_toolButtonRemoveColorStop_clicked()
{
	// FIXME
}

void BrushEditor::on_comboBoxPatternType_currentIndexChanged(int)
{
	// FIXME
}

void BrushEditor::on_comboBoxGradientType_currentIndexChanged(int)
{
	// FIXME
}

void BrushEditor::showEvent(QShowEvent *e)
{
	adjustIconSizes();

	if ( e )
		QWidget::showEvent(e);
}
