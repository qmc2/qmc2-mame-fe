#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QMultiMap>
#include <QHash>
#include <QMap>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QTextStream>

#include <algorithm> // std::sort()

#include "settings.h"
#include "arcademodesetup.h"
#include "qmc2main.h"
#include "options.h"
#include "macros.h"
#include "arcade/keysequences.h"
#include "keyseqscan.h"
#include "machinelist.h"
#if QMC2_JOYSTICK == 1
#include "joyfuncscan.h"
#endif

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
extern QHash<QString, QString> qmc2ParentHash;
extern MachineList *qmc2MachineList;
extern Options *qmc2Options;
extern MainEventFilter *qmc2MainEventFilter;
extern bool qmc2SuppressQtMessages;

int qmc2ArcadeModeSortCriteria = 0;
int qmc2ArcadeModeSortOrder = 0;

QStringList ArcadeModeSetup::keySequenceMapBases;
#if QMC2_JOYSTICK == 1
QStringList ArcadeModeSetup::joyFunctionMapBases;
#endif

ArcadeModeSetup::ArcadeModeSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

#if QMC2_JOYSTICK == 0
	tabWidget->removeTab(tabWidget->indexOf(tabJoystick));
#else
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Joystick/EnableJoystickControl", false).toBool() ) {
		tabJoystick->setEnabled(false);
		joyStatusLabel = new QLabel("<b>" + tr("Please enable joystick control!") + "<b>", this);
		tabWidgetJoyMaps->setCornerWidget(joyStatusLabel);
	}
#endif

	adjustIconSizes();

	// QSettings base-keys for key-sequence and joystick-function maps (take care that the indexes in the string-lists correspond to the QMC2_ARCADE_THEME_* macros!)
	if ( keySequenceMapBases.isEmpty() )
		keySequenceMapBases << "Arcade/ToxicWaste/keySequenceMap" << "Arcade/darkone/keySequenceMap";
#if QMC2_JOYSTICK == 1
	if ( joyFunctionMapBases.isEmpty() )
		joyFunctionMapBases << "Arcade/ToxicWaste/joyFunctionMap" << "Arcade/darkone/joyFunctionMap";
#endif

	// category and version maps
	if ( !qmc2MachineList->categoryMap.isEmpty() )
		comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_CATEGORY, tr("Category"));
	if ( !qmc2MachineList->versionMap.isEmpty() )
		comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_VERSION, tr("Version"));

	QString defaultPath, tmpString;
	int index;

	// internal settings
	tabWidget->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupCurrentTab", 0).toInt());
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Arcade/SetupWidth") && qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Arcade/SetupHeight") )
		resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupWidth").toInt(), qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupHeight").toInt());
	tabWidgetKeyMaps->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupKeyMapCurrentTab", 0).toInt());
#if QMC2_JOYSTICK == 1
	tabWidgetJoyMaps->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupJoyMapCurrentTab", 0).toInt());
#endif
	treeWidgetKeyMapToxicWaste->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupKeyMapToxicWasteHeaderState", QByteArray()).toByteArray());
	treeWidgetKeyMapDarkone->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupKeyMapDarkoneHeaderState", QByteArray()).toByteArray());
#if QMC2_JOYSTICK == 1
	treeWidgetJoyMapToxicWaste->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupJoyMapToxicWasteHeaderState", QByteArray()).toByteArray());
	treeWidgetJoyMapDarkone->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/SetupJoyMapDarkoneHeaderState", QByteArray()).toByteArray());
#endif

	// general settings
