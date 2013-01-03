#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QApplication>
#include <QMap>

#include "arcademodesetup.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern QMap<QString, QString> qmc2CategoryMap;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern QMap<QString, QString> qmc2VersionMap;
#endif

ArcadeModeSetup::ArcadeModeSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	// FIXME: keyboard- & joystick-mappings are not supported yet
#if !defined(QMC2_WIP_ENABLED) 
	tabWidget->removeTab(tabKeys);
	tabWidget->removeTab(tabJoystick);
#endif

	adjustIconSizes();

	if ( !qmc2CategoryMap.isEmpty() )
		comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_CATEGORY, tr("Category"));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	if ( !qmc2VersionMap.isEmpty() )
		comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_VERSION, tr("Version"));
#endif

	QString defaultPath;
	int index;

	// internal settings
	tabWidget->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupCurrentTab", 0).toInt());
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Arcade/SetupWidth") && qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Arcade/SetupHeight") )
		resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupWidth").toInt(), qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupHeight").toInt());

	// general settings
#if defined(QMC2_OS_WIN)
	lineEditExecutableFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", QCoreApplication::applicationDirPath() + "\\" + "qmc2-arcade.exe").toString());
#elif defined(QMC2_OS_MAC)
	defaultPath = QFileInfo(QCoreApplication::applicationDirPath() + "../../qmc2-arcade/Contents/MacOS/qmc2-arcade").absoluteFilePath();
	lineEditExecutableFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", defaultPath).toString());
#else
	lineEditExecutableFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", QCoreApplication::applicationDirPath() + "/" + "qmc2-arcade").toString());
#endif
	defaultPath = QFileInfo(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString()).absolutePath();
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
}

void ArcadeModeSetup::on_lineEditFilteredListFile_textChanged(QString text)
{
	pushButtonExport->setEnabled(checkBoxUseFilteredList->isChecked() && isWritableFile(text));
}
