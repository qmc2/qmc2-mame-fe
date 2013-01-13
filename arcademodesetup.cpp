#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QApplication>
#include <QMap>
#include <QTreeWidgetItem>
#include <QTextStream>

#include "arcademodesetup.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern QMap<QString, QString> qmc2CategoryMap;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern QMap<QString, QString> qmc2VersionMap;
#endif
extern QStringList qmc2BiosROMs;
extern QStringList qmc2DeviceROMs;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
extern Gamelist *qmc2Gamelist;

int qmc2ArcadeModeSortCriteria = 0;
int qmc2ArcadeModeSortOrder = 0;

ArcadeModeSetup::ArcadeModeSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	// FIXME: keyboard- & joystick-mappings are not supported yet
#if !defined(QMC2_WIP_ENABLED) 
	tabWidget->removeTab(tabWidget->indexOf(tabKeys));
	tabWidget->removeTab(tabWidget->indexOf(tabJoystick));
#endif

	adjustIconSizes();

	if ( !qmc2CategoryMap.isEmpty() )
		comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_CATEGORY, tr("Category"));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	if ( !qmc2VersionMap.isEmpty() )
		comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_VERSION, tr("Version"));
#endif

	QString defaultPath, tmpString;
	int index;

	// internal settings
	tabWidget->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupCurrentTab", 0).toInt());
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Arcade/SetupWidth") && qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Arcade/SetupHeight") )
		resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupWidth").toInt(), qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupHeight").toInt());

	// general settings
#if defined(QMC2_OS_WIN)
	lineEditExecutableFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", QCoreApplication::applicationDirPath() + "\\" + "qmc2-arcade.exe").toString());
#elif defined(QMC2_OS_MAC)
	defaultPath = QFileInfo(QCoreApplication::applicationDirPath() + "../../../../qmc2-arcade.app/Contents/MacOS/qmc2-arcade").absoluteFilePath();
	lineEditExecutableFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", defaultPath).toString());
#else
	lineEditExecutableFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", QCoreApplication::applicationDirPath() + "/" + "qmc2-arcade").toString());
#endif
	tmpString = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString();
	if ( !tmpString.isEmpty() )
		defaultPath = QFileInfo(tmpString).absolutePath();
	else
		defaultPath.clear();
	lineEditWorkingDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/WorkingDirectory", defaultPath).toString());
	lineEditConfigurationPath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ConfigurationPath", QMC2_DYNAMIC_DOT_PATH).toString());
	index = comboBoxGraphicsSystem->findText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/GraphicsSystem", "raster").toString());
	if ( index > 0 )
		comboBoxGraphicsSystem->setCurrentIndex(index);
	index = comboBoxArcadeTheme->findText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/Theme", "ToxicWaste").toString());
	if ( index > 0 )
		comboBoxArcadeTheme->setCurrentIndex(index);
	index = comboBoxConsoleType->findText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ConsoleType", "terminal").toString());
	if ( index > 0 )
		comboBoxConsoleType->setCurrentIndex(index);

	// game list filter
	checkBoxUseFilteredList->setChecked(qmc2Config->value(QMC2_ARCADE_PREFIX + "UseFilteredList", false).toBool());
	lineEditFilteredListFile->setText(qmc2Config->value(QMC2_ARCADE_PREFIX + "FilteredListFile", qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GamelistCacheFile", QString()).toString() + ".filtered").toString());
	toolButtonSelectC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SelectC", true).toBool());
	toolButtonSelectM->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SelectM", true).toBool());
	toolButtonSelectI->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SelectI", false).toBool());
	toolButtonSelectN->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SelectN", false).toBool());
	toolButtonSelectU->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SelectU", false).toBool());
	comboBoxSortCriteria->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SortCriteria", QMC2_SORT_BY_DESCRIPTION).toInt());
	comboBoxSortOrder->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SortOrder", 0).toInt());
	comboBoxDriverStatus->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/DriverStatus", QMC2_ARCADE_DRV_STATUS_GOOD).toInt());
	lineEditNameFilter->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/NameFilter", QString()).toString());
	updateCategoryFilter();

	connect(pushButtonOk, SIGNAL(clicked()), this, SLOT(saveSettings()));
}

ArcadeModeSetup::~ArcadeModeSetup()
{
	// internal settings
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupCurrentTab", tabWidget->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupWidth", width());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupHeight", height());
}

