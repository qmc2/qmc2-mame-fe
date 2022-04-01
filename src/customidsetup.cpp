#include <QtCore>
#include <QMenu>
#include <QApplication>
#include <QToolButton>
#include <QFontMetrics>
#include <QAction>
#include <QFileDialog>

#include "settings.h"
#include "options.h"
#include "customidsetup.h"
#include "macros.h"

extern Settings *qmc2Config;
extern Options *qmc2Options;

CustomIDSetup::CustomIDSetup(QString foreignEmulatorName, QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	copyIDsMenu = 0;
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
	QString displayFormat = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/DisplayFormat", "$ID$ - $DESCRIPTION$").toString();
	if ( !displayFormat.isEmpty() ) {
		int index = comboBoxDisplayFormat->findText(displayFormat);
		if ( index >= 0 ) {
			comboBoxDisplayFormat->setCurrentIndex(index);
		} else {
			comboBoxDisplayFormat->addItem(displayFormat);
			comboBoxDisplayFormat->setCurrentIndex(comboBoxDisplayFormat->count() - 1);
		}
	}

	QTimer::singleShot(0, this, SLOT(load()));
	QTimer::singleShot(0, this, SLOT(setupCopyIDsMenu()));
}

CustomIDSetup::~CustomIDSetup()
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/Size", size());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/IDListHeaderState", tableWidgetCustomIDs->horizontalHeader()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/SortingEnabled", toolButtonSort->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/DisplayFormat", comboBoxDisplayFormat->currentText());
}

void CustomIDSetup::adjustFontAndIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	setFont(f);
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonAddID->setIconSize(iconSize);
	toolButtonRemoveID->setIconSize(iconSize);
	toolButtonSort->setIconSize(iconSize);
	toolButtonCopyIDs->setIconSize(iconSize);
	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
}

void CustomIDSetup::setupCopyIDsMenu()
{
	qmc2Config->beginGroup(QMC2_EMULATOR_PREFIX + "CustomIDs");
	QStringList childGroups = qmc2Config->childGroups();

	if ( !childGroups.isEmpty() ) {
		copyIDsMenu = new QMenu(this);
		int emuCount = 0;
		foreach (QString emuName, childGroups) {
			if ( emuName != this->foreignEmulator ) {
				QAction *action = copyIDsMenu->addAction(emuName);
				action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
				connect(action, SIGNAL(triggered()), this, SLOT(action_copyIDsMenuItem_triggered()));
				emuCount++;
			}
		}
		toolButtonCopyIDs->setMenu(copyIDsMenu);
		if ( emuCount > 0 )
			toolButtonCopyIDs->setEnabled(true);
	}

	qmc2Config->endGroup();
}

void CustomIDSetup::action_copyIDsMenuItem_triggered()
{
	QAction *action = (QAction *)sender();
	QString emuName = action->iconText();
	if ( !emuName.isEmpty() ) {
		bool oldSortingEnabled = tableWidgetCustomIDs->isSortingEnabled();
		tableWidgetCustomIDs->setSortingEnabled(false);
		qmc2Config->beginGroup(QString(QMC2_EMULATOR_PREFIX + "CustomIDs/%1").arg(emuName));
		QStringList idList = qmc2Config->value("IDs", QStringList()).toStringList();
		QStringList descriptionList = qmc2Config->value("Descriptions", QStringList()).toStringList();
		int i = 0;
		int row = tableWidgetCustomIDs->rowCount();
		while ( i < idList.count() ) {
			QString id = idList[i];
			if ( !id.isEmpty() ) {
				tableWidgetCustomIDs->insertRow(row);
				tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_ID, new QTableWidgetItem(id));
				tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_DESCRIPTION, new QTableWidgetItem(descriptionList[i]));
				row++;
			}
			i++;
		}
		qmc2Config->endGroup();
		tableWidgetCustomIDs->setSortingEnabled(oldSortingEnabled);
	}
}

