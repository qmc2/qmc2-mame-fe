#include <QFont>
#include <QFontMetrics>
#include <QStringList>
#include <QApplication>

#include "settings.h"
#include "softwarestatefilter.h"
#include "softwarelist.h"
#include "qmc2main.h"
#include "machinelist.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern SoftwareList *qmc2SoftwareList;
extern MachineList *qmc2MachineList;

SoftwareStateFilter::SoftwareStateFilter(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	adjustIconSizes();

	isReady = false;

	checkBoxStateFilter->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/Enabled", false).toBool());
	toolButtonCorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowCorrect", true).toBool());
	toolButtonMostlyCorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowMostlyCorrect", true).toBool());
	toolButtonIncorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowIncorrect", true).toBool());
	toolButtonNotFound->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowNotFound", true).toBool());
	toolButtonUnknown->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareStateFilter/ShowUnknown", true).toBool());

	isReady = true;
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

void SoftwareStateFilter::filter()
{
	if ( !isReady )
		return;

	qmc2SoftwareList->progressBar->setFormat(tr("State filter - %p%"));
	qmc2SoftwareList->progressBar->setRange(0, qmc2SoftwareList->treeWidgetKnownSoftware->topLevelItemCount());
	qmc2SoftwareList->progressBar->setValue(0);
	qmc2SoftwareList->progressBar->setVisible(true);
	qmc2SoftwareList->treeWidgetKnownSoftware->setUpdatesEnabled(false);
	QStringList hiddenLists = qmc2MachineList->userDataDb()->hiddenLists(qmc2SoftwareList->systemName);
	if ( checkBoxStateFilter->isChecked() ) {
		for (int i = 0; i < qmc2SoftwareList->treeWidgetKnownSoftware->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = qmc2SoftwareList->treeWidgetKnownSoftware->topLevelItem(i);
			if ( hiddenLists.contains(item->text(QMC2_SWLIST_COLUMN_LIST)) )
				item->setHidden(true);
			else switch ( item->whatsThis(QMC2_SWLIST_COLUMN_NAME).at(0).toLatin1() ) {
				case 'C':
					item->setHidden(!toolButtonCorrect->isChecked());
					break;
				case 'M':
					item->setHidden(!toolButtonMostlyCorrect->isChecked());
					break;
				case 'I':
					item->setHidden(!toolButtonIncorrect->isChecked());
					break;
				case 'N':
					item->setHidden(!toolButtonNotFound->isChecked());
					break;
				case 'U':
				default:
					item->setHidden(!toolButtonUnknown->isChecked());
					break;
			}
			qmc2SoftwareList->progressBar->setValue(i + 1);
		}
	} else {
		for (int i = 0; i < qmc2SoftwareList->treeWidgetKnownSoftware->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = qmc2SoftwareList->treeWidgetKnownSoftware->topLevelItem(i);
			qmc2SoftwareList->treeWidgetKnownSoftware->topLevelItem(i)->setHidden(hiddenLists.contains(item->text(QMC2_SWLIST_COLUMN_LIST)));
			qmc2SoftwareList->progressBar->setValue(i + 1);
		}
	}
	if ( qmc2SoftwareList->toolButtonCompatFilterToggle->isChecked() )
		qmc2SoftwareList->on_toolButtonCompatFilterToggle_clicked(true);
	qmc2SoftwareList->treeWidgetKnownSoftware->setUpdatesEnabled(true);
	qmc2SoftwareList->progressBar->setVisible(false);
}

void SoftwareStateFilter::on_checkBoxStateFilter_toggled(bool checked)
{
	if ( !isReady )
		return;

	QString itemText = qmc2SoftwareList->toolBoxSoftwareList->itemText(QMC2_SWLIST_KNOWN_SW_PAGE);
	itemText.remove(QRegExp(" \\| " + tr("filtered") + "$"));
	if ( checked )
		qmc2SoftwareList->toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, itemText + " | " + tr("filtered"));
	else
		qmc2SoftwareList->toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, itemText);

	filter();
}

void SoftwareStateFilter::on_toolButtonCorrect_toggled(bool /*checked*/)
{
	filter();
}

void SoftwareStateFilter::on_toolButtonMostlyCorrect_toggled(bool /*checked*/)
{
	filter();
}

void SoftwareStateFilter::on_toolButtonIncorrect_toggled(bool /*checked*/)
{
	filter();
}

void SoftwareStateFilter::on_toolButtonNotFound_toggled(bool /*checked*/)
{
	filter();
}

void SoftwareStateFilter::on_toolButtonUnknown_toggled(bool /*checked*/)
{
	filter();
}

void SoftwareStateFilter::showEvent(QShowEvent *e)
{
	adjustIconSizes();

	if ( e )
		QWidget::showEvent(e);
}

void SoftwareStateFilter::enterEvent(QEvent *e)
{
	qmc2MainWindow->statusBar()->showMessage(tr("State filter settings"));
	QWidget::enterEvent(e);
}

void SoftwareStateFilter::leaveEvent(QEvent *e)
{
	qmc2MainWindow->statusBar()->clearMessage();
	QWidget::leaveEvent(e);
}