void ArcadeModeSetup::saveSettings()
{
	// general settings
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", lineEditExecutableFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/WorkingDirectory", lineEditWorkingDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/ConfigurationPath", lineEditConfigurationPath->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/GraphicsSystem", comboBoxGraphicsSystem->currentText());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/Theme", comboBoxArcadeTheme->currentText());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/ConsoleType", comboBoxConsoleType->currentText());

	// game list filter
	qmc2Config->setValue(QMC2_ARCADE_PREFIX + "UseFilteredList", checkBoxUseFilteredList->isChecked());
	qmc2Config->setValue(QMC2_ARCADE_PREFIX + "FilteredListFile", lineEditFilteredListFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SelectC", toolButtonSelectC->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SelectM", toolButtonSelectM->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SelectI", toolButtonSelectI->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SelectN", toolButtonSelectN->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SelectU", toolButtonSelectU->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SortCriteria", comboBoxSortCriteria->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SortOrder", comboBoxSortOrder->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/DriverStatus", comboBoxDriverStatus->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/NameFilter", lineEditNameFilter->text());
	saveCategoryFilter();
}

bool ArcadeModeSetup::isWritableFile(QString fileName)
{
	if ( fileName.isEmpty() )
		return false;

	QFileInfo fi(fileName);

	if ( fi.isDir() )
		return false;

	if ( fi.exists() )
		return fi.isWritable();
	else
		return QFileInfo(fi.path()).isWritable();

	return false;
}

void ArcadeModeSetup::updateCategoryFilter()
{
	QStringList categoryNames = qmc2CategoryMap.values();
	categoryNames.removeDuplicates();
	qSort(categoryNames.begin(), categoryNames.end(), MainWindow::qStringListLessThan);
	QStringList excludedCategories = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExcludedCategories", QStringList()).toStringList();
	listWidgetCategoryFilter->setUpdatesEnabled(false);
	listWidgetCategoryFilter->clear();
	QListWidgetItem *item = new QListWidgetItem(tr("?"), listWidgetCategoryFilter);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
	item->setCheckState(excludedCategories.contains(tr("?")) ? Qt::Unchecked : Qt::Checked);
	foreach (QString category, categoryNames) {
		if ( !category.isEmpty() ) {
			item = new QListWidgetItem(category, listWidgetCategoryFilter);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
			item->setCheckState(excludedCategories.contains(category) ? Qt::Unchecked : Qt::Checked);
		}
	}
	listWidgetCategoryFilter->setUpdatesEnabled(true);
}

void ArcadeModeSetup::saveCategoryFilter()
{
	QStringList excludedCategories;

	if ( listWidgetCategoryFilter->count() == 1 ) {
		excludedCategories = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExcludedCategories", QStringList()).toStringList();
		if ( listWidgetCategoryFilter->item(0)->checkState() == Qt::Checked )
			excludedCategories.removeAll(tr("?"));
	} else {
		for (int i = 0; i < listWidgetCategoryFilter->count(); i++) {
			QListWidgetItem *item = listWidgetCategoryFilter->item(i);
			if ( item->checkState() == Qt::Unchecked )
				excludedCategories << item->text();
		}
	}

	if ( !excludedCategories.isEmpty() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/ExcludedCategories", excludedCategories);
	else
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + "Arcade/ExcludedCategories");
}

void ArcadeModeSetup::adjustIconSizes()
{
	QFontMetrics fm(qApp->font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);

	toolButtonBrowseExecutableFile->setIconSize(iconSize);
	toolButtonBrowseWorkingDirectory->setIconSize(iconSize);
	toolButtonBrowseConfigurationPath->setIconSize(iconSize);
	toolButtonBrowseFilteredListFile->setIconSize(iconSize);
	pushButtonExport->setIconSize(iconSize);
	toolButtonSelectC->setIconSize(iconSize);
	toolButtonSelectM->setIconSize(iconSize);
	toolButtonSelectI->setIconSize(iconSize);
	toolButtonSelectN->setIconSize(iconSize);
	toolButtonSelectU->setIconSize(iconSize);
	toolButtonSelectAll->setIconSize(iconSize);
	toolButtonDeselectAll->setIconSize(iconSize);
	comboBoxSortOrder->setIconSize(iconSize);
	toolButtonClearNameFilter->setIconSize(iconSize);

	adjustSize();
}