void CustomIDSetup::on_toolButtonAddID_clicked()
{
	int row = tableWidgetCustomIDs->rowCount();
	bool oldSortingEnabled = tableWidgetCustomIDs->isSortingEnabled();
	tableWidgetCustomIDs->setSortingEnabled(false);
	tableWidgetCustomIDs->insertRow(row);
	tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_ID, new QTableWidgetItem());
	tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_DESCRIPTION, new QTableWidgetItem());
	QToolButton *tb = new QToolButton(0);
	QFontMetrics fm(qApp->font());
	tb->setIconSize(QSize(fm.height(), fm.height()));
	tb->setIcon(QIcon(QString::fromUtf8(":/data/img/pacman.png")));
	tb->setWhatsThis(":/data/img/pacman.png");
	tb->setToolTip(tr("Choose icon for this foreign ID (hold down for menu)"));
	QMenu *menu = new QMenu(tb);
	QAction *action = menu->addAction(QIcon(QString::fromUtf8(":/data/img/pacman.png")), tr("Default icon"));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(actionDefaultIdIconTriggered()));
	menu->addSeparator();
	action = menu->addAction(QIcon(QString::fromUtf8(":/data/img/no.png")), tr("No icon"));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(actionNoIdIconTriggered()));
	tb->setMenu(menu);
	tableWidgetCustomIDs->setCellWidget(row, QMC2_CUSTOMIDS_COLUMN_ICON, tb);
	connect(tb, SIGNAL(clicked()), this, SLOT(chooseIdIconClicked()));
	tableWidgetCustomIDs->setSortingEnabled(oldSortingEnabled);
	tableWidgetCustomIDs->resizeRowsToContents();
}

void CustomIDSetup::on_toolButtonRemoveID_clicked()
{
	QList<QTableWidgetItem *> selectedItems = tableWidgetCustomIDs->selectedItems();
	if ( selectedItems.count() > 0 )
		tableWidgetCustomIDs->removeRow(tableWidgetCustomIDs->row(selectedItems[0]));
	else if ( tableWidgetCustomIDs->currentRow() >= 0 )
		tableWidgetCustomIDs->removeRow(tableWidgetCustomIDs->currentRow());
}

void CustomIDSetup::on_tableWidgetCustomIDs_itemSelectionChanged()
{
	QList<QTableWidgetItem *> selectedItems = tableWidgetCustomIDs->selectedItems();
	if ( selectedItems.count() > 0 )
		toolButtonRemoveID->setEnabled(true);
	else
		toolButtonRemoveID->setEnabled(tableWidgetCustomIDs->currentRow() >= 0);
}

void CustomIDSetup::on_tableWidgetCustomIDs_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
	on_tableWidgetCustomIDs_itemSelectionChanged();
}

void CustomIDSetup::on_toolButtonSort_toggled(bool enable)
{
	tableWidgetCustomIDs->setSortingEnabled(enable);
	tableWidgetCustomIDs->setDragDropMode(enable ? QAbstractItemView::NoDragDrop : QAbstractItemView::InternalMove);
	if ( enable )
		tableWidgetCustomIDs->sortByColumn(tableWidgetCustomIDs->horizontalHeader()->sortIndicatorSection(), tableWidgetCustomIDs->horizontalHeader()->sortIndicatorOrder());
}

void CustomIDSetup::load()
{
	QString groupKey = QString(QMC2_EMULATOR_PREFIX + "CustomIDs/%1").arg(foreignEmulator);

	qmc2Config->beginGroup(groupKey);
	QStringList idList = qmc2Config->value("IDs", QStringList()).toStringList();
	QStringList descriptionList = qmc2Config->value("Descriptions", QStringList()).toStringList();
	while ( descriptionList.count() < idList.count() )
		descriptionList << QString();
	QStringList iconList = qmc2Config->value("Icons", QStringList()).toStringList();
	while ( iconList.count() < idList.count() )
		iconList << QString();
	qmc2Config->endGroup();

	tableWidgetCustomIDs->clearContents();
	bool oldSortingEnabled = tableWidgetCustomIDs->isSortingEnabled();
	tableWidgetCustomIDs->setSortingEnabled(false);
	int row = 0;
	for (int i = 0; i < idList.count(); i++) {
		QString id = idList[i];
		if ( !id.isEmpty() ) {
			QString description = descriptionList[i];
			QString idIcon = iconList[i];
			tableWidgetCustomIDs->insertRow(row);
			tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_ID, new QTableWidgetItem(id));
			tableWidgetCustomIDs->setItem(row, QMC2_CUSTOMIDS_COLUMN_DESCRIPTION, new QTableWidgetItem(description));
			QToolButton *tb = new QToolButton(0);
			QFontMetrics fm(qApp->font());
			tb->setIconSize(QSize(fm.height(), fm.height()));
			if ( idIcon.isEmpty() ) {
				tb->setIcon(QIcon(QString::fromUtf8(":/data/img/pacman.png")));
				tb->setWhatsThis(":/data/img/pacman.png");
			} else {
				if ( idIcon == "[none]" ) {
					tb->setIcon(QIcon());
					tb->setWhatsThis("[none]");
				} else {
					QIcon icon = QIcon(idIcon);
					if ( !icon.isNull() ) {
						tb->setIcon(icon);
						tb->setWhatsThis(idIcon);
					} else {
						tb->setIcon(QIcon(QString::fromUtf8(":/data/img/pacman.png")));
						tb->setWhatsThis(":/data/img/pacman.png");
					}
				}
			}
			tb->setToolTip(tr("Choose icon for this foreign ID (hold down for menu)"));
			QMenu *menu = new QMenu(tb);
			QAction *action = menu->addAction(QIcon(QString::fromUtf8(":/data/img/pacman.png")), tr("Default icon"));
			connect(action, SIGNAL(triggered(bool)), this, SLOT(actionDefaultIdIconTriggered()));
			menu->addSeparator();
			action = menu->addAction(QIcon(QString::fromUtf8(":/data/img/no.png")), tr("No icon"));
			connect(action, SIGNAL(triggered(bool)), this, SLOT(actionNoIdIconTriggered()));
			tb->setMenu(menu);
			tableWidgetCustomIDs->setCellWidget(row, QMC2_CUSTOMIDS_COLUMN_ICON, tb);
			connect(tb, SIGNAL(clicked()), this, SLOT(chooseIdIconClicked()));
			row++;
		}
	}
	tableWidgetCustomIDs->setSortingEnabled(oldSortingEnabled);
	if ( oldSortingEnabled )
		tableWidgetCustomIDs->sortByColumn(tableWidgetCustomIDs->horizontalHeader()->sortIndicatorSection(), tableWidgetCustomIDs->horizontalHeader()->sortIndicatorOrder());
	tableWidgetCustomIDs->resizeRowsToContents();
}

