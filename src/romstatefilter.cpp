#include <QFont>
#include <QFontMetrics>
#include <QStringList>
#include <QBitArray>

#include "settings.h"
#include "options.h"
#include "romstatefilter.h"
#include "qmc2main.h"
#include "machinelist.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;
extern MachineList *qmc2MachineList;
extern QBitArray qmc2Filter;
extern bool qmc2StatesTogglesEnabled;

RomStateFilter::RomStateFilter(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	adjustIconSizes();
	toolButtonCorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowCorrect", true).toBool());
	actionShowCorrect = new QAction(this);
	actionShowCorrect->setCheckable(true);
	actionShowCorrect->setChecked(toolButtonCorrect->isChecked());
	actionShowCorrect->setShortcut(QKeySequence("Ctrl+Alt+C"));
	actionShowCorrect->setShortcutContext(Qt::ApplicationShortcut);
	parentWidget()->addAction(actionShowCorrect);
	connect(actionShowCorrect, SIGNAL(triggered(bool)), this, SLOT(on_toolButtonCorrect_toggled(bool)));
	toolButtonMostlyCorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowMostlyCorrect", true).toBool());
	actionShowMostlyCorrect = new QAction(this);
	actionShowMostlyCorrect->setCheckable(true);
	actionShowMostlyCorrect->setChecked(toolButtonMostlyCorrect->isChecked());
	actionShowMostlyCorrect->setShortcut(QKeySequence("Ctrl+Alt+M"));
	actionShowMostlyCorrect->setShortcutContext(Qt::ApplicationShortcut);
	parentWidget()->addAction(actionShowMostlyCorrect);
	connect(actionShowMostlyCorrect, SIGNAL(triggered(bool)), this, SLOT(on_toolButtonMostlyCorrect_toggled(bool)));
	toolButtonIncorrect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowIncorrect", true).toBool());
	actionShowIncorrect = new QAction(this);
	actionShowIncorrect->setCheckable(true);
	actionShowIncorrect->setChecked(toolButtonIncorrect->isChecked());
	actionShowIncorrect->setShortcut(QKeySequence("Ctrl+Alt+I"));
	actionShowIncorrect->setShortcutContext(Qt::ApplicationShortcut);
	parentWidget()->addAction(actionShowIncorrect);
	connect(actionShowIncorrect, SIGNAL(triggered(bool)), this, SLOT(on_toolButtonIncorrect_toggled(bool)));
	toolButtonNotFound->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowNotFound", true).toBool());
	actionShowNotFound = new QAction(this);
	actionShowNotFound->setCheckable(true);
	actionShowNotFound->setChecked(toolButtonNotFound->isChecked());
	actionShowNotFound->setShortcut(QKeySequence("Ctrl+Alt+N"));
	actionShowNotFound->setShortcutContext(Qt::ApplicationShortcut);
	parentWidget()->addAction(actionShowNotFound);
	connect(actionShowNotFound, SIGNAL(triggered(bool)), this, SLOT(on_toolButtonNotFound_toggled(bool)));
	toolButtonUnknown->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowUnknown", true).toBool());
	actionShowUnknown = new QAction(this);
	actionShowUnknown->setCheckable(true);
	actionShowUnknown->setChecked(toolButtonUnknown->isChecked());
	actionShowUnknown->setShortcut(QKeySequence("Ctrl+Alt+U"));
	actionShowUnknown->setShortcutContext(Qt::ApplicationShortcut);
	parentWidget()->addAction(actionShowUnknown);
	connect(actionShowUnknown, SIGNAL(triggered(bool)), this, SLOT(on_toolButtonUnknown_toggled(bool)));
}

RomStateFilter::~RomStateFilter()
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowCorrect", toolButtonCorrect->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowMostlyCorrect", toolButtonMostlyCorrect->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowIncorrect", toolButtonIncorrect->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowNotFound", toolButtonNotFound->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowUnknown", toolButtonUnknown->isChecked());
}

