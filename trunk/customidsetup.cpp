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
	bool enable = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/SortingEnabled", true).toBool();
	tableWidgetCustomIDs->setSortingEnabled(enable);
	tableWidgetCustomIDs->setDragDropMode(enable ? QAbstractItemView::NoDragDrop : QAbstractItemView::InternalMove);
	toolButtonSort->setChecked(enable);

	QTimer::singleShot(0, this, SLOT(load()));
}

CustomIDSetup::~CustomIDSetup()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::~CustomIDSetup()");
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/Size", size());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/IDListHeaderState", tableWidgetCustomIDs->horizontalHeader()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/SortingEnabled", toolButtonSort->isChecked());
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
	toolButtonSort->setIconSize(iconSize);
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

	bool oldSortingEnabled = tableWidgetCustomIDs->isSortingEnabled();
	tableWidgetCustomIDs->setSortingEnabled(false);
	tableWidgetCustomIDs->insertRow(row);
	tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_ID, new QTableWidgetItem());
	tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_DESCRIPTION, new QTableWidgetItem());
	tableWidgetCustomIDs->setSortingEnabled(oldSortingEnabled);
}

void CustomIDSetup::on_toolButtonRemoveID_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::on_toolButtonRemoveID_clicked()");
#endif

	QList<QTableWidgetItem *> selectedItems = tableWidgetCustomIDs->selectedItems();
	if ( selectedItems.count() > 0 )
		tableWidgetCustomIDs->removeRow(tableWidgetCustomIDs->row(selectedItems[0]));
	else if ( tableWidgetCustomIDs->currentRow() >= 0 )
		tableWidgetCustomIDs->removeRow(tableWidgetCustomIDs->currentRow());
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
		toolButtonRemoveID->setEnabled(tableWidgetCustomIDs->currentRow() >= 0);
	}
}

void CustomIDSetup::on_tableWidgetCustomIDs_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: CustomIDSetup::on_tableWidgetCustomIDs_currentItemChanged(QTableWidgetItem *current = %1, QTableWidgetItem *previous = %2)").arg((qulonglong)current).arg((qulonglong)previous));
#endif

	on_tableWidgetCustomIDs_itemSelectionChanged();
}

void CustomIDSetup::on_toolButtonSort_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: CustomIDSetup::on_toolButtonSort_toggled(bool enable = %1)").arg(enable));
#endif

	tableWidgetCustomIDs->setSortingEnabled(enable);
	tableWidgetCustomIDs->setDragDropMode(enable ? QAbstractItemView::NoDragDrop : QAbstractItemView::InternalMove);
	if ( enable )
		tableWidgetCustomIDs->sortByColumn(tableWidgetCustomIDs->horizontalHeader()->sortIndicatorSection(), tableWidgetCustomIDs->horizontalHeader()->sortIndicatorOrder());
}

void CustomIDSetup::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::load()");
#endif

	QString groupKey = QString(QMC2_EMULATOR_PREFIX + "CustomIDs/%1").arg(foreignEmulator);

	qmc2Config->beginGroup(groupKey);
	QStringList idList = qmc2Config->value("IDs", QStringList()).toStringList();
	QStringList descriptionList = qmc2Config->value("Descriptions", QStringList()).toStringList();
	qmc2Config->endGroup();

	tableWidgetCustomIDs->clearContents();
	bool oldSortingEnabled = tableWidgetCustomIDs->isSortingEnabled();
	tableWidgetCustomIDs->setSortingEnabled(false);
	int row = 0;
	for (int i = 0; i < idList.count(); i++) {
		QString id = idList[i];
		if ( !id.isEmpty() ) {
			QString description = descriptionList[i];
			tableWidgetCustomIDs->insertRow(row);
			tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_ID, new QTableWidgetItem(id));
			tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_DESCRIPTION, new QTableWidgetItem(description));
			row++;
		}
	}
	tableWidgetCustomIDs->setSortingEnabled(oldSortingEnabled);
	if ( oldSortingEnabled )
		tableWidgetCustomIDs->sortByColumn(tableWidgetCustomIDs->horizontalHeader()->sortIndicatorSection(), tableWidgetCustomIDs->horizontalHeader()->sortIndicatorOrder());
}

void CustomIDSetup::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::save()");
#endif

	QString groupKey = QString(QMC2_EMULATOR_PREFIX + "CustomIDs/%1").arg(foreignEmulator);
	qmc2Config->remove(groupKey);

	QStringList idList, descriptionList;
	for (int row = 0; row < tableWidgetCustomIDs->rowCount(); row++) {
		QTableWidgetItem *idItem = tableWidgetCustomIDs->item(row, QMC2_CUSTOMIDS_COLUMN_ID);
		QTableWidgetItem *descriptionItem = tableWidgetCustomIDs->item(row, QMC2_CUSTOMIDS_COLUMN_DESCRIPTION);
		if ( idItem ) {
			QString id = idItem->text();
			QString description;
			if ( descriptionItem )
				description = descriptionItem->text();
			if ( !id.isEmpty() ) {
				idList << id;
				descriptionList << description;
			}
		}
	}

	if ( !idList.isEmpty() ) {
		qmc2Config->beginGroup(groupKey);
		qmc2Config->setValue("IDs", idList);
		qmc2Config->setValue("Descriptions", descriptionList);
		qmc2Config->endGroup();
	}
}
