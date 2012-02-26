#include "customidsetup.h"
#include "macros.h"

#ifndef QMC2_DEBUG
#define QMC2_DEBUG
#endif

#if defined(QMC2_DEBUG)
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif
extern QSettings *qmc2Config;

CustomIDSetup::CustomIDSetup(QString foreignEmulatorName, QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: CustomIDSetup::CustomIDSetup(QString foreignEmulatorName = %1, QWidget *parent = %2)").arg(foreignEmulatorName).arg((qulonglong) parent));
#endif

	setupUi(this);
	adjustFontAndIconSizes();
	foreignEmulator = foreignEmulatorName;
	setWindowTitle(tr("Setup custom IDs for '%1'").arg(foreignEmulator));

	QSize widgetSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/Size", QSize()).toSize();
	if ( !widgetSize.isEmpty() )
		resize(widgetSize);
	QByteArray headerState = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/IDListHeaderState", QByteArray()).toByteArray();
	if ( !headerState.isEmpty() )
		tableWidgetCustomIDs->horizontalHeader()->restoreState(headerState);

}

CustomIDSetup::~CustomIDSetup()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::~CustomIDSetup()");
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/Size", size());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/IDListHeaderState", tableWidgetCustomIDs->horizontalHeader()->saveState());
}

void CustomIDSetup::adjustFontAndIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::adjustFontAndIconSizes()");
#endif

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	setFont(f);
	QFontMetrics fm(f);
	QSize iconSize(fm.height() + 2, fm.height() + 2);
	toolButtonAddID->setIconSize(iconSize);
	toolButtonRemoveID->setIconSize(iconSize);
	toolButtonCopyIDs->setIconSize(iconSize);
	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
}

void CustomIDSetup::on_toolButtonAddID_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::on_toolButtonAddID_clicked()");
#endif

	int row = tableWidgetCustomIDs->rowCount();

	tableWidgetCustomIDs->setSortingEnabled(false);
	tableWidgetCustomIDs->insertRow(row);
	tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_ID, new QTableWidgetItem());
	tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_DESCRIPTION, new QTableWidgetItem());
	tableWidgetCustomIDs->setSortingEnabled(true);
}

void CustomIDSetup::on_toolButtonRemoveID_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::on_toolButtonRemoveID_clicked()");
#endif

	QList<QTableWidgetItem *> selectedItems = tableWidgetCustomIDs->selectedItems();
	if ( selectedItems.count() > 0 )
		tableWidgetCustomIDs->removeRow(tableWidgetCustomIDs->row(selectedItems[0]));
}

void CustomIDSetup::on_tableWidgetCustomIDs_itemSelectionChanged()
{
	QList<QTableWidgetItem *> selectedItems = tableWidgetCustomIDs->selectedItems();
	if ( selectedItems.count() > 0 ) {
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: CustomIDSetup::on_tableWidgetCustomIDs_itemSelectionChanged(): item = %1").arg((qulonglong)selectedItems[0]));
#endif
		toolButtonRemoveID->setEnabled(true);
	} else {
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: CustomIDSetup::on_tableWidgetCustomIDs_itemSelectionChanged(): item = NULL"));
#endif
		toolButtonRemoveID->setEnabled(false);
	}
}
