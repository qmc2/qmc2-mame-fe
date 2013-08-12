#include <QSettings>

#include "imageformatsetup.h"
#include "imagewidget.h"
#include "qmc2main.h"
#include "options.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern Options *qmc2Options;

ImageFormatSetup::ImageFormatSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

#if QT_VERSION < 0x050000
	treeWidget->header()->setMovable(false);
#else
	treeWidget->header()->setSectionsMovable(false);
#endif
}

ImageFormatSetup::~ImageFormatSetup()
{
}

void ImageFormatSetup::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	pushButtonRestore->setIconSize(iconSize);
	comboBoxImageType->setIconSize(iconSize);
}

void ImageFormatSetup::on_pushButtonOk_clicked()
{
	// FIXME
	accept();
}

void ImageFormatSetup::on_pushButtonCancel_clicked()
{
	// FIXME
	reject();
}

void ImageFormatSetup::on_pushButtonRestore_clicked()
{
	// FIXME
}

void ImageFormatSetup::on_comboBoxImageType_currentIndexChanged(int index)
{
	treeWidget->clear();
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/ImageType", index);
	for (int i = 0; i < QMC2_IMAGE_FORMAT_COUNT; i++) {
		QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		item->setText(QMC2_IMGFMT_SETUP_COLUMN_NAME, ImageWidget::formatNames[i]);
		item->setText(QMC2_IMGFMT_SETUP_COLUMN_DESC, ImageWidget::formatDescriptions[i]);
		item->setText(QMC2_IMGFMT_SETUP_COLUMN_EXT, ImageWidget::formatExtensions[i]);
		item->setCheckState(QMC2_IMGFMT_SETUP_COLUMN_ACT, Qt::Unchecked);
	}
	treeWidget->resizeColumnToContents(QMC2_IMGFMT_SETUP_COLUMN_NAME);
	int w = treeWidget->viewport()->width() - treeWidget->columnWidth(QMC2_IMGFMT_SETUP_COLUMN_NAME);
	treeWidget->setColumnWidth(QMC2_IMGFMT_SETUP_COLUMN_DESC, w/2);
	treeWidget->setColumnWidth(QMC2_IMGFMT_SETUP_COLUMN_EXT, w/4);
	treeWidget->setColumnWidth(QMC2_IMGFMT_SETUP_COLUMN_ACT, w/4);
}

void ImageFormatSetup::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	adjustSize();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/Geometry", QByteArray()).toByteArray());

	comboBoxImageType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/ImageType", QMC2_IMAGE_FORMAT_INDEX_PNG).toInt());
	on_comboBoxImageType_currentIndexChanged(comboBoxImageType->currentIndex());

	if ( e )
		QDialog::showEvent(e);
}

void ImageFormatSetup::resizeEvent(QResizeEvent *e)
{
	// FIXME
	if ( e )
		QDialog::resizeEvent(e);
}

void ImageFormatSetup::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/Geometry", saveGeometry());
	on_pushButtonCancel_clicked();

	if ( e )
		QDialog::hideEvent(e);
}