#if defined(QMC2_OS_WIN)
	lineEditExecutableFile->setText(QMC2_QSETTINGS_CAST(qmc2Config)->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", QCoreApplication::applicationDirPath() + "\\" + "qmc2-arcade.exe").toString());
#elif defined(QMC2_OS_MAC)
	defaultPath = QFileInfo(QCoreApplication::applicationDirPath() + "../../../../qmc2-arcade.app/Contents/MacOS/qmc2-arcade").absoluteFilePath();
	lineEditExecutableFile->setText(QMC2_QSETTINGS_CAST(qmc2Config)->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", defaultPath).toString());
#else
	lineEditExecutableFile->setText(QMC2_QSETTINGS_CAST(qmc2Config)->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", QCoreApplication::applicationDirPath() + "/" + "qmc2-arcade").toString());
#endif
	tmpString = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString();
	if ( !tmpString.isEmpty() )
		defaultPath = QFileInfo(tmpString).absolutePath();
	else
		defaultPath.clear();
	lineEditWorkingDirectory->setText(QMC2_QSETTINGS_CAST(qmc2Config)->value(QMC2_FRONTEND_PREFIX + "Arcade/WorkingDirectory", defaultPath).toString());
	lineEditConfigurationPath->setText(QMC2_QSETTINGS_CAST(qmc2Config)->value(QMC2_FRONTEND_PREFIX + "Arcade/ConfigurationPath", QMC2_DYNAMIC_DOT_PATH).toString());
	index = comboBoxGraphicsSystem->findText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/GraphicsSystem", "raster").toString());
	if ( index > 0 )
		comboBoxGraphicsSystem->setCurrentIndex(index);
	index = comboBoxArcadeTheme->findText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/Theme", "ToxicWaste").toString());
	if ( index > 0 )
		comboBoxArcadeTheme->setCurrentIndex(index);
	index = comboBoxConsoleType->findText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ConsoleType", "terminal").toString());
	if ( index > 0 )
		comboBoxConsoleType->setCurrentIndex(index);
	checkBoxDebugKeys->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/DebugKeys", false).toBool());
#if QMC2_JOYSTICK == 1
	checkBoxNoJoy->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/NoJoy", false).toBool());
	checkBoxDebugJoy->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/DebugJoy", false).toBool());
	checkBoxJoy->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/Joy", false).toBool());
	spinBoxJoyIndex->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/JoyIndex", 0).toInt());
#endif

	// machine list filter
	checkBoxFavoriteSetsOnly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/FavoriteSetsOnly", false).toBool());
	checkBoxTaggedSetsOnly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/TaggedSetsOnly", false).toBool());
	checkBoxParentSetsOnly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ParentSetsOnly", false).toBool());
	checkBoxUseFilteredList->setChecked(qmc2Config->value(QMC2_ARCADE_PREFIX + "UseFilteredList", false).toBool());
	lineEditFilteredListFile->setText(QMC2_QSETTINGS_CAST(qmc2Config)->value(QMC2_ARCADE_PREFIX + "FilteredListFile", qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MachineListCacheFile", QString()).toString() + ".filtered").toString());
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

	// miscellaneous connections
	connect(pushButtonOk, SIGNAL(clicked()), this, SLOT(saveSettings()));

	// load key-sequence and joy-function maps asynchronously
	connect(treeWidgetKeyMapToxicWaste, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(scanCustomKeySequence(QTreeWidgetItem *, int)));
	connect(treeWidgetKeyMapDarkone, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(scanCustomKeySequence(QTreeWidgetItem *, int)));
	QTimer::singleShot(0, this, SLOT(loadKeySequenceMaps()));
#if QMC2_JOYSTICK == 1
	connect(treeWidgetJoyMapToxicWaste, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(scanCustomJoyFunction(QTreeWidgetItem *, int)));
	connect(treeWidgetJoyMapDarkone, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(scanCustomJoyFunction(QTreeWidgetItem *, int)));
	QTimer::singleShot(0, this, SLOT(loadJoyFunctionMaps()));
#endif
}

ArcadeModeSetup::~ArcadeModeSetup()
{
	// internal settings
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupCurrentTab", tabWidget->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupWidth", width());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupHeight", height());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupKeyMapCurrentTab", tabWidgetKeyMaps->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupJoyMapCurrentTab", tabWidgetJoyMaps->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupKeyMapToxicWasteHeaderState", treeWidgetKeyMapToxicWaste->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupKeyMapDarkoneHeaderState", treeWidgetKeyMapDarkone->header()->saveState());
#if QMC2_JOYSTICK == 1
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupJoyMapToxicWasteHeaderState", treeWidgetJoyMapToxicWaste->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/SetupJoyMapDarkoneHeaderState", treeWidgetJoyMapDarkone->header()->saveState());
#endif
}

void ArcadeModeSetup::scanCustomKeySequence(QTreeWidgetItem *item, int /*column*/)
{
	if ( !item->parent() )
		return;

	qApp->removeEventFilter(qmc2MainEventFilter);

	KeySequenceScanner keySeqScanner(this, false, false, true);
	keySeqScanner.setWindowTitle(tr("Scanning key sequence"));
	keySeqScanner.labelStatus->setText(tr("Scanning key sequence"));

	if ( keySeqScanner.exec() == QDialog::Accepted ) {
		QString keySeqText = keySeqScanner.labelKeySequence->text();
		QTreeWidgetItemIterator it(item->treeWidget());
		while ( *it ) {
			if ( keySeqText == (*it)->text(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM) )
				(*it)->setText(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM, QString());
			it++;
		}
		item->setText(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM, keySeqText);
	} else if ( keySeqScanner.clearClicked )
		item->setText(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM, QString());

	qApp->installEventFilter(qmc2MainEventFilter);

	QTimer::singleShot(0, this, SLOT(checkKeySequenceMaps()));
}

void ArcadeModeSetup::loadKeySequenceMaps()
{
	for (int i = 0; i < QMC2_ARCADE_THEME_COUNT; i++) {
		QStringList keySequences;
		QStringList keySequenceDescriptions;
		QMC2_ARCADE_ADD_COMMON_KEYSEQUENCES(keySequences);
		QMC2_ARCADE_ADD_COMMON_DESCRIPTIONS(keySequenceDescriptions);
		QTreeWidget *treeWidget = NULL;
		switch ( i ) {
			case QMC2_ARCADE_THEME_TOXICWASTE:
				treeWidget = treeWidgetKeyMapToxicWaste;
				QMC2_ARCADE_ADD_TOXIXCWASTE_KEYSEQUENCES(keySequences);
				QMC2_ARCADE_ADD_TOXIXCWASTE_DESCRIPTIONS(keySequenceDescriptions);
				break;
			case QMC2_ARCADE_THEME_DARKONE:
				treeWidget = treeWidgetKeyMapDarkone;
				QMC2_ARCADE_ADD_DARKONE_KEYSEQUENCES(keySequences);
				QMC2_ARCADE_ADD_DARKONE_DESCRIPTIONS(keySequenceDescriptions);
				break;
		}
		QMap<QString, QTreeWidgetItem *> descriptionItemMap;
		for (int j = 0; j < keySequenceDescriptions.count(); j++) {
			QString description = keySequenceDescriptions[j];
			QTreeWidgetItem *descriptionItem;
			if ( descriptionItemMap.contains(description) )
				descriptionItem = descriptionItemMap[description];
			else {
				descriptionItem = new QTreeWidgetItem(treeWidget);
				descriptionItem->setText(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION, description);
				descriptionItemMap[description] = descriptionItem;
			}
			QTreeWidgetItem *keySequenceItem = new QTreeWidgetItem(descriptionItem);
			keySequenceItem->setText(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION, keySequences[j]);
			QString keySeqKey = keySequenceMapBases[i] + "/" + keySequences[j];
			if ( qmc2Config->contains(keySeqKey) ) {
				keySequenceItem->setText(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM, qmc2Config->value(keySeqKey, QString()).toString());
				descriptionItem->setExpanded(true);
			}
		}
	}

	QTimer::singleShot(0, this, SLOT(checkKeySequenceMaps()));
}

void ArcadeModeSetup::saveKeySequenceMaps()
{
	for (int i = 0; i < QMC2_ARCADE_THEME_COUNT; i++) {
		QTreeWidget *treeWidget = NULL;
		switch ( i ) {
			case QMC2_ARCADE_THEME_TOXICWASTE:
				treeWidget = treeWidgetKeyMapToxicWaste;
				break;
			case QMC2_ARCADE_THEME_DARKONE:
				treeWidget = treeWidgetKeyMapDarkone;
				break;
		}

		qmc2Config->remove(keySequenceMapBases[i]);

		QTreeWidgetItemIterator it(treeWidget);
		while ( *it ) {
			QString customKeySequence = (*it)->text(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM);
			if ( !customKeySequence.isEmpty() )
				qmc2Config->setValue(keySequenceMapBases[i] + "/" + (*it)->text(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION), customKeySequence);
			it++;
		}
	}
}

void ArcadeModeSetup::checkKeySequenceMaps()
{
	for (int i = 0; i < QMC2_ARCADE_THEME_COUNT; i++) {
		QTreeWidget *treeWidget = NULL;
		switch ( i ) {
			case QMC2_ARCADE_THEME_TOXICWASTE:
				treeWidget = treeWidgetKeyMapToxicWaste;
				break;
			case QMC2_ARCADE_THEME_DARKONE:
				treeWidget = treeWidgetKeyMapDarkone;
				break;
		}

		QTreeWidgetItemIterator it(treeWidget);
		QMultiMap<QString, QTreeWidgetItem *> keySeqMap;
		while ( *it ) {
			if ( (*it)->parent() ) {
				QString customKeySequence = (*it)->text(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM);
				keySeqMap.insert((*it)->text(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION), *it);
				if ( !customKeySequence.isEmpty() )
					keySeqMap.insert(customKeySequence, *it);
			}
			it++;
		}
		foreach (QString key, keySeqMap.keys()) {
			QList<QTreeWidgetItem *> itemList = keySeqMap.values(key);
			if ( itemList.count() > 1 ) {
				foreach (QTreeWidgetItem *item, itemList) {
					if ( item->text(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION) == key && item->text(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM).isEmpty() ) {
						item->setForeground(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION, Qt::red);
						item->parent()->setExpanded(true);
					} else
						item->setForeground(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION, Qt::green);
					if ( item->text(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM) == key ) {
						bool isOk = true;
						foreach (QTreeWidgetItem *it, itemList) {
							if ( it->text(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION) == key && it->text(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM).isEmpty() ) {
								isOk = false;
								break;
							}
						}

						if ( isOk )
							item->setForeground(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM, Qt::green);
						else {
							item->setForeground(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM, Qt::red);
							item->parent()->setExpanded(true);
						}
					}
				}
			} else {
				QTreeWidgetItem *item = itemList[0];
				if ( item->text(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION) == key )
					item->setForeground(QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION, Qt::green);
				if ( item->text(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM) == key )
					item->setForeground(QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM, Qt::green);
			}
		}
	}
}

#if QMC2_JOYSTICK == 1
void ArcadeModeSetup::scanCustomJoyFunction(QTreeWidgetItem *item, int /*column*/)
{
	if ( !item->parent() || qmc2Options->joystick == NULL )
		return;

	bool saveSQM = qmc2SuppressQtMessages;
	qmc2SuppressQtMessages = true;

	JoystickFunctionScanner joyFunctionScanner(qmc2Options->joystick, true, this);

	if ( joyFunctionScanner.exec() == QDialog::Accepted ) {
		QString joyFuncText = joyFunctionScanner.labelJoystickFunction->text();
		QTreeWidgetItemIterator it(item->treeWidget());
		while ( *it ) {
			if ( joyFuncText == (*it)->text(QMC2_ARCADE_JOYMAP_COLUMN_CUSTOM) )
				(*it)->setText(QMC2_ARCADE_JOYMAP_COLUMN_CUSTOM, QString());
			it++;
		}
		item->setText(QMC2_ARCADE_JOYMAP_COLUMN_CUSTOM, joyFuncText);
	} else if ( joyFunctionScanner.clearClicked )
		item->setText(QMC2_ARCADE_JOYMAP_COLUMN_CUSTOM, QString());

	qmc2SuppressQtMessages = saveSQM;
}

void ArcadeModeSetup::loadJoyFunctionMaps()
{
	for (int i = 0; i < QMC2_ARCADE_THEME_COUNT; i++) {
		QStringList keySequences;
		QStringList keySequenceDescriptions;
		QMC2_ARCADE_ADD_COMMON_KEYSEQUENCES(keySequences);
		QMC2_ARCADE_ADD_COMMON_DESCRIPTIONS(keySequenceDescriptions);
		QTreeWidget *treeWidget = NULL;
		switch ( i ) {
			case QMC2_ARCADE_THEME_TOXICWASTE:
				treeWidget = treeWidgetJoyMapToxicWaste;
				QMC2_ARCADE_ADD_TOXIXCWASTE_KEYSEQUENCES(keySequences);
				QMC2_ARCADE_ADD_TOXIXCWASTE_DESCRIPTIONS(keySequenceDescriptions);
				break;
			case QMC2_ARCADE_THEME_DARKONE:
				treeWidget = treeWidgetJoyMapDarkone;
				QMC2_ARCADE_ADD_DARKONE_KEYSEQUENCES(keySequences);
				QMC2_ARCADE_ADD_DARKONE_DESCRIPTIONS(keySequenceDescriptions);
				break;
		}
		QMap<QString, QTreeWidgetItem *> descriptionItemMap;
		for (int j = 0; j < keySequenceDescriptions.count(); j++) {
			QString description = keySequenceDescriptions[j];
			QTreeWidgetItem *descriptionItem;
			if ( descriptionItemMap.contains(description) )
				descriptionItem = descriptionItemMap[description];
			else {
				descriptionItem = new QTreeWidgetItem(treeWidget);
				descriptionItem->setText(QMC2_ARCADE_JOYMAP_COLUMN_FUNCTION, description);
				descriptionItemMap[description] = descriptionItem;
			}
			QTreeWidgetItem *keySequenceItem = new QTreeWidgetItem(descriptionItem);
			keySequenceItem->setText(QMC2_ARCADE_JOYMAP_COLUMN_FUNCTION, keySequences[j]);
			QString joyFuncKey = joyFunctionMapBases[i] + "/" + keySequences[j];
			if ( qmc2Config->contains(joyFuncKey) ) {
				keySequenceItem->setText(QMC2_ARCADE_JOYMAP_COLUMN_CUSTOM, qmc2Config->value(joyFuncKey, QString()).toString());
				descriptionItem->setExpanded(true);
			}
		}
	}
}

void ArcadeModeSetup::saveJoyFunctionMaps()
{
	for (int i = 0; i < QMC2_ARCADE_THEME_COUNT; i++) {
		QTreeWidget *treeWidget = NULL;
		switch ( i ) {
			case QMC2_ARCADE_THEME_TOXICWASTE:
				treeWidget = treeWidgetJoyMapToxicWaste;
				break;
			case QMC2_ARCADE_THEME_DARKONE:
				treeWidget = treeWidgetJoyMapDarkone;
				break;
		}

		qmc2Config->remove(joyFunctionMapBases[i]);

		QTreeWidgetItemIterator it(treeWidget);
		while ( *it ) {
			QString joyFunction = (*it)->text(QMC2_ARCADE_JOYMAP_COLUMN_CUSTOM);
			if ( !joyFunction.isEmpty() )
				qmc2Config->setValue(joyFunctionMapBases[i] + "/" + (*it)->text(QMC2_ARCADE_JOYMAP_COLUMN_FUNCTION), joyFunction);
			it++;
		}
	}
}
#endif

void ArcadeModeSetup::saveSettings()
{
	// general settings
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", lineEditExecutableFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/WorkingDirectory", lineEditWorkingDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/ConfigurationPath", lineEditConfigurationPath->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/GraphicsSystem", comboBoxGraphicsSystem->currentText());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/Theme", comboBoxArcadeTheme->currentText());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/ConsoleType", comboBoxConsoleType->currentText());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/DebugKeys", checkBoxDebugKeys->isChecked());
#if QMC2_JOYSTICK == 1
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/NoJoy", checkBoxNoJoy->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/DebugJoy", checkBoxDebugJoy->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/Joy", checkBoxJoy->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/JoyIndex", spinBoxJoyIndex->value());
#endif

	// machine list filter
	qmc2Config->setValue(QMC2_ARCADE_PREFIX + "UseFilteredList", checkBoxUseFilteredList->isChecked());
	qmc2Config->setValue(QMC2_ARCADE_PREFIX + "FilteredListFile", lineEditFilteredListFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/FavoriteSetsOnly", checkBoxFavoriteSetsOnly->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/TaggedSetsOnly", checkBoxTaggedSetsOnly->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Arcade/ParentSetsOnly", checkBoxParentSetsOnly->isChecked());
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

	// key-sequence and joystick-function maps
	saveKeySequenceMaps();
#if QMC2_JOYSTICK == 1
	saveJoyFunctionMaps();
#endif
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
	QStringList categoryNames;
	foreach (QString *category, qmc2MachineList->categoryMap.values())
		if ( category )
			categoryNames << *category;
	categoryNames.removeAll(tr("System / Device"));
	categoryNames.removeDuplicates();
	std::sort(categoryNames.begin(), categoryNames.end(), MainWindow::qStringListLessThan);
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

	checkBoxFavoriteSetsOnly->setEnabled(enable);
	checkBoxTaggedSetsOnly->setEnabled(enable);

	enable &= !checkBoxFavoriteSetsOnly->isChecked() && !checkBoxTaggedSetsOnly->isChecked();

	checkBoxParentSetsOnly->setEnabled(enable);
	toolButtonSelectC->setEnabled(enable);
	toolButtonSelectM->setEnabled(enable);
	toolButtonSelectI->setEnabled(enable);
	toolButtonSelectN->setEnabled(enable);
	toolButtonSelectU->setEnabled(enable);
	toolButtonSelectAll->setEnabled(enable);
	toolButtonDeselectAll->setEnabled(enable);
	comboBoxSortCriteria->setEnabled(enable);
	comboBoxSortOrder->setEnabled(enable);
	comboBoxDriverStatus->setEnabled(enable);
	lineEditNameFilter->setEnabled(enable);
	toolButtonClearNameFilter->setEnabled(enable);
	listWidgetCategoryFilter->setEnabled(enable);
}

