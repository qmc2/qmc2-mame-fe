#include <QSettings>
#include <QApplication>
#include <QFileDialog>
#include <QColorDialog>
#include <QImageReader>

#include "brusheditor.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

QStringList BrushEditor::patternNames;

BrushEditor::BrushEditor(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	frameImagePreview->setAutoFillBackground(true);
	framePatternColor->setAutoFillBackground(true);
	framePatternPreview->setAutoFillBackground(true);
	frameGradientPreview->setAutoFillBackground(true);
}

BrushEditor::~BrushEditor()
{
}

Qt::BrushStyle BrushEditor::patternNameToStyle(QString patternName)
{
	switch ( patternNames.indexOf(patternName) ) {
		case QMC2_BRUSHEDITOR_PATTERN_SOLIDPATTERN:
			return Qt::SolidPattern;
		case QMC2_BRUSHEDITOR_PATTERN_DENSE1PATTERN:
			return Qt::Dense1Pattern;
		case QMC2_BRUSHEDITOR_PATTERN_DENSE2PATTERN:
			return Qt::Dense2Pattern;
		case QMC2_BRUSHEDITOR_PATTERN_DENSE3PATTERN:
			return Qt::Dense3Pattern;
		case QMC2_BRUSHEDITOR_PATTERN_DENSE4PATTERN:
			return Qt::Dense4Pattern;
		case QMC2_BRUSHEDITOR_PATTERN_DENSE5PATTERN:
			return Qt::Dense5Pattern;
		case QMC2_BRUSHEDITOR_PATTERN_DENSE6PATTERN:
			return Qt::Dense6Pattern;
		case QMC2_BRUSHEDITOR_PATTERN_DENSE7PATTERN:
			return Qt::Dense7Pattern;
		case QMC2_BRUSHEDITOR_PATTERN_HORPATTERN:
			return Qt::HorPattern;
		case QMC2_BRUSHEDITOR_PATTERN_VERPATTERN:
			return Qt::VerPattern;
		case QMC2_BRUSHEDITOR_PATTERN_CROSSPATTERN:
			return Qt::CrossPattern;
		case QMC2_BRUSHEDITOR_PATTERN_BDIAGPATTERN:
			return Qt::BDiagPattern;
		case QMC2_BRUSHEDITOR_PATTERN_FDIAGPATTERN:
			return Qt::FDiagPattern;
		case QMC2_BRUSHEDITOR_PATTERN_DIAGCROSSPATTERN:
			return Qt::DiagCrossPattern;
		case QMC2_BRUSHEDITOR_PATTERN_TEXTUREPATTERN:
			return Qt::TexturePattern;
		case QMC2_BRUSHEDITOR_PATTERN_NOBRUSH:
		default:
			return Qt::NoBrush;
	}
}

QString BrushEditor::patternStyleToName(Qt::BrushStyle style)
{
	switch ( style ) {
		case Qt::SolidPattern:
			return "SolidPattern";
		case Qt::Dense1Pattern:
			return "Dense1Pattern";
		case Qt::Dense2Pattern:
			return "Dense2Pattern";
		case Qt::Dense3Pattern:
			return "Dense3Pattern";
		case Qt::Dense4Pattern:
			return "Dense4Pattern";
		case Qt::Dense5Pattern:
			return "Dense5Pattern";
		case Qt::Dense6Pattern:
			return "Dense6Pattern";
		case Qt::Dense7Pattern:
			return "Dense7Pattern";
		case Qt::HorPattern:
			return "HorPattern";
		case Qt::VerPattern:
			return "VerPattern";
		case Qt::CrossPattern:
			return "CrossPattern";
		case Qt::BDiagPattern:
			return "BDiagPattern";
		case Qt::FDiagPattern:
			return "FDiagPattern";
		case Qt::DiagCrossPattern:
			return "DiagCrossPattern";
		case Qt::TexturePattern:
			return "TexturePattern";
		case Qt::NoBrush:
		default:
			return "NoBrush";
	}
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
	toolButtonPatternColor->setIconSize(iconSize);
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
	QStringList imageFileTypes;
	foreach (QByteArray imageFormat, QImageReader::supportedImageFormats())
		imageFileTypes << "*." + QString(imageFormat).toLower();
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose image file"), lineEditImageFile->text(), tr("Supported image files (%1)").arg(imageFileTypes.join(" ")) + ";;" + tr("All files (*)"));
	if ( !fileName.isEmpty() ) {
		lineEditImageFile->setText(fileName);
		QPalette pal = frameImagePreview->palette();
		pal.setBrush(frameImagePreview->backgroundRole(), QBrush(QImage(fileName)));
		frameImagePreview->setPalette(pal);
		frameImagePreview->update();
	}
}