void ArcadeModeSetup::on_checkBoxUseFilteredList_toggled(bool enable)
{
	lineEditFilteredListFile->setEnabled(enable);
	toolButtonBrowseFilteredListFile->setEnabled(enable);
	pushButtonExport->setEnabled(enable && isWritableFile(lineEditFilteredListFile->text()));
	progressBarFilter->setEnabled(enable);
	toolButtonSelectC->setEnabled(enable);
	toolButtonSelectM->setEnabled(enable);
	toolButtonSelectI->setEnabled(enable);
	toolButtonSelectN->setEnabled(enable);
	toolButtonSelectU->setEnabled(enable);
	comboBoxSortCriteria->setEnabled(enable);
	comboBoxSortOrder->setEnabled(enable);
	comboBoxDriverStatus->setEnabled(enable);
	lineEditNameFilter->setEnabled(enable);
	toolButtonClearNameFilter->setEnabled(enable);
	listWidgetCategoryFilter->setEnabled(enable);
}

void ArcadeModeSetup::on_toolButtonBrowseExecutableFile_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose QMC2 Arcade's executable file"), lineEditExecutableFile->text(), tr("All files (*)"));
	if ( !fileName.isEmpty() )
		lineEditExecutableFile->setText(fileName);
}

void ArcadeModeSetup::on_toolButtonBrowseWorkingDirectory_clicked()
{
	QString workDir = QFileDialog::getExistingDirectory(this, tr("Choose QMC2 Arcade's working directory"), lineEditWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if ( !workDir.isEmpty() )
		lineEditWorkingDirectory->setText(workDir);
}

void ArcadeModeSetup::on_toolButtonBrowseConfigurationPath_clicked()
{
	QString configPath = QFileDialog::getExistingDirectory(this, tr("Choose QMC2 Arcade's configuration path"), lineEditConfigurationPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if ( !configPath.isEmpty() )
		lineEditConfigurationPath->setText(configPath);
}

void ArcadeModeSetup::on_toolButtonBrowseFilteredListFile_clicked()
{
	QString filteredListFile = QFileDialog::getSaveFileName(this, tr("Choose filtered list file for export"), lineEditFilteredListFile->text(), tr("All files (*)"));
	if ( !filteredListFile.isEmpty() )
		lineEditFilteredListFile->setText(filteredListFile);
}

void ArcadeModeSetup::on_pushButtonExport_clicked()
{
	QFile filteredListFile(lineEditFilteredListFile->text());

	if ( !filteredListFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: arcade mode: cannot open '%1' for writing").arg(QFileInfo(lineEditFilteredListFile->text()).absoluteFilePath()));
		return;
	}

	saveCategoryFilter();

	QTextStream ts(&filteredListFile);
	ts << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
#if defined(QMC2_EMUTYPE_MAME)
	ts << "MAME_VERSION\t" + qmc2Gamelist->emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";
#elif defined(QMC2_EMUTYPE_MESS)
	ts << "MESS_VERSION\t" + qmc2Gamelist->emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";
#elif defined(QMC2_EMUTYPE_UME)
	ts << "UME_VERSION\t" + qmc2Gamelist->emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";
#endif

	QStringList excludedCategories = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExcludedCategories", QStringList()).toStringList();
	int minDrvStatus = comboBoxDriverStatus->currentIndex();
	int itemCount = 0;
	QString nameFilter = lineEditNameFilter->text();
	QRegExp nameFilterRegExp(nameFilter);
	QList<GamelistItem *> selectedGames;

	if ( !nameFilter.isEmpty() && !nameFilterRegExp.isValid() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: arcade mode: the name filter regular expression is invalid"));

	progressBarFilter->setFormat(tr("Filtering"));
	progressBarFilter->setRange(0, qmc2GamelistItemMap.count());

	foreach (QString game, qmc2GamelistItemMap.keys()) {
		progressBarFilter->setValue(++itemCount);
		if ( !nameFilter.isEmpty() )
			if ( game.indexOf(nameFilterRegExp) < 0 )
				continue;
		QString category = qmc2CategoryMap[game];
		if ( category.isEmpty() )
			category = tr("?");
		if ( qmc2DeviceROMs.contains(game) || (!qmc2CategoryMap.isEmpty() && excludedCategories.contains(category)) )
			continue;
		GamelistItem *gameItem = (GamelistItem *)qmc2GamelistItemMap[game];
		if ( !gameItem )
			continue;
		if ( minDrvStatus < QMC2_ARCADE_DRV_STATUS_IMPERFECT ) {
			QString drvStatus = gameItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT);
			if ( minDrvStatus == QMC2_ARCADE_DRV_STATUS_PRELIMINARY ) {
				if ( drvStatus != tr("good") && drvStatus != tr("preliminary") )
					continue;
			} else {
				if ( drvStatus != tr("good") )
					continue;
			}
		}
		switch ( gameItem->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toLatin1() ) {
			case QMC2_ROMSTATE_CHAR_C:
				if ( toolButtonSelectC->isChecked() )
					selectedGames << gameItem;
				break;
		    case QMC2_ROMSTATE_CHAR_M:
				if ( toolButtonSelectM->isChecked() )
					selectedGames << gameItem;
				break;
		    case QMC2_ROMSTATE_CHAR_I:
				if ( toolButtonSelectI->isChecked() )
					selectedGames << gameItem;
				break;
		    case QMC2_ROMSTATE_CHAR_N:
				if ( toolButtonSelectN->isChecked() )
					selectedGames << gameItem;
				break;
		    case QMC2_ROMSTATE_CHAR_U:
		    default:
				if ( toolButtonSelectU->isChecked() )
					selectedGames << gameItem;
				break;
		}
	}

	progressBarFilter->setRange(0, selectedGames.count());
	progressBarFilter->setFormat(tr("Sorting"));
	progressBarFilter->setValue(0);
	qApp->processEvents();
	qmc2ArcadeModeSortCriteria = comboBoxSortCriteria->currentIndex();
	qmc2ArcadeModeSortOrder = comboBoxSortOrder->currentIndex();
	qSort(selectedGames.begin(), selectedGames.end(), ArcadeModeSetup::lessThan);

	progressBarFilter->setValue(0);
	progressBarFilter->setFormat(tr("Exporting"));
	for (int i = 0; i < selectedGames.count(); i++) {
		progressBarFilter->setValue(i + 1);
		GamelistItem *gameItem = selectedGames[i];
		QString gameName = gameItem->text(QMC2_GAMELIST_COLUMN_NAME);
		QString cloneOf;
		GamelistItem *parentItem = (GamelistItem *)qmc2HierarchyItemMap[gameName];
		if ( parentItem )
			cloneOf = parentItem->text(QMC2_GAMELIST_COLUMN_GAME);
		ts << gameName << "\t"
		   << gameItem->text(QMC2_GAMELIST_COLUMN_GAME) << "\t"
		   << gameItem->text(QMC2_GAMELIST_COLUMN_MANU) << "\t"
		   << gameItem->text(QMC2_GAMELIST_COLUMN_YEAR) << "\t"
		   << cloneOf << "\t"
	   	   << (qmc2BiosROMs.contains(gameName) ? "1": "0") << "\t"
		   << (gameItem->text(QMC2_GAMELIST_COLUMN_RTYPES).contains(tr("ROM")) ? "1" : "0") << "\t"
		   << (gameItem->text(QMC2_GAMELIST_COLUMN_RTYPES).contains(tr("CHD")) ? "1": "0") << "\t"
		   << gameItem->text(QMC2_GAMELIST_COLUMN_PLAYERS) << "\t"
		   << gameItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT) << "\t"
		   << "0\n";
	}

	progressBarFilter->setValue(0);
	progressBarFilter->setFormat(tr("Idle"));
	filteredListFile.close();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("arcade mode: exported %n filtered set(s) to '%1'", "", selectedGames.count()).arg(QFileInfo(lineEditFilteredListFile->text()).absoluteFilePath()));
}