void ArcadeModeSetup::on_checkBoxFavoriteSetsOnly_toggled(bool enable)
{
	if ( checkBoxFavoriteSetsOnly->isEnabled() && enable )
		checkBoxTaggedSetsOnly->setChecked(false);
	on_checkBoxUseFilteredList_toggled(true);
}

void ArcadeModeSetup::on_checkBoxTaggedSetsOnly_toggled(bool enable)
{
	if ( checkBoxTaggedSetsOnly->isEnabled() && enable )
		checkBoxFavoriteSetsOnly->setChecked(false);
	on_checkBoxUseFilteredList_toggled(true);
}

void ArcadeModeSetup::on_toolButtonBrowseExecutableFile_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose QMC2 Arcade's executable file"), lineEditExecutableFile->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !fileName.isEmpty() )
		lineEditExecutableFile->setText(fileName);
}

void ArcadeModeSetup::on_toolButtonBrowseWorkingDirectory_clicked()
{
	QString workDir = QFileDialog::getExistingDirectory(this, tr("Choose QMC2 Arcade's working directory"), lineEditWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !workDir.isEmpty() )
		lineEditWorkingDirectory->setText(workDir);
}

void ArcadeModeSetup::on_toolButtonBrowseConfigurationPath_clicked()
{
	QString configPath = QFileDialog::getExistingDirectory(this, tr("Choose QMC2 Arcade's configuration path"), lineEditConfigurationPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !configPath.isEmpty() )
		lineEditConfigurationPath->setText(configPath);
}