void BrushEditor::on_toolButtonPatternColor_clicked()
{
	QColor color = QColorDialog::getColor(framePatternColor->palette().color(framePatternColor->backgroundRole()), this, tr("Choose pattern color"), QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
	if ( color.isValid() ) {
		QPalette pal = framePatternColor->palette();
		pal.setColor(framePatternColor->backgroundRole(), color);
		framePatternColor->setPalette(pal);
		framePatternColor->update();
		on_comboBoxPatternType_currentIndexChanged(comboBoxPatternType->currentIndex());
	}
}

void BrushEditor::on_toolButtonBrowseTextureImage_clicked()
{
	QStringList imageFileTypes;
	foreach (QByteArray imageFormat, QImageReader::supportedImageFormats())
		imageFileTypes << "*." + QString(imageFormat).toLower();
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose image file"), lineEditTextureImage->text(), tr("Supported image files (%1)").arg(imageFileTypes.join(" ")) + ";;" + tr("All files (*)"));
	if ( !fileName.isEmpty() ) {
		lineEditTextureImage->setText(fileName);
		on_comboBoxPatternType_currentIndexChanged(comboBoxPatternType->currentIndex());
	}
}

void BrushEditor::on_toolButtonAddColorStop_clicked()
{
	// FIXME
}

void BrushEditor::on_toolButtonRemoveColorStop_clicked()
{
	// FIXME
}

void BrushEditor::on_comboBoxPatternType_currentIndexChanged(int index)
{
	switch ( index ) {
		case QMC2_BRUSHEDITOR_PATTERN_NOBRUSH:
			framePatternColor->setEnabled(false);
			toolButtonPatternColor->setEnabled(false);
			lineEditTextureImage->setEnabled(false);
			toolButtonBrowseTextureImage->setEnabled(false);
			break;
		case QMC2_BRUSHEDITOR_PATTERN_TEXTUREPATTERN:
			framePatternColor->setEnabled(false);
			toolButtonPatternColor->setEnabled(false);
			lineEditTextureImage->setEnabled(true);
			toolButtonBrowseTextureImage->setEnabled(true);
			break;
		default:
			framePatternColor->setEnabled(true);
			toolButtonPatternColor->setEnabled(true);
			lineEditTextureImage->setEnabled(false);
			toolButtonBrowseTextureImage->setEnabled(false);
			break;
	}
	QPalette pal = framePatternPreview->palette();
	if ( index == QMC2_BRUSHEDITOR_PATTERN_TEXTUREPATTERN )
		pal.setBrush(framePatternPreview->backgroundRole(), QBrush(QImage(lineEditTextureImage->text())));
	else
		pal.setBrush(framePatternPreview->backgroundRole(), QBrush(framePatternColor->palette().color(framePatternColor->backgroundRole()), patternNameToStyle(comboBoxPatternType->currentText())));
	framePatternPreview->setPalette(pal);
	framePatternPreview->update();
}

void BrushEditor::on_comboBoxGradientType_currentIndexChanged(int index)
{
	// FIXME
}

void BrushEditor::showEvent(QShowEvent *e)
{
	adjustIconSizes();

	if ( e )
		QWidget::showEvent(e);
}
