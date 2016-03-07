#include <QApplication>
#include <QFileDialog>
#include <QColorDialog>
#include <QImageReader>
#include <QDoubleSpinBox>
#include <QPalette>

#include "settings.h"
#include "brusheditor.h"
#include "colorwidget.h"
#include "gradientstopactions.h"
#include "qmc2main.h"
#include "options.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;

QStringList BrushEditor::patternNames;
QStringList BrushEditor::gradientTypeNames;
QStringList BrushEditor::gradientSpreadNames;

BrushEditor::BrushEditor(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	frameImagePreview->setAutoFillBackground(true);
	framePatternColor->setAutoFillBackground(true);
	framePatternPreview->setAutoFillBackground(true);
	frameGradientPreview->setAutoFillBackground(true);

	connect(doubleSpinBoxLinearStartPointX, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxLinearStartPointY, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxLinearEndPointX, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxLinearEndPointY, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxRadialCenterPointX, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxRadialCenterPointY, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxRadialFocalPointX, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxRadialFocalPointY, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxRadialCenterRadius, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxRadialFocalRadius, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxConicalCenterPointX, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxConicalCenterPointY, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(doubleSpinBoxConicalAngle, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));

#if QT_VERSION < 0x050000
	treeWidgetColorStops->header()->setMovable(false);
	treeWidgetColorStops->header()->setClickable(false);
	treeWidgetColorStops->header()->setResizeMode(QHeaderView::Fixed);
#else
	treeWidgetColorStops->header()->setSectionsMovable(false);
	treeWidgetColorStops->header()->setSectionsClickable(false);
	treeWidgetColorStops->header()->setSectionResizeMode(QHeaderView::Fixed);
#endif
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
		case Qt::NoBrush:
		default:
			return "NoBrush";
	}
}

QGradient::Spread BrushEditor::nameToGradientSpread(QString name)
{
	switch ( gradientSpreadNames.indexOf(name) ) {
		case QMC2_BRUSHEDITOR_GRADIENT_REPEATSPREAD:
			return QGradient::RepeatSpread;
		case QMC2_BRUSHEDITOR_GRADIENT_REFLECTSPREAD:
			return QGradient::ReflectSpread;
		case QMC2_BRUSHEDITOR_GRADIENT_PADSPREAD:
		default:
			return QGradient::PadSpread;
	}
}

QString BrushEditor::gradientSpreadToName(QGradient::Spread spread)
{
	switch ( spread ) {
		case QGradient::RepeatSpread:
			return "RepeatSpread";
		case QGradient::ReflectSpread:
			return "ReflectSpread";
		case QGradient::PadSpread:
		default:
			return "PadSpread";
	}
}

QGradient::Type BrushEditor::nameToGradientType(QString name)
{
	switch ( gradientTypeNames.indexOf(name) ) {
		case QMC2_BRUSHEDITOR_GRADIENT_RADIAL:
			return QGradient::RadialGradient;
		case QMC2_BRUSHEDITOR_GRADIENT_CONICAL:
			return QGradient::ConicalGradient;
		case QMC2_BRUSHEDITOR_GRADIENT_LINEAR:
		default:
			return QGradient::LinearGradient;
	}
}

