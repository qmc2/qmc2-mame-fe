#include <QFont>
#include <QFontMetrics>
#include <QSettings>
#include <QApplication>

#include "softwarestatefilter.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

SoftwareStateFilter::SoftwareStateFilter(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	// FIXME
}

SoftwareStateFilter::~SoftwareStateFilter()
{
}

void SoftwareStateFilter::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height(), fm.height());

	toolButtonCorrect->setIconSize(iconSize);
	toolButtonMostlyCorrect->setIconSize(iconSize);
	toolButtonIncorrect->setIconSize(iconSize);
	toolButtonNotFound->setIconSize(iconSize);
	toolButtonUnknown->setIconSize(iconSize);
}

void SoftwareStateFilter::on_checkBoxStateFilter_stateChanged(int state)
{
}

void SoftwareStateFilter::on_toolButtonCorrect_toggled(bool checked)
{
}

void SoftwareStateFilter::on_toolButtonMostlyCorrect_toggled(bool checked)
{
}

void SoftwareStateFilter::on_toolButtonIncorrect_toggled(bool checked)
{
}

void SoftwareStateFilter::on_toolButtonNotFound_toggled(bool checked)
{
}

void SoftwareStateFilter::on_toolButtonUnknown_toggled(bool checked)
{
}

void SoftwareStateFilter::showEvent(QShowEvent *e)
{
	adjustIconSizes();

	if ( e )
		QWidget::showEvent(e);
}