void ArcadeModeSetup::on_lineEditFilteredListFile_textChanged(QString text)
{
	pushButtonExport->setEnabled(checkBoxUseFilteredList->isChecked() && isWritableFile(text));
}

void ArcadeModeSetup::on_toolButtonSelectAll_clicked()
{
	for (int i = 0; i < listWidgetCategoryFilter->count(); i++) {
		QListWidgetItem *item = listWidgetCategoryFilter->item(i);
		item->setCheckState(Qt::Checked);
	}
}

void ArcadeModeSetup::on_toolButtonDeselectAll_clicked()
{
	for (int i = 0; i < listWidgetCategoryFilter->count(); i++) {
		QListWidgetItem *item = listWidgetCategoryFilter->item(i);
		item->setCheckState(Qt::Unchecked);
	}
}

bool ArcadeModeSetup::lessThan(const GamelistItem *item1, const GamelistItem *item2)
{
	switch ( qmc2ArcadeModeSortCriteria ) {
		case QMC2_SORT_BY_DESCRIPTION:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_GAME).toUpper() > item2->text(QMC2_GAMELIST_COLUMN_GAME).toUpper());
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_GAME).toUpper() < item2->text(QMC2_GAMELIST_COLUMN_GAME).toUpper());
		case QMC2_SORT_BY_ROM_STATE:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toLatin1() > item2->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toLatin1());
			else
				return (item1->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toLatin1() < item2->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toLatin1());
		case QMC2_SORT_BY_TAG:
			if ( qmc2ArcadeModeSortOrder )
				return (int(item1->checkState(QMC2_GAMELIST_COLUMN_TAG)) > int(item2->checkState(QMC2_GAMELIST_COLUMN_TAG)));
			else
				return (int(item1->checkState(QMC2_GAMELIST_COLUMN_TAG)) < int(item2->checkState(QMC2_GAMELIST_COLUMN_TAG)));
		case QMC2_SORT_BY_YEAR:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_YEAR) > item2->text(QMC2_GAMELIST_COLUMN_YEAR));
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_YEAR) < item2->text(QMC2_GAMELIST_COLUMN_YEAR));
		case QMC2_SORT_BY_MANUFACTURER:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_MANU).toUpper() > item2->text(QMC2_GAMELIST_COLUMN_MANU).toUpper());
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_MANU).toUpper() < item2->text(QMC2_GAMELIST_COLUMN_MANU).toUpper());
		case QMC2_SORT_BY_NAME:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_NAME).toUpper() > item2->text(QMC2_GAMELIST_COLUMN_NAME).toUpper());
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_NAME).toUpper() < item2->text(QMC2_GAMELIST_COLUMN_NAME).toUpper());
		case QMC2_SORT_BY_ROMTYPES:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper() > item2->text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper());
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper() < item2->text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper());
		case QMC2_SORT_BY_PLAYERS:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_PLAYERS).toUpper() > item2->text(QMC2_GAMELIST_COLUMN_PLAYERS).toUpper());
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_PLAYERS).toUpper() < item2->text(QMC2_GAMELIST_COLUMN_PLAYERS).toUpper());
		case QMC2_SORT_BY_DRVSTAT:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_DRVSTAT).toUpper() > item2->text(QMC2_GAMELIST_COLUMN_DRVSTAT).toUpper());
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_DRVSTAT).toUpper() < item2->text(QMC2_GAMELIST_COLUMN_DRVSTAT).toUpper());
		case QMC2_SORT_BY_CATEGORY:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper() > item2->text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper());
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper() < item2->text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper());
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		case QMC2_SORT_BY_VERSION:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_GAMELIST_COLUMN_VERSION).toUpper() > item2->text(QMC2_GAMELIST_COLUMN_VERSION).toUpper());
			else
				return (item1->text(QMC2_GAMELIST_COLUMN_VERSION).toUpper() < item2->text(QMC2_GAMELIST_COLUMN_VERSION).toUpper());
#endif
		default:
			return qmc2ArcadeModeSortOrder == 1;
	}
}