void CustomIDSetup::save()
{
	QString groupKey = QString(QMC2_EMULATOR_PREFIX + "CustomIDs/%1").arg(foreignEmulator);
	qmc2Config->remove(groupKey);

	QStringList idList, descriptionList, iconList;
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
			QToolButton *tb = (QToolButton *)tableWidgetCustomIDs->cellWidget(row, QMC2_CUSTOMIDS_COLUMN_ICON);
			if ( tb ) {
				QString idIcon = tb->whatsThis();
				if ( !idIcon.startsWith(":") )
					iconList << idIcon;
				else
					iconList << QString();
			} else
				iconList << QString();
		}
	}

	if ( !idList.isEmpty() ) {
		qmc2Config->beginGroup(groupKey);
		qmc2Config->setValue("IDs", idList);
		qmc2Config->setValue("Descriptions", descriptionList);
		QStringList iconSaveList = iconList;
		iconSaveList.removeAll(QString());
		if ( !iconSaveList.isEmpty() )
			qmc2Config->setValue("Icons", iconList);
		else
			qmc2Config->remove("Icons");
		qmc2Config->endGroup();
	}
}

void CustomIDSetup::chooseIdIconClicked()
{
	QToolButton *tb = (QToolButton *)sender();
	if ( tb ) {
		QString idIcon = tb->whatsThis();
		if ( idIcon.startsWith(":") )
			idIcon.clear();
		QStringList imageFileTypes;
		foreach (QByteArray imageFormat, QImageReader::supportedImageFormats())
			imageFileTypes << "*." + QString(imageFormat).toLower();
		QString fileName = QFileDialog::getOpenFileName(this, tr("Choose image file"), idIcon, tr("Supported image files (%1)").arg(imageFileTypes.join(" ")) + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
		if ( !fileName.isEmpty() ) {
			QIcon icon = QIcon(fileName);
			if ( !icon.isNull() ) {
				tb->setIcon(icon);
				tb->setWhatsThis(fileName);
			}
		}
	}
}

void CustomIDSetup::actionDefaultIdIconTriggered()
{
	QAction *action = (QAction *)sender();
	if ( action ) {
		QToolButton *tb = (QToolButton *)action->parentWidget()->parentWidget();
		if ( tb ) {
			tb->setIcon(QIcon(QString::fromUtf8(":/data/img/pacman.png")));
			tb->setWhatsThis(":/data/img/pacman.png");
		}
	}
}

void CustomIDSetup::actionNoIdIconTriggered()
{
	QAction *action = (QAction *)sender();
	if ( action ) {
		QToolButton *tb = (QToolButton *)action->parentWidget()->parentWidget();
		if ( tb ) {
			tb->setIcon(QIcon());
			tb->setWhatsThis("[none]");
		}
	}
}
