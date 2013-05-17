#include <QSettings>

#include "additionalartworksetup.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

AdditionalArtworkSetup::AdditionalArtworkSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

AdditionalArtworkSetup::~AdditionalArtworkSetup()
{
}

void AdditionalArtworkSetup::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	pushButtonRestore->setIconSize(iconSize);
	toolButtonAdd->setIconSize(iconSize);
	toolButtonRemove->setIconSize(iconSize);
}

void AdditionalArtworkSetup::on_pushButtonOk_clicked()
{
	// FIXME
	accept();
}

void AdditionalArtworkSetup::on_pushButtonCancel_clicked()
{
	// FIXME
	reject();
}

void AdditionalArtworkSetup::on_pushButtonRestore_clicked()
{
	// FIXME
}

void AdditionalArtworkSetup::on_toolButtonAdd_clicked()
{
	// FIXME
}

void AdditionalArtworkSetup::on_toolButtonRemove_clicked()
{
	// FIXME
}

void AdditionalArtworkSetup::showEvent(QShowEvent *e)
{
	// FIXME
	adjustIconSizes();
	adjustSize();

	if ( e )
		QDialog::showEvent(e);
}

void AdditionalArtworkSetup::resizeEvent(QResizeEvent *e)
{
	// FIXME
	if ( e )
		QDialog::resizeEvent(e);
}

void AdditionalArtworkSetup::hideEvent(QHideEvent *e)
{
	// FIXME
	if ( e )
		QDialog::hideEvent(e);
}
