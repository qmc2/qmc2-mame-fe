#include <QSettings>

#include "imageformatsetup.h"
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

void ImageFormatSetup::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	adjustSize();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/Geometry", QByteArray()).toByteArray());

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