void ArcadeModeSetup::on_toolButtonBrowseFilteredListFile_clicked()
{
	QString filteredListFile = QFileDialog::getSaveFileName(this, tr("Choose filtered list file for export"), lineEditFilteredListFile->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
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
	ts << "MAME_VERSION\t" + qmc2MachineList->emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";

	QStringList excludedCategories = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExcludedCategories", QStringList()).toStringList();
	int minDrvStatus = comboBoxDriverStatus->currentIndex();
	int itemCount = 0;
	QString nameFilter = lineEditNameFilter->text();
	QRegExp nameFilterRegExp(nameFilter);
	QList<MachineListItem *> selectedGames;

	if ( !nameFilter.isEmpty() && !nameFilterRegExp.isValid() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: arcade mode: the name filter regular expression is invalid"));

	progressBarFilter->setFormat(tr("Filtering"));
	progressBarFilter->setRange(0, qmc2MachineListItemHash.count());

	foreach (QString game, qmc2MachineListItemHash.keys()) {
		progressBarFilter->setValue(++itemCount);

		MachineListItem *gameItem = (MachineListItem *)qmc2MachineListItemHash[game];
		if ( !gameItem )
			continue;

		// no devices
		if ( qmc2MachineList->isDevice(game) )
			continue;

		// tagged sets only?
		if ( checkBoxTaggedSetsOnly->isChecked() ) {
			if ( gameItem->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked )
				selectedGames << gameItem;
			continue;
		}

		// favorite sets only?
		if ( checkBoxFavoriteSetsOnly->isChecked() ) {
			MachineListItem *gameItem = (MachineListItem *)qmc2MachineListItemHash[game];
			QList<QListWidgetItem *> favoritesMatches = qmc2MainWindow->listWidgetFavorites->findItems(gameItem->text(QMC2_MACHINELIST_COLUMN_MACHINE), Qt::MatchExactly);
			if ( !favoritesMatches.isEmpty() )
				selectedGames << gameItem;
			continue;
		}

		// parent sets only?
		if ( checkBoxParentSetsOnly->isChecked() )
			if ( !qmc2ParentHash[game].isEmpty() )
				continue;

		// name filter
		if ( !nameFilter.isEmpty() )
			if ( game.indexOf(nameFilterRegExp) < 0 )
				continue;

		// category
		if ( !qmc2MachineList->categoryMap.isEmpty() ) {
			QString category = gameItem->text(QMC2_MACHINELIST_COLUMN_CATEGORY);
			if ( excludedCategories.contains(category) )
				continue;
		}

		// driver status
		if ( minDrvStatus < QMC2_ARCADE_DRV_STATUS_PRELIMINARY ) {
			QString drvStatus = gameItem->text(QMC2_MACHINELIST_COLUMN_DRVSTAT);
			if ( minDrvStatus == QMC2_ARCADE_DRV_STATUS_IMPERFECT ) {
				if ( drvStatus != tr("good") && drvStatus != tr("imperfect") )
					continue;
			} else {
				if ( drvStatus != tr("good") )
					continue;
			}
		}

		// ROM status
		switch ( qmc2MachineList->romState(game) ) {
			case 'C':
				if ( toolButtonSelectC->isChecked() )
					selectedGames << gameItem;
				break;
			case 'M':
				if ( toolButtonSelectM->isChecked() )
					selectedGames << gameItem;
				break;
			case 'I':
				if ( toolButtonSelectI->isChecked() )
					selectedGames << gameItem;
				break;
			case 'N':
				if ( toolButtonSelectN->isChecked() )
					selectedGames << gameItem;
				break;
			case 'U':
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
	std::sort(selectedGames.begin(), selectedGames.end(), ArcadeModeSetup::lessThan);

	progressBarFilter->setValue(0);
	progressBarFilter->setFormat(tr("Exporting"));
	for (int i = 0; i < selectedGames.count(); i++) {
		progressBarFilter->setValue(i + 1);
		MachineListItem *gameItem = selectedGames[i];
		QString gameName = gameItem->text(QMC2_MACHINELIST_COLUMN_NAME);
		ts << gameName << "\t"
		   << gameItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) << "\t"
		   << gameItem->text(QMC2_MACHINELIST_COLUMN_MANU) << "\t"
		   << gameItem->text(QMC2_MACHINELIST_COLUMN_YEAR) << "\t"
		   << qmc2ParentHash[gameName] << "\t"
	   	   << (qmc2MachineList->isBios(gameName) ? "1": "0") << "\t"
		   << (gameItem->text(QMC2_MACHINELIST_COLUMN_RTYPES).contains(tr("ROM")) ? "1" : "0") << "\t"
		   << (gameItem->text(QMC2_MACHINELIST_COLUMN_RTYPES).contains(tr("CHD")) ? "1": "0") << "\t"
		   << gameItem->text(QMC2_MACHINELIST_COLUMN_PLAYERS) << "\t"
		   << gameItem->text(QMC2_MACHINELIST_COLUMN_DRVSTAT) << "\t"
		   << "0\t"
		   << gameItem->text(QMC2_MACHINELIST_COLUMN_SRCFILE) << "\n";
	}

	progressBarFilter->setRange(0, 100);
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

bool ArcadeModeSetup::lessThan(const MachineListItem *item1, const MachineListItem *item2)
{
	switch ( qmc2ArcadeModeSortCriteria ) {
		case QMC2_SORT_BY_DESCRIPTION:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_MACHINE).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_MACHINE).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_MACHINE).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_MACHINE).toUpper());
		case QMC2_SORT_BY_ROM_STATE:
			if ( qmc2ArcadeModeSortOrder )
				return (qmc2MachineList->romState(item1->text(QMC2_MACHINELIST_COLUMN_NAME)) > qmc2MachineList->romState(item2->text(QMC2_MACHINELIST_COLUMN_NAME)));
			else
				return (qmc2MachineList->romState(item1->text(QMC2_MACHINELIST_COLUMN_NAME)) < qmc2MachineList->romState(item2->text(QMC2_MACHINELIST_COLUMN_NAME)));
		case QMC2_SORT_BY_TAG:
			if ( qmc2ArcadeModeSortOrder )
				return (int(item1->checkState(QMC2_MACHINELIST_COLUMN_TAG)) > int(item2->checkState(QMC2_MACHINELIST_COLUMN_TAG)));
			else
				return (int(item1->checkState(QMC2_MACHINELIST_COLUMN_TAG)) < int(item2->checkState(QMC2_MACHINELIST_COLUMN_TAG)));
		case QMC2_SORT_BY_YEAR:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_YEAR) > item2->text(QMC2_MACHINELIST_COLUMN_YEAR));
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_YEAR) < item2->text(QMC2_MACHINELIST_COLUMN_YEAR));
		case QMC2_SORT_BY_MANUFACTURER:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_MANU).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_MANU).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_MANU).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_MANU).toUpper());
		case QMC2_SORT_BY_NAME:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_NAME).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_NAME).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_NAME).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_NAME).toUpper());
		case QMC2_SORT_BY_ROMTYPES:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_RTYPES).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_RTYPES).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_RTYPES).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_RTYPES).toUpper());
		case QMC2_SORT_BY_PLAYERS:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_PLAYERS).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_PLAYERS).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_PLAYERS).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_PLAYERS).toUpper());
		case QMC2_SORT_BY_DRVSTAT:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_DRVSTAT).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_DRVSTAT).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_DRVSTAT).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_DRVSTAT).toUpper());
		case QMC2_SORT_BY_SRCFILE:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_SRCFILE).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_SRCFILE).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_SRCFILE).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_SRCFILE).toUpper());
		case QMC2_SORT_BY_RANK:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->whatsThis(QMC2_MACHINELIST_COLUMN_RANK).toInt() < item2->whatsThis(QMC2_MACHINELIST_COLUMN_RANK).toInt());
			else
				return (item1->whatsThis(QMC2_MACHINELIST_COLUMN_RANK).toInt() > item2->whatsThis(QMC2_MACHINELIST_COLUMN_RANK).toInt());
		case QMC2_SORT_BY_CATEGORY:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_CATEGORY).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_CATEGORY).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_CATEGORY).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_CATEGORY).toUpper());
		case QMC2_SORT_BY_VERSION:
			if ( qmc2ArcadeModeSortOrder )
				return (item1->text(QMC2_MACHINELIST_COLUMN_VERSION).toUpper() > item2->text(QMC2_MACHINELIST_COLUMN_VERSION).toUpper());
			else
				return (item1->text(QMC2_MACHINELIST_COLUMN_VERSION).toUpper() < item2->text(QMC2_MACHINELIST_COLUMN_VERSION).toUpper());
		default:
			return qmc2ArcadeModeSortOrder == 1;
	}
}
