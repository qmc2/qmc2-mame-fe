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
	adjustIconSizes();

	checkBoxStateFilter->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/Enabled", false).toBool());
	toolButtonCorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowCorrect", true).toBool());
	toolButtonMostlyCorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowMostlyCorrect", true).toBool());
	toolButtonIncorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowIncorrect", true).toBool());
	toolButtonNotFound->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowNotFound", true).toBool());
	toolButtonUnknown->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowUnknown", true).toBool());
}

SoftwareStateFilter::~SoftwareStateFilter()
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/Enabled", checkBoxStateFilter->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowCorrect", toolButtonCorrect->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowMostlyCorrect", toolButtonMostlyCorrect->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowIncorrect", toolButtonIncorrect->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowNotFound", toolButtonNotFound->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowUnknown", toolButtonUnknown->isChecked());
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

void SoftwareStateFilter::on_checkBoxStateFilter_toggled(bool checked)
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