QString BrushEditor::gradientTypeToName(QGradient::Type type)
{
	switch ( type ) {
		case QGradient::RadialGradient:
			return "Radial";
		case QGradient::ConicalGradient:
			return "Conical";
		case QGradient::LinearGradient:
		default:
			return "Linear";
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
	toolButtonAddColorStop->setIconSize(iconSize);
}

void BrushEditor::adjustWidgetSizes()
{
	int w = treeWidgetColorStops->viewport()->width() / treeWidgetColorStops->columnCount();
	for (int i = 0; i < treeWidgetColorStops->columnCount(); i++)
		treeWidgetColorStops->header()->resizeSection(i, w);
}

void BrushEditor::updateGradientPreview()
{
	if ( treeWidgetColorStops->topLevelItemCount() > 0 ) {
		QGradient gradient;
		switch ( comboBoxGradientType->currentIndex() ) {
			case QMC2_BRUSHEDITOR_GRADIENT_LINEAR:
				gradient = QLinearGradient(doubleSpinBoxLinearStartPointX->value(), doubleSpinBoxLinearStartPointY->value(), doubleSpinBoxLinearEndPointX->value(), doubleSpinBoxLinearEndPointY->value());
				break;
			case QMC2_BRUSHEDITOR_GRADIENT_RADIAL:
				gradient = QRadialGradient(doubleSpinBoxRadialCenterPointX->value(), doubleSpinBoxRadialCenterPointY->value(), doubleSpinBoxRadialCenterRadius->value(), doubleSpinBoxRadialFocalPointX->value(), doubleSpinBoxRadialFocalPointY->value(), doubleSpinBoxRadialFocalRadius->value());
				break;
			case QMC2_BRUSHEDITOR_GRADIENT_CONICAL:
				gradient = QConicalGradient(doubleSpinBoxConicalCenterPointX->value(), doubleSpinBoxConicalCenterPointY->value(), doubleSpinBoxConicalAngle->value());
				break;
		}
		gradient.setSpread(nameToGradientSpread(comboBoxSpreadType->currentText()));
		for (int i = 0; i < treeWidgetColorStops->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidgetColorStops->topLevelItem(i);
			QDoubleSpinBox *dsb = (QDoubleSpinBox *)treeWidgetColorStops->itemWidget(item, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_STOP);
			item->setData(QMC2_BRUSHEDITOR_GRADIENT_COLIDX_STOP, Qt::DisplayRole, dsb->textFromValue(dsb->value()));
			ColorWidget *cw = (ColorWidget *)treeWidgetColorStops->itemWidget(item, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_COLOR);
			gradient.setColorAt(dsb->value(), cw->frameBrush->palette().color(cw->frameBrush->backgroundRole()));
		}
		treeWidgetColorStops->sortItems(QMC2_BRUSHEDITOR_GRADIENT_COLIDX_STOP, Qt::AscendingOrder);
		QTimer::singleShot(0, this, SLOT(updateGradientStopActions()));
		QPalette pal = frameGradientPreview->palette();
		pal.setBrush(frameGradientPreview->backgroundRole(), QBrush(gradient));
		frameGradientPreview->setPalette(pal);
		frameGradientPreview->update();
	} else {
		frameGradientPreview->setPalette(qApp->palette());
		frameGradientPreview->update();
	}
}

void  BrushEditor::updateGradientStopActions()
{
	QTreeWidgetItem *lastItem = 0;
	for (int i = 0; i < treeWidgetColorStops->topLevelItemCount(); i++) {
		QTreeWidgetItem *thisItem = treeWidgetColorStops->topLevelItem(i);
		GradientStopActions *gsa = (GradientStopActions *)treeWidgetColorStops->itemWidget(thisItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_ACTIONS);
		if ( i == 0 ) {
			gsa->toolButtonUp->setEnabled(false);
			gsa->toolButtonDown->setEnabled(i < treeWidgetColorStops->topLevelItemCount() - 1);
		} else if ( i == treeWidgetColorStops->topLevelItemCount() - 1 ) {
			gsa->toolButtonUp->setEnabled(i > 0);
			gsa->toolButtonDown->setEnabled(false);
		} else {
			gsa->toolButtonUp->setEnabled(true);
			gsa->toolButtonDown->setEnabled(true);
		}
		if ( lastItem ) {
			GradientStopActions *gsa1 = (GradientStopActions *)treeWidgetColorStops->itemWidget(lastItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_ACTIONS);
			QDoubleSpinBox *dsb1 = (QDoubleSpinBox *)treeWidgetColorStops->itemWidget(lastItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_STOP);
			QDoubleSpinBox *dsb2 = (QDoubleSpinBox *)treeWidgetColorStops->itemWidget(thisItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_STOP);
			ColorWidget *cw1 = (ColorWidget *)treeWidgetColorStops->itemWidget(lastItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_COLOR);
			treeWidgetColorStops->setTabOrder(dsb1, cw1->toolButtonColor);
			treeWidgetColorStops->setTabOrder(cw1->toolButtonColor, gsa1->toolButtonUp);
			treeWidgetColorStops->setTabOrder(gsa1->toolButtonUp, gsa1->toolButtonDown);
			treeWidgetColorStops->setTabOrder(gsa1->toolButtonDown, gsa1->toolButtonRemove);
			treeWidgetColorStops->setTabOrder(gsa1->toolButtonRemove, dsb2);
		}
		lastItem = thisItem;
	}

	QTimer::singleShot(0, this, SLOT(adjustWidgetSizes()));
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
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose image file"), lineEditImageFile->text(), tr("Supported image files (%1)").arg(imageFileTypes.join(" ")) + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
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

void BrushEditor::on_toolButtonAddColorStop_clicked()
{
	QTreeWidgetItem *newItem = new QTreeWidgetItem(treeWidgetColorStops);
	QDoubleSpinBox *dsb = new QDoubleSpinBox();
	dsb->setDecimals(2);
	dsb->setSingleStep(0.01);
	dsb->setRange(0.0, 1.0);
	dsb->setValue(0.0);
	ColorWidget *cw = new ColorWidget(QString(), QString(), QPalette::Normal, QPalette::NoRole, QColor("black"), QBrush(), 0, false, true);
	GradientStopActions *gsa = new GradientStopActions(newItem);
	treeWidgetColorStops->setItemWidget(newItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_STOP, dsb);
	treeWidgetColorStops->setItemWidget(newItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_COLOR, cw);
	treeWidgetColorStops->setItemWidget(newItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_ACTIONS, gsa);
	connect(dsb, SIGNAL(valueChanged(double)), this, SLOT(updateGradientPreview()));
	connect(cw, SIGNAL(colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor)), this, SLOT(updateGradientPreview()));
	connect(gsa, SIGNAL(removeRequested(QTreeWidgetItem *)), this, SLOT(gradientStopAction_removeRequested(QTreeWidgetItem *)));
	connect(gsa, SIGNAL(upRequested(QTreeWidgetItem *)), this, SLOT(gradientStopAction_upRequested(QTreeWidgetItem *)));
	connect(gsa, SIGNAL(downRequested(QTreeWidgetItem *)), this, SLOT(gradientStopAction_downRequested(QTreeWidgetItem *)));

	QTimer::singleShot(0, this, SLOT(updateGradientPreview()));
}

void BrushEditor::gradientStopAction_removeRequested(QTreeWidgetItem *item)
{
	if ( item ) {
		item = treeWidgetColorStops->takeTopLevelItem(treeWidgetColorStops->indexOfTopLevelItem(item));
		delete item;
		QTimer::singleShot(0, this, SLOT(updateGradientPreview()));
	}
}

void BrushEditor::gradientStopAction_upRequested(QTreeWidgetItem *item)
{
	if ( item ) {
		int index = treeWidgetColorStops->indexOfTopLevelItem(item);
		ColorWidget *thisCw = (ColorWidget *)treeWidgetColorStops->itemWidget(item, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_COLOR);
		QTreeWidgetItem *otherItem = treeWidgetColorStops->topLevelItem(index - 1);
		ColorWidget *otherCw = (ColorWidget *)treeWidgetColorStops->itemWidget(otherItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_COLOR);
		QColor otherColor = otherCw->frameBrush->palette().color(otherCw->frameBrush->backgroundRole());
		QColor thisColor = thisCw->frameBrush->palette().color(thisCw->frameBrush->backgroundRole());
		QPalette pal = otherCw->frameBrush->palette();
		pal.setColor(otherCw->frameBrush->backgroundRole(), thisColor);
		otherCw->frameBrush->setPalette(pal);
		otherCw->frameBrush->update();
		pal = thisCw->frameBrush->palette();
		pal.setColor(thisCw->frameBrush->backgroundRole(), otherColor);
		thisCw->frameBrush->setPalette(pal);
		thisCw->frameBrush->update();
		QTimer::singleShot(0, this, SLOT(updateGradientPreview()));
	}
}

void BrushEditor::gradientStopAction_downRequested(QTreeWidgetItem *item)
{
	if ( item ) {
		int index = treeWidgetColorStops->indexOfTopLevelItem(item);
		ColorWidget *thisCw = (ColorWidget *)treeWidgetColorStops->itemWidget(item, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_COLOR);
		QTreeWidgetItem *otherItem = treeWidgetColorStops->topLevelItem(index + 1);
		ColorWidget *otherCw = (ColorWidget *)treeWidgetColorStops->itemWidget(otherItem, QMC2_BRUSHEDITOR_GRADIENT_COLIDX_COLOR);
		QColor otherColor = otherCw->frameBrush->palette().color(otherCw->frameBrush->backgroundRole());
		QColor thisColor = thisCw->frameBrush->palette().color(thisCw->frameBrush->backgroundRole());
		QPalette pal = otherCw->frameBrush->palette();
		pal.setColor(otherCw->frameBrush->backgroundRole(), thisColor);
		otherCw->frameBrush->setPalette(pal);
		otherCw->frameBrush->update();
		pal = thisCw->frameBrush->palette();
		pal.setColor(thisCw->frameBrush->backgroundRole(), otherColor);
		thisCw->frameBrush->setPalette(pal);
		thisCw->frameBrush->update();
		QTimer::singleShot(0, this, SLOT(updateGradientPreview()));
	}
}

void BrushEditor::on_comboBoxPatternType_currentIndexChanged(int /*index*/)
{
	QPalette pal = framePatternPreview->palette();
	pal.setBrush(framePatternPreview->backgroundRole(), QBrush(framePatternColor->palette().color(framePatternColor->backgroundRole()), patternNameToStyle(comboBoxPatternType->currentText())));
	framePatternPreview->setPalette(pal);
	framePatternPreview->update();
}

void BrushEditor::on_comboBoxGradientType_currentIndexChanged(int)
{
	QTimer::singleShot(0, this, SLOT(updateGradientPreview()));
}

void BrushEditor::on_comboBoxSpreadType_currentIndexChanged(int)
{
	QTimer::singleShot(0, this, SLOT(updateGradientPreview()));
}

void BrushEditor::on_toolBox_currentChanged(int index)
{
	switch ( index ) {
		case QMC2_BRUSHEDITOR_PAGEINDEX_IMAGE:
			break;
		case QMC2_BRUSHEDITOR_PAGEINDEX_PATTERN:
			break;
		case QMC2_BRUSHEDITOR_PAGEINDEX_GRADIENT:
			QTimer::singleShot(0, this, SLOT(adjustWidgetSizes()));
			break;
	}
}

void BrushEditor::showEvent(QShowEvent *e)
{
	adjustIconSizes();

	if ( e )
		QDialog::showEvent(e);
}

void BrushEditor::resizeEvent(QResizeEvent *e)
{
	QTimer::singleShot(0, this, SLOT(adjustWidgetSizes()));

	if ( e )
		QDialog::resizeEvent(e);
}