void RomStateFilter::adjustIconSizes()
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

void RomStateFilter::on_toolButtonCorrect_toggled(bool checked)
{
	if ( !qmc2StatesTogglesEnabled )
		return;
	qmc2Options->toolButtonShowC->setChecked(checked);
	toolButtonCorrect->blockSignals(true);
	toolButtonCorrect->setChecked(checked);
	toolButtonCorrect->blockSignals(false);
	actionShowCorrect->blockSignals(true);
	actionShowCorrect->setChecked(checked);
	actionShowCorrect->blockSignals(false);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowCorrect", checked);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_C, checked);
	qmc2MachineList->verifyCurrentOnly = false;
	qmc2MachineList->filter();
}

void RomStateFilter::on_toolButtonMostlyCorrect_toggled(bool checked)
{
	if ( !qmc2StatesTogglesEnabled )
		return;
	qmc2Options->toolButtonShowM->setChecked(checked);
	toolButtonMostlyCorrect->blockSignals(true);
	toolButtonMostlyCorrect->setChecked(checked);
	toolButtonMostlyCorrect->blockSignals(false);
	actionShowMostlyCorrect->blockSignals(true);
	actionShowMostlyCorrect->setChecked(checked);
	actionShowMostlyCorrect->blockSignals(false);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowMostlyCorrect", checked);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_M, checked);
	qmc2MachineList->verifyCurrentOnly = false;
	qmc2MachineList->filter();
}

void RomStateFilter::on_toolButtonIncorrect_toggled(bool checked)
{
	if ( !qmc2StatesTogglesEnabled )
		return;
	qmc2Options->toolButtonShowI->setChecked(checked);
	toolButtonIncorrect->blockSignals(true);
	toolButtonIncorrect->setChecked(checked);
	toolButtonIncorrect->blockSignals(false);
	actionShowIncorrect->blockSignals(true);
	actionShowIncorrect->setChecked(checked);
	actionShowIncorrect->blockSignals(false);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowIncorrect", checked);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_I, checked);
	qmc2MachineList->verifyCurrentOnly = false;
	qmc2MachineList->filter();
}

void RomStateFilter::on_toolButtonNotFound_toggled(bool checked)
{
	if ( !qmc2StatesTogglesEnabled )
		return;
	qmc2Options->toolButtonShowN->setChecked(checked);
	toolButtonNotFound->blockSignals(true);
	toolButtonNotFound->setChecked(checked);
	toolButtonNotFound->blockSignals(false);
	actionShowNotFound->blockSignals(true);
	actionShowNotFound->setChecked(checked);
	actionShowNotFound->blockSignals(false);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowNotFound", checked);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_N, checked);
	qmc2MachineList->verifyCurrentOnly = false;
	qmc2MachineList->filter();
}

void RomStateFilter::on_toolButtonUnknown_toggled(bool checked)
{
	if ( !qmc2StatesTogglesEnabled )
		return;
	qmc2Options->toolButtonShowU->setChecked(checked);
	toolButtonUnknown->blockSignals(true);
	toolButtonUnknown->setChecked(checked);
	toolButtonUnknown->blockSignals(false);
	actionShowUnknown->blockSignals(true);
	actionShowUnknown->setChecked(checked);
	actionShowUnknown->blockSignals(false);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowUnknown", checked);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_U, checked);
	qmc2MachineList->verifyCurrentOnly = false;
	qmc2MachineList->filter();
}

void RomStateFilter::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	QWidget::showEvent(e);
}

void RomStateFilter::enterEvent(QEvent *e)
{
	qmc2MainWindow->statusBar()->showMessage(tr("State filter settings"));
	QWidget::enterEvent(e);
}

void RomStateFilter::leaveEvent(QEvent *e)
{
	qmc2MainWindow->statusBar()->clearMessage();
	QWidget::leaveEvent(e);
}
