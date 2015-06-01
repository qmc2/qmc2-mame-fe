#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#endif
#include <QMap>
#include <QHash>

#include <algorithm> // std::sort()

#include "messdevcfg.h"
#include "machinelist.h"
#include "macros.h"
#include "qmc2main.h"
#include "options.h"
#include "fileeditwidget.h"
#include "iconlineedit.h"
#include "fileiconprovider.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern MachineList *qmc2MachineList;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern QString qmc2FileEditStartPath;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;
extern int qmc2DefaultLaunchMode;
extern bool qmc2CriticalSection;
extern MESSDeviceConfigurator *qmc2MESSDeviceConfigurator;
extern bool qmc2UseDefaultEmulator;
extern bool qmc2TemplateCheck;
extern Options *qmc2Options;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;

QList<FileEditWidget *> messFileEditWidgetList;
QHash<QString, QHash<QString, QStringList> > messSystemSlotHash;
QHash<QString, QString> messSlotNameHash;
QHash<QString, QIcon> messDevIconHash;
bool messSystemSlotsSupported = true;

MESSDeviceFileDelegate::MESSDeviceFileDelegate(QObject *parent)
	: QItemDelegate(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::MESSDeviceFileDelegate(QObject *parent = %1)").arg((qulonglong)parent));
#endif

	messFileEditWidgetList.clear();
}

QWidget *MESSDeviceFileDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::createEditor(QWidget *parent = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)parent));
#endif

	int row = index.row();
	QModelIndex sibling = index.sibling(row, QMC2_DEVCONFIG_COLUMN_EXT);
	QStringList extensions = sibling.model()->data(sibling, Qt::EditRole).toString().split("/", QString::SkipEmptyParts);
	QString filterString = tr("All files") + " (*)";
	if ( extensions.count() > 0 ) {
#if defined(QMC2_OS_WIN)
		filterString = tr("Valid device files") + " (*.zip";
#else
		filterString = tr("Valid device files") + " (*.[zZ][iI][pP]";
#endif
		for (int i = 0; i < extensions.count(); i++) {
			QString ext = extensions[i];
#if !defined(QMC2_OS_WIN)
			QString altExt;
			for (int j = 0; j < ext.length(); j++) {
				QChar c = ext[j].toLower();
				altExt += QString("[%1%2]").arg(c).arg(c.toUpper());
			}
			filterString += QString(" *.%1").arg(altExt);
#else
			filterString += QString(" *.%1").arg(ext);
#endif
		}
		filterString += ");;" + tr("All files") + " (*)";
	}
	FileEditWidget *fileEditWidget = new FileEditWidget("", filterString, "", parent, true);
	fileEditWidget->installEventFilter(const_cast<MESSDeviceFileDelegate*>(this));
	connect(fileEditWidget, SIGNAL(dataChanged(QWidget *)), this, SLOT(dataChanged(QWidget *)));
	messFileEditWidgetList.insert(row, fileEditWidget);
	return fileEditWidget;
}

void MESSDeviceFileDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::setEditorData(QWidget *editor = %1, const QModelIndex &index)").arg((qulonglong)editor));
#endif

	QString value = index.model()->data(index, Qt::EditRole).toString();
	FileEditWidget *fileEditWidget = static_cast<FileEditWidget *>(editor);
	int cPos = fileEditWidget->lineEditFile->cursorPosition();
	fileEditWidget->lineEditFile->setText(value);
	fileEditWidget->lineEditFile->setCursorPosition(cPos);
}

void MESSDeviceFileDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::setModelData(QWidget *editor = %1, QAbstractItemModel *model = %2, const QModelIndex &index)").arg((qulonglong)editor).arg((qulonglong)model));
#endif

	FileEditWidget *fileEditWidget = static_cast<FileEditWidget*>(editor);
	QString v = fileEditWidget->lineEditFile->text();
	model->setData(index, v, Qt::EditRole);
}

void MESSDeviceFileDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::updateEditorGeometry(QWidget *editor = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)editor));
#endif

	editor->setGeometry(option.rect);
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	FileEditWidget *fileEditWidget = static_cast<FileEditWidget *>(editor);
	fileEditWidget->toolButtonBrowse->setIconSize(iconSize);
}

void MESSDeviceFileDelegate::dataChanged(QWidget *widget)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::dataChanged(QWidget *widget = %1)").arg((qulonglong)widget));
#endif

	emit commitData(widget);
	FileEditWidget *fileEditWidget = static_cast<FileEditWidget*>(widget);
	emit editorDataChanged(fileEditWidget->lineEditFile->text());
}

MESSDeviceConfigurator::MESSDeviceConfigurator(QString machineName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::MESSDeviceConfigurator(QString machineName = %1, QWidget *parent = %2)").arg(machineName).arg((qulonglong)parent));
#endif

	setupUi(this);

	listWidgetDeviceConfigurations->setSortingEnabled(false);
	listWidgetDeviceConfigurations->blockSignals(true);
	listWidgetDeviceConfigurations->setCurrentItem(listWidgetDeviceConfigurations->item(0));
	listWidgetDeviceConfigurations->blockSignals(false);

	tabWidgetDeviceSetup->setCornerWidget(toolButtonConfiguration, Qt::TopRightCorner);

#if (!defined(QMC2_OS_UNIX) && !defined(QMC2_OS_WIN)) || QT_VERSION >= 0x050000
	toolButtonChooserPlayEmbedded->setVisible(false);
#endif

	connect(&searchTimer, SIGNAL(timeout()), this, SLOT(comboBoxChooserFilterPattern_editTextChanged_delayed()));
	comboBoxChooserFilterPattern->setLineEdit(new IconLineEdit(QIcon(QString::fromUtf8(":/data/img/find.png")), QMC2_ALIGN_LEFT, comboBoxChooserFilterPattern));
	comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Enter search string"));
	comboBoxChooserFilterPatternHadFocus = false;
	dirModel = NULL;
	fileModel = NULL;
	configurationRenameItem = NULL;
	fileChooserSetup = refreshRunning = dontIgnoreNameChange = isLoading = isManualSlotOptionChange = fullyLoaded = forceQuit = false;
	updateSlots = true;

	lineEditConfigurationName->blockSignals(true);
	if ( messSystemSlotHash.isEmpty() )
		lineEditConfigurationName->setText(tr("Reading slot info, please wait..."));
	else
		lineEditConfigurationName->setText(tr("Default configuration"));
	lineEditConfigurationName->setPlaceholderText(tr("Enter configuration name"));
	lineEditConfigurationName->blockSignals(false);

	messMachineName = machineName;
	treeWidgetDeviceSetup->setItemDelegateForColumn(QMC2_DEVCONFIG_COLUMN_FILE, &fileEditDelegate);
	connect(&fileEditDelegate, SIGNAL(editorDataChanged(const QString &)), this, SLOT(editorDataChanged(const QString &)));
	tabWidgetDeviceSetup->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DeviceSetupTab", 0).toInt());
	treeWidgetDeviceSetup->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DeviceSetupHeaderState").toByteArray());
	treeWidgetSlotOptions->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/SlotSetupHeaderState").toByteArray());
	toolButtonChooserAutoSelect->blockSignals(true);
	toolButtonChooserAutoSelect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserAutoSelect", false).toBool());
	toolButtonChooserAutoSelect->blockSignals(false);
	toolButtonChooserFilter->blockSignals(true);
	toolButtonChooserFilter->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserFilter", false).toBool());
	toolButtonChooserFilter->blockSignals(false);
	toolButtonChooserProcessZIPs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserProcessZIPs", false).toBool());
	toolButtonChooserMergeMaps->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserMergeMaps", false).toBool());
	QString folderMode = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FolderMode", "folders-off").toString();
	if ( folderMode == "folders-first" ) {
		toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-first.png")));
		includeFolders = true;
		foldersFirst = true;
	} else if ( folderMode == "folders-on" ) {
		toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-on.png")));
		includeFolders = true;
		foldersFirst = false;
	} else {
		toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-off.png")));
		includeFolders = false;
		foldersFirst = false;
	}

	QList<int> splitterSizes;
	QSize splitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserSplitter").toSize();
	if ( splitterSize.width() > 0 || splitterSize.height() > 0 )
		splitterSizes << splitterSize.width() << splitterSize.height();
	else
		splitterSizes << 30 << 70;
	splitterFileChooser->setSizes(splitterSizes);

	QList<int> vSplitterSizes;
	QSize vSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/vSplitter").toSize();
	if ( vSplitterSize.width() > 0 || vSplitterSize.height() > 0 )
		vSplitterSizes << vSplitterSize.width() << vSplitterSize.height();
	else
		vSplitterSizes << 100 << 100;
	vSplitter->setSizes(vSplitterSizes);

	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonConfiguration->setIconSize(iconSize);
	toolButtonNewConfiguration->setIconSize(iconSize);
	toolButtonCloneConfiguration->setIconSize(iconSize);
	toolButtonSaveConfiguration->setIconSize(iconSize);
	toolButtonRemoveConfiguration->setIconSize(iconSize);
	toolButtonChooserPlay->setIconSize(iconSize);
	toolButtonChooserPlayEmbedded->setIconSize(iconSize);
	toolButtonChooserReload->setIconSize(iconSize);
	toolButtonChooserClearFilterPattern->setIconSize(iconSize);
	toolButtonChooserAutoSelect->setIconSize(iconSize);
	toolButtonChooserFilter->setIconSize(iconSize);
	toolButtonChooserProcessZIPs->setIconSize(iconSize);
	toolButtonChooserMergeMaps->setIconSize(iconSize);
	toolButtonChooserSaveConfiguration->setIconSize(iconSize);
	toolButtonFolderMode->setIconSize(iconSize);
	comboBoxDeviceInstanceChooser->setIconSize(iconSize);
	treeWidgetDeviceSetup->setIconSize(iconSize);
	treeWidgetSlotOptions->setIconSize(iconSize);

	connect(listWidgetDeviceConfigurations, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(configurationItemChanged(QListWidgetItem *)));

	// configuration menu
	configurationMenu = new QMenu(toolButtonConfiguration);
	QString s = tr("Select default device directory");
	QAction *action = configurationMenu->addAction(tr("&Default device directory for '%1'...").arg(messMachineName));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(actionSelectDefaultDeviceDirectory_triggered()));
	toolButtonConfiguration->setMenu(configurationMenu);

	// device configuration list context menu
	deviceConfigurationListMenu = new QMenu(this);
	s = tr("Play selected machine");
	action = deviceConfigurationListMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlay_triggered()));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Play selected machine (embedded)");
	action = deviceConfigurationListMenu->addAction(tr("Play &embedded"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlayEmbedded_triggered()));
#endif
	deviceConfigurationListMenu->addSeparator();
	s = tr("Rename configuration");
	action = deviceConfigurationListMenu->addAction(tr("Re&name configuration"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rename.png")));
	actionRenameConfiguration = action;
	connect(action, SIGNAL(triggered()), this, SLOT(actionRenameConfiguration_activated()));
	s = tr("Remove configuration");
	action = deviceConfigurationListMenu->addAction(tr("&Remove configuration"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/remove.png")));
	actionRemoveConfiguration = action;
	connect(action, SIGNAL(triggered()), this, SLOT(actionRemoveConfiguration_activated()));

	// device instance list context menu
	deviceContextMenu = new QMenu(this);
	s = tr("Select a file to be mapped to this device instance");
	action = deviceContextMenu->addAction(tr("Select file..."));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(actionSelectFile_triggered()));

	// slot options context menu
	slotContextMenu = new QMenu(this);
	// FIXME: initially left blank :)

	// directory chooser context menu
	dirChooserContextMenu = new QMenu(this);
	s = tr("Use as default directory");
	action = dirChooserContextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	QIcon icon = FileIconProvider::folderIcon();
	if ( icon.isNull() )
		icon = QIcon(QString::fromUtf8(":/data/img/folders-on.png"));
	action->setIcon(icon);
	connect(action, SIGNAL(triggered()), this, SLOT(dirChooserUseCurrentAsDefaultDirectory()));

	// file chooser context menu
	fileChooserContextMenu = new QMenu(this);
	s = tr("Play selected machine");
	action = fileChooserContextMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlay_triggered()));
	actionChooserPlay = action;
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Play selected machine (embedded)");
	action = fileChooserContextMenu->addAction(tr("Play &embedded"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlayEmbedded_triggered()));
	actionChooserPlayEmbedded = action;
#endif
	fileChooserContextMenu->addSeparator();
	action = fileChooserContextMenu->addAction(tr("&Open folder"));
	action->setToolTip(s); action->setStatusTip(s);
	icon = FileIconProvider::folderIcon();
	if ( icon.isNull() )
		icon = QIcon(QString::fromUtf8(":/data/img/folders-on.png"));
	action->setIcon(icon);
	connect(action, SIGNAL(triggered()), this, SLOT(treeViewFileChooser_openFolder()));
	actionChooserOpenFolder = action;

	fileChooserContextMenu->addSeparator();
	action = fileChooserContextMenu->addAction(tr("&Open archive"));
	action->setToolTip(s); action->setStatusTip(s);
	icon = FileIconProvider::fileIcon("dummy.zip");
	if ( icon.isNull() )
		icon = QIcon(QString::fromUtf8(":/data/img/compressed.png"));
	action->setIcon(icon);
	connect(action, SIGNAL(triggered()), this, SLOT(treeViewFileChooser_toggleArchive()));
	actionChooserToggleArchive = action;

	action = fileChooserContextMenu->addAction(tr("View PDF..."));
	action->setToolTip(s); action->setStatusTip(s);
	icon = FileIconProvider::fileIcon("dummy.pdf");
	if ( icon.isNull() )
		icon = QIcon(QString::fromUtf8(":/data/img/pdf.png"));
	action->setIcon(icon);
	connect(action, SIGNAL(triggered()), this, SLOT(treeViewFileChooser_viewPdf()));
	actionChooserViewPdf = action;

	action = fileChooserContextMenu->addAction(tr("View Postscript..."));
	action->setToolTip(s); action->setStatusTip(s);
	icon = FileIconProvider::fileIcon("dummy.ps");
	if ( icon.isNull() )
		icon = QIcon(QString::fromUtf8(":/data/img/postscript.png"));
	action->setIcon(icon);
	connect(action, SIGNAL(triggered()), this, SLOT(treeViewFileChooser_viewPdf()));
	actionChooserViewPostscript = action;

	action = fileChooserContextMenu->addAction(tr("View HTML..."));
	action->setToolTip(s); action->setStatusTip(s);
	icon = FileIconProvider::fileIcon("dummy.html");
	if ( icon.isNull() )
		icon = QIcon(QString::fromUtf8(":/data/img/html.png"));
	action->setIcon(icon);
	connect(action, SIGNAL(triggered()), this, SLOT(treeViewFileChooser_viewHtml()));
	actionChooserViewHtml = action;

	fileChooserContextMenu->addSeparator();
	action = fileChooserContextMenu->addAction(tr("Open e&xternally..."));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(treeViewFileChooser_openFileExternally()));
	actionChooserOpenExternally = action;

	// folder mode menu
	folderModeMenu = new QMenu(this);
	s = tr("Hide folders");
	action = folderModeMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-off.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(folderModeMenu_foldersOff()));
	s = tr("Show folders");
	action = folderModeMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-on.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(folderModeMenu_foldersOn()));
	s = tr("Show folders first");
	action = folderModeMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-first.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(folderModeMenu_foldersFirst()));
	toolButtonFolderMode->setMenu(folderModeMenu);

	if ( messDevIconHash.isEmpty() ) {
		messDevIconHash["cartridge"] = QIcon(QString::fromUtf8(":/data/img/dev_cartridge.png"));
		messDevIconHash["cassette"] = QIcon(QString::fromUtf8(":/data/img/dev_cassette.png"));
		messDevIconHash["cdrom"] = QIcon(QString::fromUtf8(":/data/img/dev_cdrom.png"));
		messDevIconHash["cylinder"] = QIcon(QString::fromUtf8(":/data/img/dev_cylinder.png"));
		messDevIconHash["floppydisk"] = QIcon(QString::fromUtf8(":/data/img/dev_floppydisk.png"));
		messDevIconHash["harddisk"] = QIcon(QString::fromUtf8(":/data/img/dev_harddisk.png"));
		messDevIconHash["magtape"] = QIcon(QString::fromUtf8(":/data/img/dev_magtape.png"));
		messDevIconHash["memcard"] = QIcon(QString::fromUtf8(":/data/img/dev_memcard.png"));
		messDevIconHash["parallel"] = QIcon(QString::fromUtf8(":/data/img/dev_parallel.png"));
		messDevIconHash["printer"] = QIcon(QString::fromUtf8(":/data/img/dev_printer.png"));
		messDevIconHash["punchtape"] = QIcon(QString::fromUtf8(":/data/img/dev_punchtape.png"));
		messDevIconHash["quickload"] = QIcon(QString::fromUtf8(":/data/img/dev_quickload.png"));
		messDevIconHash["serial"] = QIcon(QString::fromUtf8(":/data/img/dev_serial.png"));
		messDevIconHash["snapshot"] = QIcon(QString::fromUtf8(":/data/img/dev_snapshot.png"));
		messDevIconHash["romimage"] = QIcon(QString::fromUtf8(":/data/img/rom.png"));
	}

	FileChooserKeyEventFilter *eventFilter = new FileChooserKeyEventFilter(this);
	treeViewFileChooser->installEventFilter(eventFilter);
	connect(eventFilter, SIGNAL(expandRequested()), this, SLOT(treeViewFileChooser_expandRequested()));
}

MESSDeviceConfigurator::~MESSDeviceConfigurator()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::~MESSDeviceConfigurator()");
#endif

}

void MESSDeviceConfigurator::saveSetup()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::saveSetup()");
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DeviceSetupHeaderState", treeWidgetDeviceSetup->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/SlotSetupHeaderState", treeWidgetSlotOptions->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DeviceSetupTab", tabWidgetDeviceSetup->currentIndex());
	if ( tabWidgetDeviceSetup->currentIndex() != QMC2_DEVSETUP_TAB_FILECHOOSER )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/vSplitter", QSize(vSplitter->sizes().at(0), vSplitter->sizes().at(1)));
	else
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserSplitter", QSize(splitterFileChooser->sizes().at(0), splitterFileChooser->sizes().at(1)));
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserFilter", toolButtonChooserFilter->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserAutoSelect", toolButtonChooserAutoSelect->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserProcessZIPs", toolButtonChooserProcessZIPs->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserMergeMaps", toolButtonChooserMergeMaps->isChecked());
	if ( !fileChooserHeaderState.isEmpty() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserHeaderState", fileChooserHeaderState);
	if ( !dirChooserHeaderState.isEmpty() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DirChooserHeaderState", dirChooserHeaderState);
	if ( comboBoxDeviceInstanceChooser->currentText() != tr("No devices available") )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserDeviceInstance", comboBoxDeviceInstanceChooser->currentText());
	if ( includeFolders && foldersFirst )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FolderMode", "folders-first");
	else if ( includeFolders )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FolderMode", "folders-on");
	else
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FolderMode", "folders-off");
}

bool MESSDeviceConfigurator::checkParentSlot(QTreeWidgetItem *item, QString &slotName)
{
	QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
	if ( cb ) {
		QString currentSlotName = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT);
		if ( slotName.startsWith(currentSlotName + ":" + cb->currentText().split(" ")[0]) ) {
			if ( item->parent() )
				return checkParentSlot(item->parent(), currentSlotName);
			else
				return true;
		} else
			return false;
	} else
		return false;
}

QString &MESSDeviceConfigurator::getXmlDataWithEnabledSlots(QString machineName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::getXmlDataWithEnabledSlots(QString machineName = %1)").arg(machineName));
#endif

	qmc2CriticalSection = true;
	slotXmlBuffer.clear();

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	QProcess commandProc;
#if defined(QMC2_SDLMAME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#endif
#if !defined(QMC2_OS_WIN)
	commandProc.setStandardErrorFile("/dev/null");
#endif

	QStringList args;

	args << machineName;

	QList<QTreeWidgetItem *> allSlotItems;
	for (int i = 0; i < treeWidgetSlotOptions->topLevelItemCount() && !forceQuit; i++) {
		QTreeWidgetItem *item = treeWidgetSlotOptions->topLevelItem(i);
		allSlotItems << item;
		insertChildItems(item, allSlotItems);
	}

	foreach (QTreeWidgetItem *item, allSlotItems) {
		if ( forceQuit )
			break;
		QString slotName = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT);
		if ( !slotName.isEmpty() ) {
			bool isNestedSlot = !messSystemSlotHash[messMachineName].contains(slotName);
			QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
			if ( cb ) {
				int defaultIndex = -1;
				bool addArg = false;
				if ( slotPreselectionMap.contains(cb) )
					defaultIndex = slotPreselectionMap[cb];
				else if ( nestedSlotPreselectionMap.contains(cb) )
					defaultIndex = nestedSlotPreselectionMap[cb];
				if ( isNestedSlot ) {
					// there must be a "parent" slot, otherwise ignore this argument
					if ( item->parent() != NULL )
						addArg = checkParentSlot(item->parent(), slotName);
				} else
					addArg = true;
#ifdef QMC2_DEBUG
				printf("MESSDeviceConfigurator::getXmlDataWithEnabledSlots(): slotName = %s, isNested = %s, defaultIndex = %d, addArg = %s\n", (const char *)slotName.toLocal8Bit(), isNestedSlot ? "true" : "false", defaultIndex, addArg ? "true" : "false");
#endif
				if ( addArg ) {
					if ( cb->currentIndex() > 0 && defaultIndex == 0 )
						args << QString("-%1").arg(slotName) << cb->currentText().split(" ")[0];
					else if ( cb->currentIndex() == 0 && defaultIndex > 0 )
						args << QString("-%1").arg(slotName) << "\"\"";
					else if ( cb->currentIndex() > 0 && defaultIndex > 0 && cb->currentIndex() != defaultIndex )
						args << QString("-%1").arg(slotName) << cb->currentText().split(" ")[0];
				}
			}
		}
	}

	if ( forceQuit )
		return slotXmlBuffer;

	args << "-listxml";

#ifdef QMC2_DEBUG
	printf("MESSDeviceConfigurator::getXmlDataWithEnabledSlots(): args = %s\n", (const char *)args.join(" ").toLocal8Bit());
#endif

	bool commandProcStarted = false;
	int retries = 0;
	commandProc.start(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString(), args);
	bool started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
	while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
		started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
		qApp->processEvents();
	}

	if ( started ) {
		commandProcStarted = true;
		bool commandProcRunning = (commandProc.state() == QProcess::Running);
		while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning && !forceQuit ) {
			qApp->processEvents();
			commandProcRunning = (commandProc.state() == QProcess::Running);
		}
	} else {
		if ( !forceQuit )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start emulator executable within a reasonable time frame, giving up"));
		qmc2CriticalSection = false;
		return slotXmlBuffer;
	}

#if defined(QMC2_SDLMAME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#endif

	if ( commandProcStarted && qmc2TempXml.open(QFile::ReadOnly) ) {
		QTextStream ts(&qmc2TempXml);
		slotXmlBuffer = ts.readAll();
#if defined(QMC2_OS_WIN)
		slotXmlBuffer.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
		qmc2TempXml.close();
		qmc2TempXml.remove();
		if ( !slotXmlBuffer.isEmpty() ) {
			QStringList xmlLines = slotXmlBuffer.split("\n");
			qApp->processEvents();
			slotXmlBuffer.clear();
			if ( !xmlLines.isEmpty() ) {
				int i = 0;
				QString s = "<machine name=\"" + machineName + "\"";
				while ( i < xmlLines.count() && !xmlLines[i].contains(s) ) i++;
				slotXmlBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
				if ( i < xmlLines.count() ) {
					while ( i < xmlLines.count() && !xmlLines[i].contains("</machine>") )
						slotXmlBuffer += xmlLines[i++].simplified() + "\n";
					if ( i == xmlLines.count() && !xmlLines[i - 1].contains("</machine>") ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: invalid XML data retrieved for '%1'").arg(machineName));
						slotXmlBuffer.clear();
					} else
						slotXmlBuffer += "</machine>\n";
				} else {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: invalid XML data retrieved for '%1'").arg(machineName));
					slotXmlBuffer.clear();
				}
			}
		}
	}

	qmc2CriticalSection = false;
	return slotXmlBuffer;
}

QString &MESSDeviceConfigurator::getXmlData(QString machineName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::getXmlData(QString machineName = %1)").arg(machineName));
#endif

	normalXmlBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	normalXmlBuffer += qmc2MachineList->xmlDb()->xml(machineName);
	return normalXmlBuffer;
}

bool MESSDeviceConfigurator::readSystemSlots()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::readSystemSlots()");
#endif

	QTime elapsedTime(0, 0, 0, 0);
	QTime loadTimer;

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	QString slotInfoCachePath;
	slotInfoCachePath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SlotInfoCacheFile", userScopePath + "/mame.sic").toString();

	setEnabled(false);
	lineEditConfigurationName->blockSignals(true);
	lineEditConfigurationName->setText(tr("Reading slot info, please wait..."));
	lineEditConfigurationName->blockSignals(false);

	loadTimer.start();

	bool fromCache = true;
	bool commandProcStarted = false;
	QFile slotInfoFile(slotInfoCachePath);
	listWidgetDeviceConfigurations->setUpdatesEnabled(true);
	qApp->processEvents();
	listWidgetDeviceConfigurations->setUpdatesEnabled(false);
	if ( !slotInfoFile.exists() ) {
		fromCache = false;
		if ( slotInfoFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) ) {
			QTextStream ts(&slotInfoFile);
			ts << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
			ts << "MAME_VERSION\t" + qmc2MachineList->emulatorVersion + "\n";
			slotInfoFile.close();
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open slot info cache for writing, path = %1").arg(slotInfoCachePath));
			lineEditConfigurationName->blockSignals(true);
			lineEditConfigurationName->setText(tr("Failed to read slot info"));
			lineEditConfigurationName->blockSignals(false);
			return false;
		}
		QProcess commandProc;
		commandProc.setStandardOutputFile(slotInfoCachePath, QIODevice::Append);
#if !defined(QMC2_OS_WIN)
		commandProc.setStandardErrorFile("/dev/null");
#endif
		QStringList args;
		args << "-listslots";
		
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading available system slots and recreating cache"));
		qApp->processEvents();

		int retries = 0;
		commandProc.start(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString(), args);
		bool started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
		while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
			qApp->processEvents();
			started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
		}
		if ( started ) {
			commandProcStarted = true;
			bool commandProcRunning = (commandProc.state() == QProcess::Running);
			while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
				qApp->processEvents();
				commandProcRunning = (commandProc.state() == QProcess::Running);
			}
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MAME executable within a reasonable time frame, giving up"));
			lineEditConfigurationName->blockSignals(true);
			lineEditConfigurationName->setText(tr("Failed to read slot info"));
			lineEditConfigurationName->blockSignals(false);
			return false;
		}

		if ( commandProc.exitStatus() == QProcess::CrashExit )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external command used for reading the available system slots crashed, slot-options may not be complete"));
	}

	bool retVal = true;
	if ( (fromCache || commandProcStarted) && slotInfoFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream ts(&slotInfoFile);

		QString slotLine = ts.readLine(); // comment line
		slotLine = ts.readLine();

		QStringList versionWords = slotLine.split("\t");
		bool sameVersion = false;

		if ( versionWords.count() >= 2 ) {
			if ( versionWords[0] == "MAME_VERSION" )
				sameVersion = (versionWords[1] == qmc2MachineList->emulatorVersion);
		}

		if ( !sameVersion ) {
			slotInfoFile.close();
			slotInfoFile.remove();
			return readSystemSlots();
		}

		if ( fromCache ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading available system slots from cache"));
			loadTimer.start();
		}

		QString systemName, slotName, slotOption, slotDeviceName;
		QRegExp rxSlotDev1("^\\S+\\s+\\S+\\s+\\S+\\s+");
		QRegExp rxSlotDev2("^\\S+\\s+");
		QRegExp rxSlotDev3("^\\S+\\s+\\S+\\s+");
		QString strNone("[none]");
		QString strUnused("QMC2_UNUSED_SLOTS");

		int lineCounter = 0;
		while ( !ts.atEnd() ) {
			slotLine = ts.readLine();
			QString slotLineTrimmed = slotLine.trimmed();
			if ( lineCounter++ % QMC2_SLOTINFO_READ_RESPONSE == 0 )
				qApp->processEvents();
			if ( !slotLineTrimmed.isEmpty() ) {
				if ( !slotLine.startsWith(" ") ) {
					QStringList slotWords = slotLineTrimmed.split(" ", QString::SkipEmptyParts);
					if ( slotWords.count() >= 4 ) {
						systemName = slotWords[0];
						slotName = slotWords[1];
						if ( slotName.split(":", QString::SkipEmptyParts).count() < 3 && slotWords.count() > 2 ) {
							slotOption = slotWords[2];
							if ( slotOption != strNone ) {
								slotDeviceName = slotLineTrimmed;
								slotDeviceName.remove(rxSlotDev1);
								messSlotNameHash[slotOption] = slotDeviceName;
								messSystemSlotHash[systemName][slotName] << slotOption;
							} else
								messSystemSlotHash[systemName][strUnused] << slotName;
						} else
							messSystemSlotHash[systemName].remove(slotName);
					} else {
						systemName = slotWords[0];
						messSystemSlotHash[systemName].clear();
					}
				} else {
					QStringList slotWords = slotLineTrimmed.split(" ", QString::SkipEmptyParts);
					if ( slotLine[13] == ' ' ) { // this isn't nice, but I see no other way at the moment...
						if ( slotName.split(":", QString::SkipEmptyParts).count() < 3 ) {
							slotOption = slotWords[0];
							if ( slotOption != strNone ) {
								slotDeviceName = slotLineTrimmed;
								slotDeviceName.remove(rxSlotDev2);
								messSlotNameHash[slotOption] = slotDeviceName;
								messSystemSlotHash[systemName][slotName] << slotOption;
							} else
								messSystemSlotHash[systemName][strUnused] << slotName;
						}
					} else {
						slotName = slotWords[0];
						if ( slotName.split(":", QString::SkipEmptyParts).count() < 3 && slotWords.count() > 1 ) {
							slotOption = slotWords[1];
							if ( slotOption != strNone ) {
								slotDeviceName = slotLineTrimmed;
								slotDeviceName.remove(rxSlotDev3);
								messSlotNameHash[slotOption] = slotDeviceName;
								messSystemSlotHash[systemName][slotName] << slotOption;
							} else
								messSystemSlotHash[systemName][strUnused] << slotName;
						} else
							messSystemSlotHash[systemName].remove(slotName);
					}
				}
			}
		}
		slotInfoFile.close();
	} else {
		lineEditConfigurationName->blockSignals(true);
		lineEditConfigurationName->setText(tr("Failed to read slot info"));
		lineEditConfigurationName->blockSignals(false);
		retVal = false;
	}
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	if ( fromCache )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading available system slots from cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading available system slots and recreating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	setEnabled(qmc2UseDefaultEmulator);
	return retVal;
}

void MESSDeviceConfigurator::slotOptionChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::slotOptionChanged(int index = %1)").arg(index));
#endif

	isManualSlotOptionChange = true;
	QTimer::singleShot(QMC2_SLOTOPTION_CHANGE_DELAY, this, SLOT(refreshDeviceMap()));
}

void MESSDeviceConfigurator::addNestedSlot(QString slotName, QStringList slotOptionNames, QStringList slotOptionDescriptions, QString defaultSlotOption)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::addNestedSlot(QString slotName = %1, QStringList slotOptionNames = ..., QStringList slotOptionDescriptions = ..., QString defaultSlotOption = %2)").arg(slotName).arg(defaultSlotOption));
#endif

	if ( nestedSlots.contains(slotName) )
		return;

	QStringList slotOptions;
	QStringList slotOptionsShort;
	int count = 0;
	foreach (QString s, slotOptionNames) {
		slotOptions << QString("%1 - %2").arg(s).arg(slotOptionDescriptions[count]);
		slotOptionsShort << s;
		nestedSlotOptionMap[slotName][s] = slotOptionDescriptions[count++];
	}
	slotOptions.sort();
	slotOptionsShort.sort();
	QComboBox *cb = new QComboBox(0);
	cb->setAutoFillBackground(true);
	cb->setMaxVisibleItems(20);
	if ( defaultSlotOption.isEmpty() ) {
		cb->insertItem(0, tr("not used") + " / " + tr("default"));
		if ( slotOptions.count() > 0 ) {
			cb->insertSeparator(1);
			cb->insertItems(2, slotOptions);
		}
		nestedSlotPreselectionMap[cb] = 0;
		newNestedSlotPreselectionMap[cb] = 0;
	} else {
		cb->insertItem(0, tr("not used"));
		if ( slotOptions.count() > 0 ) {
			cb->insertSeparator(1);
			cb->insertItems(2, slotOptions);
			int defaultIndex = slotOptionsShort.indexOf(defaultSlotOption);
			nestedSlotPreselectionMap[cb] = defaultIndex + 2;
			newNestedSlotPreselectionMap[cb] = defaultIndex + 2;
			cb->setItemText(defaultIndex + 2, slotOptions[defaultIndex] + " / " + tr("default"));
		} else {
			nestedSlotPreselectionMap[cb] = 0;
			newNestedSlotPreselectionMap[cb] = 0;
		}
	}

	// find parent slot
	QTreeWidgetItem *parentItem = NULL;
	QList<QTreeWidgetItem *> allSlotItems;
	for (int i = 0; i < treeWidgetSlotOptions->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidgetSlotOptions->topLevelItem(i);
		allSlotItems << item;
		insertChildItems(item, allSlotItems);
	}
	foreach (QTreeWidgetItem *item, allSlotItems) {
		if ( slotName.startsWith(item->text(QMC2_SLOTCONFIG_COLUMN_SLOT)) ) {
			QStringList parentSlotParts = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT).split(":", QString::SkipEmptyParts);
			if ( parentSlotParts.count() >= slotName.split(":", QString::SkipEmptyParts).count() - 2 ) {
				parentItem = item;
				break;
			}
		}
	}
	if ( parentItem == NULL ) {
		foreach (QTreeWidgetItem *item, allSlotItems) {
			QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
			if ( cb ) {
				if ( slotName.startsWith(item->text(QMC2_SLOTCONFIG_COLUMN_SLOT) + ":" + cb->currentText().split(" ")[0]) ) {
					parentItem = item;
					break;
				}
			}
		}
	}

#ifdef QMC2_DEBUG
	printf("MESSDeviceConfigurator::addNestedSlot(): slotName = %s, parentItem = %p\n", (const char *)slotName.toLocal8Bit(), parentItem);
#endif

	QTreeWidgetItem *slotItem;
	if ( parentItem ) {
		slotItem = new QTreeWidgetItem(parentItem);
		parentItem->setExpanded(true);
	} else
		slotItem = new QTreeWidgetItem(treeWidgetSlotOptions);
	slotItem->setText(QMC2_SLOTCONFIG_COLUMN_SLOT, slotName);
	slotItem->setIcon(QMC2_SLOTCONFIG_COLUMN_SLOT, QIcon(QString::fromUtf8(":/data/img/slot.png")));
	treeWidgetSlotOptions->setItemWidget(slotItem, QMC2_SLOTCONFIG_COLUMN_OPTION, cb);

	nestedSlots << slotName;
}

void MESSDeviceConfigurator::insertChildItems(QTreeWidgetItem *parentItem, QList<QTreeWidgetItem *> &itemList)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		itemList << parentItem->child(i);
		insertChildItems(parentItem->child(i), itemList);
	}
}

void MESSDeviceConfigurator::checkRemovedSlots(QTreeWidgetItem *parentItem)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::checkRemovedSlots(QTreeWidgetItem *parentItem = %1)").arg((qulonglong)parentItem));
#endif

	if ( parentItem == NULL ) {
		for (int i = 0; i < treeWidgetSlotOptions->topLevelItemCount(); i++)
			checkRemovedSlots(treeWidgetSlotOptions->topLevelItem(i));
	} else {
		QList<QTreeWidgetItem *> itemsToDelete;
		for (int i = 0; i < parentItem->childCount(); i++) {
			QTreeWidgetItem *item = parentItem->child(i);
			QString slotName = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT);
			if ( !allSlots.contains(slotName) ) {
#ifdef QMC2_DEBUG
				printf("MESSDeviceConfigurator::checkRemovedSlots(): removedSlot = %s\n", (const char *)slotName.toLocal8Bit());
#endif
				nestedSlotPreselectionMap.remove((QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION));
				nestedSlots.removeAll(slotName);
				nestedSlotOptionMap.remove(slotName);

				itemsToDelete << item;
			}
			checkRemovedSlots(item);
		}
		foreach (QTreeWidgetItem *item, itemsToDelete) {
			parentItem->removeChild(item);
			delete item;
		}
	}
}

QComboBox *MESSDeviceConfigurator::comboBoxByName(QString slotName, QTreeWidgetItem **returnItem)
{
	QList<QTreeWidgetItem *> allSlotItems;
	for (int i = 0; i < treeWidgetSlotOptions->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidgetSlotOptions->topLevelItem(i);
		allSlotItems << item;
		insertChildItems(item, allSlotItems);
	}
	QMap<QString, QTreeWidgetItem *> itemMap;
	foreach (QTreeWidgetItem *item, allSlotItems)
		itemMap[item->text(QMC2_SLOTCONFIG_COLUMN_SLOT)] = item;
	if ( itemMap.contains(slotName) ) {
		QTreeWidgetItem *item = itemMap[slotName];
		if ( returnItem )
			*returnItem = item;
		return (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
	} else {
		if ( returnItem )
			*returnItem = NULL;
		return NULL;
	}
}

bool MESSDeviceConfigurator::refreshDeviceMap()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::refreshDeviceMap(): refreshRunning = %1").arg(refreshRunning ? "true" : "false"));
#endif

	bool wasManualSlotOptionChange = isManualSlotOptionChange;
	isManualSlotOptionChange = false;

	if ( refreshRunning || forceQuit )
		return false;

	refreshRunning = true;

	QString xmlBuffer = getXmlDataWithEnabledSlots(messMachineName);

	if ( xmlBuffer.isEmpty() ) {
		refreshRunning = false;
		return false;
	}

	QList<QListWidgetItem *> itemList = listWidgetDeviceConfigurations->selectedItems();
	QString configName;
	if ( !itemList.isEmpty() )
		if ( itemList[0]->text() != tr("Default configuration") )
			configName = itemList[0]->text();

	treeWidgetDeviceSetup->clear();

	QXmlInputSource xmlInputSource;
	xmlInputSource.setData(xmlBuffer);
	MESSDeviceConfiguratorXmlHandler xmlHandler(treeWidgetDeviceSetup);
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(&xmlHandler);
	if ( !xmlReader.parse(xmlInputSource) ) {
		refreshRunning = false;
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for '%1'").arg(messMachineName));
		tabSlotOptions->setUpdatesEnabled(true);
		return false;
	}
	treeWidgetDeviceSetup->sortItems(QMC2_DEVCONFIG_COLUMN_NAME, Qt::AscendingOrder);

	// update (nested) slots
	QStringList oldNestedSlots = nestedSlots;
	newNestedSlotPreselectionMap.clear();
	foreach (QString newSlot, xmlHandler.newSlots) {
		if ( nestedSlots.contains(newSlot) )
			continue;
		if ( !xmlHandler.newSlotOptions[newSlot].isEmpty() ) {
#ifdef QMC2_DEBUG
			printf("MESSDeviceConfigurator::refreshDeviceMap(): newSlot = %s\n", (const char *)newSlot.toLocal8Bit());
#endif
			QStringList newSlotOptionDescriptions;
			foreach (QString newSlotOption, xmlHandler.newSlotOptions[newSlot]) {
				QString slotOptionDescription = qmc2MachineListItemHash[xmlHandler.newSlotDevices[newSlotOption]]->text(QMC2_MACHINELIST_COLUMN_MACHINE);
#ifdef QMC2_DEBUG
				printf("MESSDeviceConfigurator::refreshDeviceMap():     newSlotOption = %s [%s], default = %s\n",
				       newSlotOption.toLocal8Bit().constData(), slotOptionDescription.toLocal8Bit().constData(), xmlHandler.defaultSlotOptions[newSlot] == newSlotOption ? "yes" : "no");
#endif
				newSlotOptionDescriptions << slotOptionDescription;
			}
			addNestedSlot(newSlot, xmlHandler.newSlotOptions[newSlot], newSlotOptionDescriptions, xmlHandler.defaultSlotOptions[newSlot]);
		}
	}

	bool slotsChanged = oldNestedSlots.count() != nestedSlots.count();
	if ( !slotsChanged ) {
		foreach (QString ns, nestedSlots) {
			if ( !oldNestedSlots.contains(ns) ) {
				slotsChanged = true;
				break;
			}
		}
	}

	if ( !newNestedSlotPreselectionMap.isEmpty() || slotsChanged )
		preselectNestedSlots();

	allSlots = xmlHandler.allSlots;
	slotDeviceNames = xmlHandler.slotDeviceNames;
	checkRemovedSlots();
	updateSlotBiosSelections();
	treeWidgetSlotOptions->sortItems(QMC2_SLOTCONFIG_COLUMN_SLOT, Qt::AscendingOrder);
	tabSlotOptions->setUpdatesEnabled(true);

	// update file-chooser's device instance selector
	comboBoxDeviceInstanceChooser->setUpdatesEnabled(false);

	QStringList instances;
	extensionInstanceMap.clear();

	for (int i = 0; i < treeWidgetDeviceSetup->topLevelItemCount(); i++) {
		QString instance = treeWidgetDeviceSetup->topLevelItem(i)->text(QMC2_DEVCONFIG_COLUMN_NAME);
		if ( !instance.isEmpty() )
			instances << instance;
	}

	comboBoxDeviceInstanceChooser->clear();

	if ( instances.count() > 0 ) {
		std::sort(instances.begin(), instances.end());
		foreach (QString instance, instances) {
			QList<QTreeWidgetItem *> items = treeWidgetDeviceSetup->findItems(instance, Qt::MatchExactly);
			if ( items.count() > 0 ) {
				QStringList extensions = items[0]->text(QMC2_DEVCONFIG_COLUMN_EXT).split("/", QString::SkipEmptyParts);
				foreach (QString extension, extensions) {
					extension = extension.toLower();
					if ( !extensionInstanceMap.contains(extension) )
						extensionInstanceMap[extension] = instance;
				}
				QString devType = items[0]->text(QMC2_DEVCONFIG_COLUMN_TYPE);
				comboBoxDeviceInstanceChooser->addItem(messDevIconHash[devType], instance);
			}
		}
		QString oldFileChooserDeviceInstance = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserDeviceInstance", QString()).toString();
		if ( !oldFileChooserDeviceInstance.isEmpty() ) {
			int index = comboBoxDeviceInstanceChooser->findText(oldFileChooserDeviceInstance, Qt::MatchExactly);
			if ( index >= 0 ) {
				comboBoxDeviceInstanceChooser->blockSignals(true);
				comboBoxDeviceInstanceChooser->setCurrentIndex(index);
				comboBoxDeviceInstanceChooser->blockSignals(false);
			}
		}
		if ( tabWidgetDeviceSetup->currentIndex() == QMC2_DEVSETUP_TAB_FILECHOOSER )
			QTimer::singleShot(0, this, SLOT(setupFileChooser()));
	} else {
		comboBoxDeviceInstanceChooser->insertItem(0, tr("No devices available"));
		tabFileChooser->setEnabled(false);
		treeWidgetDeviceSetup->setEnabled(false);
	}

	comboBoxDeviceInstanceChooser->setUpdatesEnabled(true);

	if ( treeWidgetDeviceSetup->topLevelItemCount() == 1 ) {
		// this avoids delegate-resizing issues when only one item is available
		treeWidgetDeviceSetup->setUpdatesEnabled(false);
		QTreeWidgetItem *dummyItem = new QTreeWidgetItem(treeWidgetDeviceSetup);
		treeWidgetDeviceSetup->openPersistentEditor(dummyItem, QMC2_DEVCONFIG_COLUMN_FILE);
		delete treeWidgetDeviceSetup->takeTopLevelItem(1);
		treeWidgetDeviceSetup->setUpdatesEnabled(true);
		qApp->processEvents();
	}

	if ( slotsChanged && !wasManualSlotOptionChange )
		updateSlots = true;
	else
		updateSlots = isLoading;

	dontIgnoreNameChange = true;
	on_lineEditConfigurationName_textChanged(configName);
	dontIgnoreNameChange = updateSlots = false;

	refreshRunning = false;

	if ( slotsChanged )
		QTimer::singleShot(0, this, SLOT(refreshDeviceMap()));
	else {
		// adjust tab orders for device-maps and slot-options
		QTreeWidgetItem *lastItem = NULL;
		for (int i = 0; i < treeWidgetDeviceSetup->topLevelItemCount(); i++) {
			QTreeWidgetItem *thisItem = treeWidgetDeviceSetup->topLevelItem(i);
			if ( lastItem ) {
				FileEditWidget *w1 = (FileEditWidget *)treeWidgetDeviceSetup->itemWidget(lastItem, QMC2_DEVCONFIG_COLUMN_FILE);
				FileEditWidget *w2 = (FileEditWidget *)treeWidgetDeviceSetup->itemWidget(thisItem, QMC2_DEVCONFIG_COLUMN_FILE);
				treeWidgetDeviceSetup->setTabOrder(w1->lineEditFile, w1->toolButtonBrowse);
				treeWidgetDeviceSetup->setTabOrder(w1->toolButtonBrowse, w1->toolButtonClear);
				treeWidgetDeviceSetup->setTabOrder(w1->toolButtonClear, w2->lineEditFile);
			}
			lastItem = thisItem;
		}
		lastItem = NULL;
		QTreeWidgetItemIterator it(treeWidgetSlotOptions);
		while ( *it ) {
			if ( lastItem ) {
				QWidget *w11 = treeWidgetSlotOptions->itemWidget(lastItem, QMC2_SLOTCONFIG_COLUMN_OPTION);
				QWidget *w12 = treeWidgetSlotOptions->itemWidget(lastItem, QMC2_SLOTCONFIG_COLUMN_BIOS);
				QWidget *w21 = treeWidgetSlotOptions->itemWidget(*it, QMC2_SLOTCONFIG_COLUMN_OPTION);
				treeWidgetDeviceSetup->setTabOrder(w11, w12);
				treeWidgetDeviceSetup->setTabOrder(w12, w21);
			}
			lastItem = *it;
			++it;
		}
	}

	return true;
}

void MESSDeviceConfigurator::updateSlotBiosSelections()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::updateSlotBiosSelections()");
#endif

	for (int i = 0; i < allSlots.count(); i++) {
		if ( forceQuit )
			break;
		QString slotName = allSlots[i];
		QTreeWidgetItem *item;
		QComboBox *cb = comboBoxByName(slotName, &item);
		if ( cb && cb->currentIndex() > 0 ) {
			QString slotOption = cb->currentText().split(" ", QString::SkipEmptyParts)[0];
			QString slotDeviceName = slotDeviceNames[slotOption];
			QString defaultChoice;
			QStringList biosChoices = qmc2MainWindow->getXmlChoices(slotDeviceName, "rom", "bios", &defaultChoice);
			if ( !biosChoices.isEmpty() )
				biosChoices.removeAt(0); // the very first choice is empty (this is *required* at another place)
			QComboBox *cbBIOS = NULL;
			bool isNewCB = false;
			if ( item )
				cbBIOS = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
			if ( item && !cbBIOS ) {
				cbBIOS = new QComboBox(0);
				isNewCB = true;
				cbBIOS->setAutoFillBackground(true);
				cbBIOS->setMaxVisibleItems(20);
			}
			if ( biosChoices.isEmpty() ) {
				if ( cbBIOS ) {
					cbBIOS->clear();
					cbBIOS->setCurrentIndex(-1);
					cbBIOS->addItem(tr("N/A"));
					cbBIOS->setEnabled(false);
				}
			} else {
				QStringList biosEntries;
				int defaultIndex = -1;
				int index = 0;
				foreach (QString biosChoice, biosChoices) {
					if ( biosChoice == defaultChoice ) {
						biosEntries << biosChoice + " / " + tr("default");
						defaultIndex = index;
					} else
						biosEntries << biosChoice;
					index++;
				}
				if ( cbBIOS ) {
					int cIdx = -1;
					QString cTxt = cbBIOS->currentText();
					if ( cTxt != tr("N/A") ) {
						cIdx = cbBIOS->currentIndex();
						cTxt = cbBIOS->currentText();
					}
					cbBIOS->clear();
					cbBIOS->addItems(biosEntries);
					int slotIndex = -1;
					QPair<QStringList, QStringList> biosValuePair;
					if ( !currentConfigName.isEmpty() ) {
						biosValuePair = slotBiosMap[currentConfigName];
						slotIndex = biosValuePair.first.indexOf(slotName);
					} else {
						cIdx = defaultIndex;
						cTxt = cbBIOS->itemText(defaultIndex);
					}
					if ( slotIndex >= 0 ) {
						QString slotBios = biosValuePair.second[slotIndex];
						slotIndex = cbBIOS->findText(QString("^%1(| / %2)$").arg(biosValuePair.second[slotIndex]).arg(tr("default")), Qt::MatchRegExp);
						if ( slotIndex >= 0 )
							cbBIOS->setCurrentIndex(slotIndex);
						else if ( cIdx >= 0 && cbBIOS->itemText(cIdx) == cTxt )
							cbBIOS->setCurrentIndex(cIdx);
						else if ( defaultIndex >= 0 )
							cbBIOS->setCurrentIndex(defaultIndex);
					} else if ( cIdx >= 0 && cbBIOS->itemText(cIdx) == cTxt )
						cbBIOS->setCurrentIndex(cIdx);
					else if ( defaultIndex >= 0 )
						cbBIOS->setCurrentIndex(defaultIndex);
					cbBIOS->setEnabled(true);
				}
			}
			if ( isNewCB )
				treeWidgetSlotOptions->setItemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS, cbBIOS);
		} else if ( item ) {
			QComboBox *cbBIOS = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
			bool isNewCB = false;
			if ( !cbBIOS ) {
				cbBIOS = new QComboBox(0);
				cbBIOS->setAutoFillBackground(true);
				cbBIOS->setMaxVisibleItems(20);
				isNewCB = true;
			}
			cbBIOS->clear();
			cbBIOS->setCurrentIndex(-1);
			cbBIOS->addItem(tr("N/A"));
			cbBIOS->setEnabled(false);
			if ( isNewCB )
				treeWidgetSlotOptions->setItemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS, cbBIOS);
		}
	}
}

void MESSDeviceConfigurator::preselectNestedSlots()
{
	QMapIterator<QComboBox *, int> it(newNestedSlotPreselectionMap);
	while ( it.hasNext() ) {
		it.next();
		QComboBox *cb = it.key();
		int index = it.value();
		cb->setCurrentIndex(index);
		connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOptionChanged(int)));
	}
}

void MESSDeviceConfigurator::preselectSlots()
{
	QMapIterator<QComboBox *, int> it(slotPreselectionMap);
	while ( it.hasNext() ) {
		it.next();
		QComboBox *cb = it.key();
		int index = it.value();
		cb->setCurrentIndex(index);
		connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOptionChanged(int)));
	}
}

bool MESSDeviceConfigurator::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::load()");
#endif

	refreshRunning = true;
	fullyLoaded = false;

	if ( messSystemSlotHash.isEmpty() && messSystemSlotsSupported )
		if ( !readSystemSlots() ) {
			listWidgetDeviceConfigurations->setUpdatesEnabled(true);
			refreshRunning = false;
			return false;
		}

	isLoading = true;

	setEnabled(qmc2UseDefaultEmulator);
	tabSlotOptions->setUpdatesEnabled(false);
	listWidgetDeviceConfigurations->setUpdatesEnabled(false);
	listWidgetDeviceConfigurations->setSortingEnabled(false);
	listWidgetDeviceConfigurations->clear();

	QString xmlBuffer = getXmlData(messMachineName);
  
	QXmlInputSource xmlInputSource;
	xmlInputSource.setData(xmlBuffer);
	MESSDeviceConfiguratorXmlHandler xmlHandler(treeWidgetDeviceSetup);
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(&xmlHandler);
	xmlReader.parse(xmlInputSource);

	comboBoxDeviceInstanceChooser->setUpdatesEnabled(false);
	QStringList instances;
	extensionInstanceMap.clear();

	treeWidgetDeviceSetup->sortItems(QMC2_DEVCONFIG_COLUMN_NAME, Qt::AscendingOrder);

	for (int i = 0; i < treeWidgetDeviceSetup->topLevelItemCount(); i++) {
		QString instance = treeWidgetDeviceSetup->topLevelItem(i)->text(QMC2_DEVCONFIG_COLUMN_NAME);
		if ( !instance.isEmpty() )
			instances << instance;
	}

	if ( instances.count() > 0 ) {
		std::sort(instances.begin(), instances.end());
		foreach (QString instance, instances) {
			QList<QTreeWidgetItem *> items = treeWidgetDeviceSetup->findItems(instance, Qt::MatchExactly);
			if ( items.count() > 0 ) {
				QStringList extensions = items[0]->text(QMC2_DEVCONFIG_COLUMN_EXT).split("/", QString::SkipEmptyParts);
				foreach (QString extension, extensions) {
					extension = extension.toLower();
					if ( !extensionInstanceMap.contains(extension) )
						extensionInstanceMap[extension] = instance;
				}
				QString devType = items[0]->text(QMC2_DEVCONFIG_COLUMN_TYPE);
				comboBoxDeviceInstanceChooser->addItem(messDevIconHash[devType], instance);
			}
		}
		QString oldFileChooserDeviceInstance = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserDeviceInstance", QString()).toString();
		if ( !oldFileChooserDeviceInstance.isEmpty() ) {
			int index = comboBoxDeviceInstanceChooser->findText(oldFileChooserDeviceInstance, Qt::MatchExactly);
			if ( index >= 0 ) {
				comboBoxDeviceInstanceChooser->blockSignals(true);
				comboBoxDeviceInstanceChooser->setCurrentIndex(index);
				comboBoxDeviceInstanceChooser->blockSignals(false);
			}
		}
		if ( tabWidgetDeviceSetup->currentIndex() == QMC2_DEVSETUP_TAB_FILECHOOSER )
			QTimer::singleShot(0, this, SLOT(setupFileChooser()));
	} else {
		comboBoxDeviceInstanceChooser->insertItem(0, tr("No devices available"));
		tabFileChooser->setEnabled(false);
		treeWidgetDeviceSetup->setEnabled(false);
	}

	comboBoxDeviceInstanceChooser->setUpdatesEnabled(true);

	if ( treeWidgetDeviceSetup->topLevelItemCount() == 1 ) {
		// this avoids delegate-resizing issues when only one item is available
		treeWidgetDeviceSetup->setUpdatesEnabled(false);
		QTreeWidgetItem *dummyItem = new QTreeWidgetItem(treeWidgetDeviceSetup);
		treeWidgetDeviceSetup->openPersistentEditor(dummyItem, QMC2_DEVCONFIG_COLUMN_FILE);
		delete treeWidgetDeviceSetup->takeTopLevelItem(1);
		treeWidgetDeviceSetup->setUpdatesEnabled(true);
		qApp->processEvents();
	}

	QHashIterator<QString, QStringList> it(messSystemSlotHash[messMachineName]);
	slotPreselectionMap.clear();
	nestedSlotPreselectionMap.clear();
	while ( it.hasNext() ) {
		it.next();
		QString slotName = it.key();
		if ( slotName == "QMC2_UNUSED_SLOTS" )
			continue;
		bool isNestedSlot = !messSystemSlotHash[messMachineName].contains(slotName);
		QStringList slotOptions;
		QStringList slotOptionsShort;
		foreach (QString s, it.value()) {
			slotOptions << QString("%1 - %2").arg(s).arg(messSlotNameHash[s]);
			slotOptionsShort << s;
		}
		slotOptions.sort();
		slotOptionsShort.sort();
		QComboBox *cb = new QComboBox(0);
		cb->setAutoFillBackground(true);
		cb->setMaxVisibleItems(20);
		QString defaultSlotOption = xmlHandler.defaultSlotOptions[slotName];
		if ( defaultSlotOption.isEmpty() ) {
			cb->insertItem(0, tr("not used") + " / " + tr("default"));
			if ( slotOptions.count() > 0 ) {
				cb->insertSeparator(1);
				cb->insertItems(2, slotOptions);
			}
			if ( isNestedSlot )
				nestedSlotPreselectionMap[cb] = 0;	
			else
				slotPreselectionMap[cb] = 0;
		} else {
			cb->insertItem(0, tr("not used"));
			if ( slotOptions.count() > 0 ) {
				cb->insertSeparator(1);
				cb->insertItems(2, slotOptions);
				int defaultIndex = slotOptionsShort.indexOf(defaultSlotOption);
				if ( isNestedSlot )
					nestedSlotPreselectionMap[cb] = defaultIndex + 2;
				else
					slotPreselectionMap[cb] = defaultIndex + 2;
				cb->setItemText(defaultIndex + 2, slotOptions[defaultIndex] + " / " + tr("default"));
			} else {
				if ( isNestedSlot )
					nestedSlotPreselectionMap[cb] = 0;
				else
					slotPreselectionMap[cb] = 0;
			}
		}

		QTreeWidgetItem *slotItem = new QTreeWidgetItem(treeWidgetSlotOptions);
		slotItem->setText(QMC2_SLOTCONFIG_COLUMN_SLOT, slotName);
		slotItem->setIcon(QMC2_SLOTCONFIG_COLUMN_SLOT, QIcon(QString::fromUtf8(":/data/img/slot.png")));
		treeWidgetSlotOptions->setItemWidget(slotItem, QMC2_SLOTCONFIG_COLUMN_OPTION, cb);
	}
	treeWidgetSlotOptions->sortItems(QMC2_SLOTCONFIG_COLUMN_SLOT, Qt::AscendingOrder);
	QTimer::singleShot(0, this, SLOT(preselectSlots()));

	configurationMap.clear();
	slotMap.clear();
	slotBiosMap.clear();

	QString group = QString("MESS/Configuration/Devices/%1").arg(messMachineName);
	currentConfigName = qmc2Config->value(group + "/SelectedConfiguration").toString();
	qmc2Config->beginGroup(group);
	QStringList configurationList = qmc2Config->childGroups();
	qmc2Config->endGroup();

	QListWidgetItem *selectedItem = NULL;
	foreach (QString configName, configurationList) {
		if ( configName == tr("Default configuration") )
			continue;
		configurationMap[configName].first = qmc2Config->value(group + QString("/%1/Instances").arg(configName)).toStringList();
		configurationMap[configName].second = qmc2Config->value(group + QString("/%1/Files").arg(configName)).toStringList();
		slotMap[configName].first = qmc2Config->value(group + QString("/%1/Slots").arg(configName), QStringList()).toStringList();
		slotMap[configName].second = qmc2Config->value(group + QString("/%1/SlotOptions").arg(configName), QStringList()).toStringList();
		slotBiosMap[configName].first << qmc2Config->value(group + QString("/%1/Slots").arg(configName), QStringList()).toStringList();
		slotBiosMap[configName].second = qmc2Config->value(group + QString("/%1/SlotBIOSs").arg(configName), QStringList()).toStringList();
		QListWidgetItem *item = new QListWidgetItem(configName, listWidgetDeviceConfigurations);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		if ( currentConfigName == configName ) selectedItem = item;
	}
	listWidgetDeviceConfigurations->setSortingEnabled(true);
	listWidgetDeviceConfigurations->sortItems(Qt::AscendingOrder);
	listWidgetDeviceConfigurations->setUpdatesEnabled(true);

	qmc2FileEditStartPath = qmc2Config->value(group + "/DefaultDeviceDirectory").toString();

	// use the 'general software folder' as fall-back, if applicable
	if ( qmc2FileEditStartPath.isEmpty() ) {
		qmc2FileEditStartPath = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(qmc2FileEditStartPath + "/" + messMachineName);
		if ( machineSoftwareFolder.exists() )
			qmc2FileEditStartPath = machineSoftwareFolder.canonicalPath();
	}

	updateSlots = dontIgnoreNameChange = true;
	refreshRunning = false;
	QListWidgetItem *noDeviceItem = new QListWidgetItem(tr("Default configuration"), listWidgetDeviceConfigurations);
	if ( selectedItem == NULL )
		listWidgetDeviceConfigurations->setCurrentItem(noDeviceItem);
	else
		listWidgetDeviceConfigurations->setCurrentItem(selectedItem);
	QTimer::singleShot(0, this, SLOT(refreshDeviceMap()));
	updateSlots = dontIgnoreNameChange = isLoading = false;

	if ( treeWidgetSlotOptions->topLevelItemCount() == 0 )
		treeWidgetSlotOptions->setEnabled(false);

	fullyLoaded = true;

	tabSlotOptions->setUpdatesEnabled(true);

	return true;
}

bool MESSDeviceConfigurator::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::save()");
#endif

	if ( !fullyLoaded )
		return false;

	QString group = QString("MESS/Configuration/Devices/%1").arg(messMachineName);
	QString devDir = qmc2Config->value(QString("%1/DefaultDeviceDirectory").arg(group), "").toString();

	qmc2Config->remove(group);

	if ( configurationMap.count() > 0 ) {
		foreach (QString configName, configurationMap.keys()) {
			QPair<QStringList, QStringList> config = configurationMap[configName];
			qmc2Config->setValue(group + QString("/%1/Instances").arg(configName), config.first);
			qmc2Config->setValue(group + QString("/%1/Files").arg(configName), config.second);
			QPair<QStringList, QStringList> slotConfig = slotMap[configName];
			if ( slotConfig.first.isEmpty() ) {
				qmc2Config->remove(group + QString("/%1/Slots").arg(configName));
				qmc2Config->remove(group + QString("/%1/SlotOptions").arg(configName));
			} else {
				qmc2Config->setValue(group + QString("/%1/Slots").arg(configName), slotConfig.first);
				qmc2Config->setValue(group + QString("/%1/SlotOptions").arg(configName), slotConfig.second);
			}
			QPair<QStringList, QStringList> slotBIOSs = slotBiosMap[configName];
			if ( slotBIOSs.first.isEmpty() )
				qmc2Config->remove(group + QString("/%1/SlotBIOSs").arg(configName));
			else
				qmc2Config->setValue(group + QString("/%1/SlotBIOSs").arg(configName), slotBIOSs.second);
		}
	}

	if ( !devDir.isEmpty() )
		qmc2Config->setValue(group + "/DefaultDeviceDirectory", devDir);

	QListWidgetItem *curItem = listWidgetDeviceConfigurations->currentItem();
	if ( curItem != NULL ) {
		if ( curItem->text() == tr("Default configuration") )
			qmc2Config->remove(group + "/SelectedConfiguration");
		else
			qmc2Config->setValue(group + "/SelectedConfiguration", curItem->text());
	}

	return true;
}

void MESSDeviceConfigurator::on_toolButtonNewConfiguration_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_toolButtonNewConfiguration_clicked()");
#endif

	dontIgnoreNameChange = updateSlots = true;
	lineEditConfigurationName->setText(tr("Default configuration"));
	dontIgnoreNameChange = updateSlots = true;
	on_lineEditConfigurationName_textChanged(tr("Default configuration"));
	dontIgnoreNameChange = updateSlots = false;
	lineEditConfigurationName->blockSignals(true);
	lineEditConfigurationName->clear();
	lineEditConfigurationName->blockSignals(false);
	lineEditConfigurationName->setFocus();
}

void MESSDeviceConfigurator::on_toolButtonCloneConfiguration_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_toolButtonCloneConfiguration_clicked()");
#endif

	// create a clone of an existing device configuration
	QString sourceName = lineEditConfigurationName->text();
	int copies = 1;
	QString targetName = tr("%1. copy of ").arg(copies) + sourceName;
	while ( configurationMap.contains(targetName) )
		targetName = tr("%1. copy of ").arg(++copies) + sourceName;

	configurationMap.insert(targetName, configurationMap[sourceName]);
	slotMap.insert(targetName, slotMap[sourceName]);
	slotBiosMap.insert(targetName, slotBiosMap[sourceName]);
	listWidgetDeviceConfigurations->setSortingEnabled(false);
	int row = listWidgetDeviceConfigurations->count();
	listWidgetDeviceConfigurations->insertItem(row, targetName);
	listWidgetDeviceConfigurations->item(row)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	listWidgetDeviceConfigurations->setSortingEnabled(true);
	listWidgetDeviceConfigurations->sortItems(Qt::AscendingOrder);

	dontIgnoreNameChange = true;
	lineEditConfigurationName->setText(targetName);
}

void MESSDeviceConfigurator::on_toolButtonSaveConfiguration_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_toolButtonSaveConfiguration_clicked()");
#endif

	QString cfgName = lineEditConfigurationName->text();

	if ( cfgName.isEmpty() )
		return;

	QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(cfgName, Qt::MatchExactly);
	if ( matchedItemList.count() > 0 ) {
		// save device configuration
		QStringList instances, files;
		for (int i = 0; i < treeWidgetDeviceSetup->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidgetDeviceSetup->topLevelItem(i);
			QString fileName = item->data(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole).toString();
			if ( !fileName.isEmpty() ) {
				instances << item->data(QMC2_DEVCONFIG_COLUMN_NAME, Qt::EditRole).toString();
				files << fileName;
			}
		}
		configurationMap[cfgName].first = instances;
		configurationMap[cfgName].second = files;

		// save slot setup
		QList<QTreeWidgetItem *> allSlotItems;
		for (int i = 0; i < treeWidgetSlotOptions->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidgetSlotOptions->topLevelItem(i);
			allSlotItems << item;
			insertChildItems(item, allSlotItems);
		}
		QStringList slotNames, slotOptions, slotBIOSs;
		foreach (QTreeWidgetItem *item, allSlotItems) {
			QString slotName = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT);
			if ( !slotName.isEmpty() ) {
				QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
				if ( cb ) {
					int defaultIndex = -1;
					if ( slotPreselectionMap.contains(cb) )
						defaultIndex = slotPreselectionMap[cb];
					else if ( nestedSlotPreselectionMap.contains(cb) )
						defaultIndex = nestedSlotPreselectionMap[cb];
					QComboBox *cbBIOS = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
					if ( cb->currentIndex() > 0 && defaultIndex == 0 ) {
						slotNames << slotName;
						slotOptions << cb->currentText().split(" ")[0];
						if ( cbBIOS ) {
							QString biosChoice = cbBIOS->currentText().split(" ", QString::SkipEmptyParts)[0];
							if ( biosChoice == tr("N/A") )
								biosChoice.clear();
							slotBIOSs << biosChoice;
						} else
							slotBIOSs << QString();
					} else if ( cb->currentIndex() == 0 && defaultIndex > 0 ) {
						slotNames << slotName;
						slotOptions << "\"\"";
						slotBIOSs << QString();
					} else if ( cb->currentIndex() > 0 && defaultIndex > 0 && cb->currentIndex() != defaultIndex ) {
						slotNames << slotName;
						slotOptions << cb->currentText().split(" ")[0];
						if ( cbBIOS ) {
							QString biosChoice = cbBIOS->currentText().split(" ", QString::SkipEmptyParts)[0];
							if ( biosChoice == tr("N/A") )
								biosChoice.clear();
							slotBIOSs << biosChoice;
						} else
							slotBIOSs << QString();
					} else {
						slotNames << slotName;
						if ( cbBIOS ) {
							bool isDefaultBiosChoice = cbBIOS->currentText().endsWith(" / " + tr("default"));
							QString biosChoice = cbBIOS->currentText().split(" ", QString::SkipEmptyParts)[0];
							if ( biosChoice == tr("N/A") )
								biosChoice.clear();
							slotBIOSs << biosChoice;
							if ( !biosChoice.isEmpty() && !isDefaultBiosChoice )
								slotOptions << cb->currentText().split(" ")[0];
							else
								slotOptions << QString();
						} else  {
							slotOptions << QString();
							slotBIOSs << QString();
						}
					}
				}
			}
		}
		slotMap[cfgName].first = slotNames;
		slotMap[cfgName].second = slotOptions;
		slotBiosMap[cfgName].first = slotNames;
		slotBiosMap[cfgName].second = slotBIOSs;
	} else {
		// add new device configuration
		listWidgetDeviceConfigurations->setSortingEnabled(false);
		int row = listWidgetDeviceConfigurations->count();
		listWidgetDeviceConfigurations->insertItem(row, cfgName);
		listWidgetDeviceConfigurations->item(row)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		listWidgetDeviceConfigurations->setSortingEnabled(true);
		listWidgetDeviceConfigurations->sortItems(Qt::AscendingOrder);

		dontIgnoreNameChange = true;
		on_toolButtonSaveConfiguration_clicked();
	}

	on_lineEditConfigurationName_textChanged(lineEditConfigurationName->text());
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	item->setSelected(true);
	dontIgnoreNameChange = updateSlots = true;
	on_lineEditConfigurationName_textChanged(item->text());
	dontIgnoreNameChange = updateSlots = false;
	lineEditConfigurationName->blockSignals(true);
	lineEditConfigurationName->setText(item->text());
	lineEditConfigurationName->blockSignals(false);
}

void MESSDeviceConfigurator::on_toolButtonRemoveConfiguration_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_toolButtonRemoveConfiguration_clicked()");
#endif

	QString cfgName = lineEditConfigurationName->text();

	QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(cfgName, Qt::MatchExactly);
	if ( matchedItemList.count() > 0 ) {
		configurationMap.remove(cfgName);
		slotMap.remove(cfgName);
		slotBiosMap.remove(cfgName);
		int row = listWidgetDeviceConfigurations->row(matchedItemList[0]);
		QListWidgetItem *prevItem = NULL;
		if ( row > 0 )
			prevItem = listWidgetDeviceConfigurations->item(row - 1);
		QListWidgetItem *item = listWidgetDeviceConfigurations->takeItem(row);
		delete item;
		if ( prevItem )
			listWidgetDeviceConfigurations->setCurrentItem(prevItem);
	}
}

void MESSDeviceConfigurator::actionRenameConfiguration_activated()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::actionRenameConfiguration_activated()");
#endif

	configurationRenameItem = NULL;
	QList<QListWidgetItem *> sl = listWidgetDeviceConfigurations->selectedItems();
	if ( sl.count() > 0 ) {
		configurationRenameItem = sl[0];
		oldConfigurationName = configurationRenameItem->text();
		listWidgetDeviceConfigurations->editItem(configurationRenameItem);
	}
}

void MESSDeviceConfigurator::configurationItemChanged(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::configurationItemChanged(QListWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( item && configurationRenameItem == item && item->text() != oldConfigurationName ) {
		QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(item->text(), Qt::MatchExactly);
		if ( matchedItemList.count() > 1 ) {
			item->setText(oldConfigurationName);
		} else {
			configurationMap.insert(item->text(), configurationMap[oldConfigurationName]);
			configurationMap.remove(oldConfigurationName);
			slotMap.insert(item->text(), slotMap[oldConfigurationName]);
			slotMap.remove(oldConfigurationName);
			slotBiosMap.insert(item->text(), slotBiosMap[oldConfigurationName]);
			slotBiosMap.remove(oldConfigurationName);
		}
		lineEditConfigurationName->setText(item->text());
	}
	configurationRenameItem = NULL;
	oldConfigurationName.clear();
}

void MESSDeviceConfigurator::actionRemoveConfiguration_activated()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::actionRemoveConfiguration_activated()");
#endif

	QList<QListWidgetItem *> sl = listWidgetDeviceConfigurations->selectedItems();

	if ( sl.count() > 0 ) {
		QListWidgetItem *item = sl[0];
		configurationMap.remove(item->text());
		slotMap.remove(item->text());
		slotBiosMap.remove(item->text());
		int row = listWidgetDeviceConfigurations->row(item);
		QListWidgetItem *prevItem = NULL;
		if ( row > 0 )
			prevItem = listWidgetDeviceConfigurations->item(row - 1);
		item = listWidgetDeviceConfigurations->takeItem(row);
		delete item;
		if ( prevItem )
			listWidgetDeviceConfigurations->setCurrentItem(prevItem);
	}
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemActivated(QListWidgetItem *item = %1)").arg((qulonglong) item));
#endif

	switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		case QMC2_LAUNCH_MODE_EMBEDDED:
			QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_triggered()));
			break;
#endif
		case QMC2_LAUNCH_MODE_INDEPENDENT:
		default:
			QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_triggered()));
			break;
	}
}

void MESSDeviceConfigurator::on_lineEditConfigurationName_textChanged(const QString &text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_lineEditConfigurationName_textChanged(const QString &text = %1)").arg(text));
#endif

	if ( qmc2CriticalSection || forceQuit )
		return;

	qmc2CriticalSection = true;

	toolButtonSaveConfiguration->setEnabled(false);
	if ( text == tr("Default configuration") ) {
		toolButtonCloneConfiguration->setEnabled(false);
		toolButtonRemoveConfiguration->setEnabled(false);
		currentConfigName.clear();
	} else if ( !text.isEmpty() ) {
		QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(text, Qt::MatchExactly);
		if ( matchedItemList.count() > 0 ) {
			toolButtonRemoveConfiguration->setEnabled(true);
			toolButtonCloneConfiguration->setEnabled(true);
			currentConfigName = text;
		} else {
			toolButtonRemoveConfiguration->setEnabled(false);
			toolButtonCloneConfiguration->setEnabled(false);
			currentConfigName.clear();
		}
		toolButtonSaveConfiguration->setEnabled(true);
	} else {
		toolButtonCloneConfiguration->setEnabled(false);
		toolButtonRemoveConfiguration->setEnabled(false);
		currentConfigName.clear();
	}

	if ( dontIgnoreNameChange ) {
		QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(text, Qt::MatchExactly);
		if ( !matchedItemList.isEmpty() ) {
			matchedItemList[0]->setSelected(true);
			listWidgetDeviceConfigurations->setCurrentItem(matchedItemList[0]);
			listWidgetDeviceConfigurations->scrollToItem(matchedItemList[0]);
			QString configName = matchedItemList[0]->text();
			if ( updateSlots ) {
				QList<QTreeWidgetItem *> itemList;
				QMap<QString, QTreeWidgetItem *> itemMap;
				for (int i = 0; i < treeWidgetSlotOptions->topLevelItemCount(); i++) {
					QTreeWidgetItem *item = treeWidgetSlotOptions->topLevelItem(i);
					itemList << item;
					insertChildItems(item, itemList);
				}
				foreach (QTreeWidgetItem *item, itemList) {
					QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
					if ( cb ) {
						int index = -1;
						if ( slotPreselectionMap.contains(cb) )
							index = slotPreselectionMap[cb];
						else if ( nestedSlotPreselectionMap.contains(cb) )
							index = nestedSlotPreselectionMap[cb];
						if ( index >= 0 ) {
							cb->blockSignals(true);
							cb->setCurrentIndex(index);
							cb->blockSignals(false);
						}
					}
					itemMap[item->text(QMC2_SLOTCONFIG_COLUMN_SLOT)] = item;
				}
				if ( slotMap.contains(configName) ) {
					QPair<QStringList, QStringList> valuePair = slotMap[configName];
					QPair<QStringList, QStringList> biosValuePair = slotBiosMap[configName];
					for (int i = 0; i < valuePair.first.count(); i++) {
						if ( itemMap.contains(valuePair.first[i]) ) {
							QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(itemMap[valuePair.first[i]], QMC2_SLOTCONFIG_COLUMN_OPTION);
							if ( cb ) {
								int index = -1;
								bool isNestedSlot = !messSystemSlotHash[messMachineName].contains(valuePair.first[i]);
								if ( valuePair.second[i] != "\"\"" ) {
									if ( isNestedSlot )
										index = cb->findText(QString("%1 - %2").arg(valuePair.second[i]).arg(nestedSlotOptionMap[valuePair.first[i]][valuePair.second[i]]));
									else
										index = cb->findText(QString("%1 - %2").arg(valuePair.second[i]).arg(messSlotNameHash[valuePair.second[i]]));
								} else
									index = 0;

								if ( index >= 0 ) {
									cb->blockSignals(true);
									cb->setCurrentIndex(index);
									cb->blockSignals(false);
								}
							}
							QComboBox *cbBIOS = (QComboBox *)treeWidgetSlotOptions->itemWidget(itemMap[valuePair.first[i]], QMC2_SLOTCONFIG_COLUMN_BIOS);
							if ( cbBIOS ) {
								int index = -1;
								if ( !biosValuePair.second.isEmpty() && !biosValuePair.second[i].isEmpty() )
									index = cbBIOS->findText(QString("^%1(| / %2)$").arg(biosValuePair.second[i]).arg(tr("default")), Qt::MatchRegExp);
								if ( index >= 0 ) {
									cbBIOS->blockSignals(true);
									cbBIOS->setCurrentIndex(index);
									cbBIOS->blockSignals(false);
								}
							}
						}
					}
				}
				refreshDeviceMap();
			}
			if ( configurationMap.contains(configName) ) {
				QPair<QStringList, QStringList> valuePair = configurationMap[configName];
				for (int i = 0; i < valuePair.first.count(); i++) {
					QList<QTreeWidgetItem *> itemList = treeWidgetDeviceSetup->findItems(valuePair.first[i], Qt::MatchExactly);
					if ( itemList.count() > 0 ) {
						QTreeWidgetItem *item = itemList[0];
						QString data = valuePair.second[i];
						item->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, data);
						FileEditWidget *few = (FileEditWidget*)treeWidgetDeviceSetup->itemWidget(item, QMC2_DEVCONFIG_COLUMN_FILE);
						if ( few )
							few->lineEditFile->setText(data);
					}
				}
			}
		} else {
			QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(tr("Default configuration"), Qt::MatchExactly);
			if ( !matchedItemList.isEmpty() )
				listWidgetDeviceConfigurations->setCurrentItem(matchedItemList[0]);
			else
				listWidgetDeviceConfigurations->clearSelection();
			for (int i = 0; i < treeWidgetDeviceSetup->topLevelItemCount(); i++) {
				QTreeWidgetItem *item = treeWidgetDeviceSetup->topLevelItem(i);
				item->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, QString());
				FileEditWidget *few = (FileEditWidget*)treeWidgetDeviceSetup->itemWidget(item, QMC2_DEVCONFIG_COLUMN_FILE);
				if ( few )
					few->lineEditFile->clear();
			}
			toolButtonRemoveConfiguration->setEnabled(false);
			toolButtonCloneConfiguration->setEnabled(false);
		}
	}
	dontIgnoreNameChange = false;
	qmc2CriticalSection = false;
}

void MESSDeviceConfigurator::editorDataChanged(const QString &text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::editorDataChanged(const QString &text = %1)").arg(text));
#endif

	if ( lineEditConfigurationName->text().isEmpty() && !text.isEmpty() ) {
		int copies = 0;
		QString sourceName = text;
		QFileInfo fi(sourceName);
		sourceName = fi.completeBaseName();
		QString targetName = sourceName;
		while ( configurationMap.contains(targetName) )
			targetName = tr("%1. variant of ").arg(++copies) + sourceName;
		lineEditConfigurationName->setText(targetName);
	}
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_currentTextChanged(const QString &text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_currentTextChanged(const QString &text = %1)").arg(text));
#endif

	dontIgnoreNameChange = updateSlots = true;
	lineEditConfigurationName->setText(text);
	dontIgnoreNameChange = updateSlots = false;
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &point)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &point = [%1, %2])").arg(point.x()).arg(point.y()));
#endif

	QListWidgetItem *item = listWidgetDeviceConfigurations->itemAt(point);
	if ( item ) {
		if ( item->text() == tr("Default configuration") ) {
			actionRenameConfiguration->setVisible(false);
			actionRemoveConfiguration->setVisible(false);
		} else {
			actionRenameConfiguration->setVisible(true);
			actionRemoveConfiguration->setVisible(true);
		}
		listWidgetDeviceConfigurations->setCurrentItem(item);
		listWidgetDeviceConfigurations->setItemSelected(item, true);
		deviceConfigurationListMenu->move(listWidgetDeviceConfigurations->viewport()->mapToGlobal(point));
		deviceConfigurationListMenu->show();
	}
}

void MESSDeviceConfigurator::on_treeWidgetDeviceSetup_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_treeWidgetDeviceSetup_customContextMenuRequested(const QPoint &p = [%1, %2])").arg(p.x()).arg(p.y()));
#endif

	if ( treeWidgetDeviceSetup->itemAt(p) ) {
		deviceContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetDeviceSetup->viewport()->mapToGlobal(p), deviceContextMenu));
		deviceContextMenu->show();
	}
}

void MESSDeviceConfigurator::on_treeWidgetSlotOptions_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_treeWidgetSlotOptions_customContextMenuRequested(const QPoint &p = [%1, %2])").arg(p.x()).arg(p.y()));
#endif

	if ( treeWidgetSlotOptions->itemAt(p) ) {
		slotContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSlotOptions->viewport()->mapToGlobal(p), slotContextMenu));
		slotContextMenu->show();
	}
}

void MESSDeviceConfigurator::actionSelectFile_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::actionSelectFile_triggered()");
#endif

	QTreeWidgetItem *item = treeWidgetDeviceSetup->currentItem();
	if ( item ) {
		int row = treeWidgetDeviceSetup->indexOfTopLevelItem(item);
		if ( row >= 0 ) {
			FileEditWidget *few = messFileEditWidgetList[row];
			if ( few )
				QTimer::singleShot(0, few, SLOT(on_toolButtonBrowse_clicked()));
		}
	}
}

void MESSDeviceConfigurator::actionSelectDefaultDeviceDirectory_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::actionSelectDefaultDeviceDirectory_triggered()");
#endif

	QString group = QString("MESS/Configuration/Devices/%1").arg(messMachineName);
	QString path = qmc2Config->value(group + "/DefaultDeviceDirectory", "").toString();

	if ( path.isEmpty() ) {
		path = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(path + "/" + messMachineName);
		if ( machineSoftwareFolder.exists() )
			path = machineSoftwareFolder.canonicalPath();
	}

	qmc2Config->beginGroup(group);

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose default device directory for '%1'").arg(messMachineName), path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isEmpty() )
		qmc2Config->setValue("DefaultDeviceDirectory", s);
	qmc2FileEditStartPath = qmc2Config->value("DefaultDeviceDirectory").toString();

	qmc2Config->endGroup();

	if ( qmc2FileEditStartPath.isEmpty() ) {
		qmc2FileEditStartPath = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(qmc2FileEditStartPath + "/" + messMachineName);
		if ( machineSoftwareFolder.exists() )
			qmc2FileEditStartPath = machineSoftwareFolder.canonicalPath();
	}
}

void MESSDeviceConfigurator::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e )
		e->accept();
}

void MESSDeviceConfigurator::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	save();

	if ( e )
		e->accept();
}

void MESSDeviceConfigurator::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e )
		e->accept();
}

void MESSDeviceConfigurator::on_tabWidgetDeviceSetup_currentChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_tabWidgetDeviceSetup_currentChanged(int index = %1)").arg(index));
#endif

	switch ( index ) {
		case QMC2_DEVSETUP_TAB_DEVMAPPINGS:
		case QMC2_DEVSETUP_TAB_SLOTCONFIG:
			labelActiveDeviceConfiguration->show();
			lineEditConfigurationName->show();
			toolButtonNewConfiguration->show();
			toolButtonCloneConfiguration->show();
			toolButtonRemoveConfiguration->show();
			toolButtonSaveConfiguration->show();
			vSplitter->widget(1)->show();
			break;
		case QMC2_DEVSETUP_TAB_FILECHOOSER:
			labelActiveDeviceConfiguration->hide();
			lineEditConfigurationName->hide();
			toolButtonNewConfiguration->hide();
			toolButtonCloneConfiguration->hide();
			toolButtonRemoveConfiguration->hide();
			toolButtonSaveConfiguration->hide();
			vSplitter->widget(1)->hide();
			if ( !fileChooserSetup )
				if ( comboBoxDeviceInstanceChooser->count() > 0 && comboBoxDeviceInstanceChooser->currentText() != tr("No devices available") )
	  				QTimer::singleShot(0, this, SLOT(setupFileChooser()));
			break;
		default:
			break;
	}
}

void MESSDeviceConfigurator::setupFileChooser()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::setupFileChooser()");
#endif

	if ( fileChooserSetup ) {
		tabFileChooser->setEnabled(true);
		return;
	}

	fileChooserSetup = true;

	toolButtonChooserPlay->setEnabled(false);
	toolButtonChooserPlayEmbedded->setEnabled(false);
	toolButtonChooserSaveConfiguration->setEnabled(false);

	QString group = QString("MESS/Configuration/Devices/%1").arg(messMachineName);
	QString path = qmc2Config->value(group + "/DefaultDeviceDirectory", "").toString();

	if ( path.isEmpty() ) {
		path = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(path + "/" + messMachineName);
		if ( machineSoftwareFolder.exists() )
			path = machineSoftwareFolder.canonicalPath();
	}

	if ( path.isEmpty() )
		path = QDir::rootPath();

	treeViewDirChooser->setUpdatesEnabled(false);
	dirModel = new DirectoryModel(this);
	dirModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Drives | QDir::CaseSensitive);
#if defined(QMC2_OS_WIN)
	dirModel->setRootPath(dirModel->myComputer().toString());
#else
	dirModel->setRootPath("/");
#endif
	treeViewDirChooser->setModel(dirModel);
	treeViewDirChooser->setCurrentIndex(dirModel->index(path));
	for (int i = treeViewDirChooser->header()->count(); i > 0; i--) treeViewDirChooser->setColumnHidden(i, true);
	treeViewDirChooser->setSortingEnabled(true);

#if QT_VERSION < 0x050000
	treeViewDirChooser->header()->setMovable(false);
	treeViewDirChooser->header()->setResizeMode(QHeaderView::Stretch);
#else
	treeViewDirChooser->header()->setSectionsMovable(false);
	treeViewDirChooser->header()->setSectionResizeMode(QHeaderView::Stretch);
#endif
	treeViewDirChooser->header()->setStretchLastSection(true);
  	treeViewDirChooser->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DirChooserHeaderState").toByteArray());
	treeViewDirChooser->sortByColumn(0, treeViewDirChooser->header()->sortIndicatorOrder());
	treeViewDirChooser->updateGeometry();

	fileModelRowInsertionCounter = 0;
	lcdNumberFileCounter->display(0);
	lcdNumberFileCounter->setSegmentStyle(QLCDNumber::Flat);
	lcdNumberFileCounter->update();
	fileModel = new FileSystemModel(this);
	fileModel->setIncludeFolders(includeFolders);
	fileModel->setFoldersFirst(foldersFirst);
	fileModel->setCurrentPath(path, false);
	connect(fileModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(fileModel_rowsInserted(const QModelIndex &, int, int)));
	connect(fileModel, SIGNAL(finished()), this, SLOT(fileModel_finished()));
	treeViewFileChooser->setUpdatesEnabled(false);
	treeViewFileChooser->setModel(fileModel);
  	treeViewFileChooser->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserHeaderState").toByteArray());
	connect(treeViewDirChooser->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeViewDirChooser_headerClicked(int)));
	connect(treeViewFileChooser->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeViewFileChooser_headerClicked(int)));
	connect(treeViewFileChooser->header(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(treeViewFileChooser_sectionMoved(int, int, int)));
	connect(treeViewFileChooser->header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(treeViewFileChooser_sectionResized(int, int, int)));
	on_toolButtonChooserFilter_toggled(toolButtonChooserFilter->isChecked());

	connect(treeViewDirChooser->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(treeViewDirChooser_selectionChanged(const QItemSelection &, const QItemSelection &)));
	connect(treeViewFileChooser->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(treeViewFileChooser_selectionChanged(const QItemSelection &, const QItemSelection &)));
	connect(toolButtonChooserPlay, SIGNAL(clicked()), qmc2MainWindow, SLOT(on_actionPlay_triggered()));
	connect(toolButtonChooserPlayEmbedded, SIGNAL(clicked()), qmc2MainWindow, SLOT(on_actionPlayEmbedded_triggered()));

	QTimer::singleShot(QMC2_DIRCHOOSER_INIT_WAIT, this, SLOT(dirChooserDelayedInit()));
}

void MESSDeviceConfigurator::dirChooserDelayedInit()
{
	treeViewDirChooser->scrollTo(treeViewDirChooser->currentIndex(), qmc2CursorPositioningMode);
	treeViewFileChooser->setUpdatesEnabled(true);
	treeViewDirChooser->setUpdatesEnabled(true);
}

void MESSDeviceConfigurator::treeViewDirChooser_selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewDirChooser_selectionChanged(constQItemSelection &selected = ..., const QItemSelection &deselected = ...)");
#endif

	if ( !selected.isEmpty() ) {
		toolButtonChooserPlay->setEnabled(false);
		toolButtonChooserPlayEmbedded->setEnabled(false);
		toolButtonChooserSaveConfiguration->setEnabled(false);
		QString path = dirModel->fileInfo(selected.indexes()[0]).absoluteFilePath();
		fileModel->setCurrentPath(path, false);
		on_toolButtonChooserFilter_toggled(toolButtonChooserFilter->isChecked());
	}
}

void MESSDeviceConfigurator::treeViewFileChooser_selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_selectionChanged(constQItemSelection &selected = ..., const QItemSelection &deselected = ...)");
#endif

	if ( selected.indexes().count() > 0 ) {
		toolButtonChooserPlay->setEnabled(true);
		toolButtonChooserPlayEmbedded->setEnabled(true);
		toolButtonChooserSaveConfiguration->setEnabled(!fileModel->isFolder(selected.indexes().first()));
		if ( toolButtonChooserAutoSelect->isChecked() ) {
			QFileInfo fi(fileModel->absolutePath(selected.indexes().first()));
			QString instance = extensionInstanceMap[fi.suffix().toLower()];
			if ( !instance.isEmpty() ) {
		  		int index = comboBoxDeviceInstanceChooser->findText(instance, Qt::MatchExactly);
				if ( index >= 0 ) {
					comboBoxDeviceInstanceChooser->blockSignals(true);
				  	comboBoxDeviceInstanceChooser->setCurrentIndex(index);
					comboBoxDeviceInstanceChooser->blockSignals(false);
				}
			}
		}
	} else {
		toolButtonChooserPlay->setEnabled(false);
		toolButtonChooserPlayEmbedded->setEnabled(false);
		toolButtonChooserSaveConfiguration->setEnabled(false);
	}
}

void MESSDeviceConfigurator::on_toolButtonChooserFilter_toggled(bool enabled)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_toolButtonChooserFilter_toggled(bool enabled = %1)").arg(enabled));
#endif

	if ( enabled ) {
		QList<QTreeWidgetItem *> items = treeWidgetDeviceSetup->findItems(comboBoxDeviceInstanceChooser->currentText(), Qt::MatchExactly);
		QStringList extensions = items[0]->text(QMC2_DEVCONFIG_COLUMN_EXT).split("/", QString::SkipEmptyParts);
		extensions << "zip";
		for (int i = 0; i < extensions.count(); i++) {
			QString ext = extensions[i];
			QString altExt;
			for (int j = 0; j < ext.length(); j++) {
				QChar c = ext[j].toLower();
				altExt += QString("[%1%2]").arg(c).arg(c.toUpper());
			}
			extensions[i] = QString("*.%1").arg(altExt);
		}
		fileModel->setNameFilters(extensions);
	} else
		fileModel->setNameFilters(QStringList());

	fileModelRowInsertionCounter = 0;
	lcdNumberFileCounter->display(0);
	lcdNumberFileCounter->setSegmentStyle(QLCDNumber::Flat);
	lcdNumberFileCounter->update();
	treeViewFileChooser->selectionModel()->clearSelection();
	treeViewFileChooser->selectionModel()->reset();
	treeViewFileChooser->setUpdatesEnabled(false);
	toolButtonChooserReload->setEnabled(false);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
}

void MESSDeviceConfigurator::folderModeMenu_foldersOff()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::folderModeMenu_foldersOff()");
#endif

	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-off.png")));
	includeFolders = false;
	foldersFirst = false;
	fileModel->setIncludeFolders(includeFolders);
	fileModel->setFoldersFirst(foldersFirst);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
}

void MESSDeviceConfigurator::folderModeMenu_foldersOn()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::folderModeMenu_foldersOn()");
#endif

	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-on.png")));
	includeFolders = true;
	foldersFirst = false;
	fileModel->setIncludeFolders(includeFolders);
	fileModel->setFoldersFirst(foldersFirst);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
}

void MESSDeviceConfigurator::folderModeMenu_foldersFirst()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::folderModeMenu_foldersFirst()");
#endif

	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-first.png")));
	includeFolders = true;
	foldersFirst = true;
	fileModel->setIncludeFolders(includeFolders);
	fileModel->setFoldersFirst(foldersFirst);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
}

void MESSDeviceConfigurator::on_comboBoxDeviceInstanceChooser_activated(const QString &text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_comboBoxDeviceInstanceChooser_activated(const QString &text = %1)").arg(text));
#endif

	if ( toolButtonChooserFilter->isChecked() )
		on_toolButtonChooserFilter_toggled(true);
}

void MESSDeviceConfigurator::on_treeViewDirChooser_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_treeViewDirChooser_customContextMenuRequested(const QPoint &p = ...)");
#endif

	modelIndexDirModel = treeViewDirChooser->indexAt(p);
	if ( modelIndexDirModel.isValid() ) {
		dirChooserContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeViewDirChooser->viewport()->mapToGlobal(p), dirChooserContextMenu));
		dirChooserContextMenu->show();
	}
}

void MESSDeviceConfigurator::dirChooserUseCurrentAsDefaultDirectory()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::dirChooserUseCurrentAsDefaultDirectory()");
#endif

	if ( modelIndexDirModel.isValid() ) {
		QString path = dirModel->fileInfo(modelIndexDirModel).absoluteFilePath();
		if ( !path.isEmpty() )
			 qmc2Config->setValue(QString("MESS/Configuration/Devices/%1/DefaultDeviceDirectory").arg(messMachineName), path);
	}
}

void MESSDeviceConfigurator::on_treeViewFileChooser_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_treeViewFileChooser_customContextMenuRequested(const QPoint &p = ...)");
#endif

	modelIndexFileModel = treeViewFileChooser->indexAt(p);
	actionChooserPlay->setVisible(true);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	actionChooserPlayEmbedded->setVisible(true);
#endif
	if ( modelIndexFileModel.isValid() ) {
		actionChooserViewPdf->setVisible(fileModel->isPdf(modelIndexFileModel));
		actionChooserViewPostscript->setVisible(fileModel->isPostscript(modelIndexFileModel));
		actionChooserViewHtml->setVisible(fileModel->isHtml(modelIndexFileModel));
		if ( fileModel->isZip(modelIndexFileModel) ) {
			actionChooserToggleArchive->setText(treeViewFileChooser->isExpanded(modelIndexFileModel) ? tr("&Close archive") : tr("&Open archive"));
			actionChooserToggleArchive->setVisible(true);
		} else
			actionChooserToggleArchive->setVisible(false);
		if ( fileModel->isFolder(modelIndexFileModel) ) {
			actionChooserPlay->setVisible(false);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			actionChooserPlayEmbedded->setVisible(false);
#endif
			actionChooserOpenFolder->setVisible(true);
			actionChooserOpenExternally->setVisible(false);
		} else {
			actionChooserOpenFolder->setVisible(false);
			actionChooserOpenExternally->setVisible(!fileModel->isZipContent(modelIndexFileModel));
		}
		fileChooserContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeViewFileChooser->viewport()->mapToGlobal(p), fileChooserContextMenu));
		fileChooserContextMenu->show();
	}
}

void MESSDeviceConfigurator::on_treeViewFileChooser_clicked(const QModelIndex &index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_treeViewFileChooser_clicked(const QModelIndex &index = ...)");
#endif

	treeViewFileChooser_selectionChanged(QItemSelection(index, index), QItemSelection());
}

void MESSDeviceConfigurator::on_treeViewFileChooser_activated(const QModelIndex &index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_treeViewFileChooser_activated(const QModelIndex &index = ...)");
#endif

	if ( toolButtonChooserProcessZIPs->isChecked() && fileModel->isZip(index) ) {
		if ( treeViewFileChooser->isExpanded(index) ) {
			treeViewFileChooser->setExpanded(index, false);
		} else {
			treeViewFileChooser->setExpanded(index, true);
			fileModel->openZip(index);
			fileModel->sortOpenZip(index, treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
		}
	} else if ( fileModel->isFolder(index) ) {
		treeViewFileChooser_openFolder();
	} else {
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_triggered()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_triggered()));
				break;
		}
	}
}

void MESSDeviceConfigurator::treeViewFileChooser_toggleArchive()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_toggleArchive()");
#endif

	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 ) {
		QModelIndex index = selected[0];
		if ( treeViewFileChooser->isExpanded(index) ) {
			treeViewFileChooser->setExpanded(index, false);
		} else {
			treeViewFileChooser->setExpanded(index, true);
			fileModel->openZip(index);
			fileModel->sortOpenZip(index, treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
		}
	}
}

void MESSDeviceConfigurator::treeViewFileChooser_viewPdf()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_viewPdf()");
#endif

	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 )
		qmc2MainWindow->viewPdf(fileModel->fileName(selected[0]));
}

void MESSDeviceConfigurator::treeViewFileChooser_viewHtml()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_viewHtml()");
#endif

	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 )
		qmc2MainWindow->viewHtml(fileModel->fileName(selected[0]));
}

void MESSDeviceConfigurator::treeViewFileChooser_openFileExternally()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_openFileExternally()");
#endif

	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 )
		QDesktopServices::openUrl(QUrl::fromUserInput(fileModel->fileName(selected[0])));
}

void MESSDeviceConfigurator::treeViewFileChooser_openFolder()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_openFolder()");
#endif

	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 ) {
		QModelIndex index = selected[0];
		QString folderPath = fileModel->absolutePath(index);
		QModelIndex dirIndex = dirModel->index(folderPath);
		if ( dirIndex.isValid() )
			treeViewDirChooser->setCurrentIndex(dirIndex);
		else
			treeViewDirChooser->setCurrentIndex(dirModel->index(dirModel->rootPath()));
	}
}

void MESSDeviceConfigurator::treeViewFileChooser_expandRequested()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_expandRequested()");
#endif

	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 ) {
		QModelIndex index = selected[0];
		if ( !treeViewFileChooser->isExpanded(index) ) {
			if ( toolButtonChooserProcessZIPs->isChecked() && fileModel->isZip(index) ) {
				treeViewFileChooser->setExpanded(index, true);
				fileModel->openZip(index);
				fileModel->sortOpenZip(index, treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
			}
		}
	}
}

void MESSDeviceConfigurator::treeViewDirChooser_headerClicked(int)
{
	dirChooserHeaderState = treeViewDirChooser->header()->saveState();
  	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DirChooserHeaderState", dirChooserHeaderState);
}

void MESSDeviceConfigurator::treeViewFileChooser_headerClicked(int)
{
	fileChooserHeaderState = treeViewFileChooser->header()->saveState();
  	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserHeaderState", fileChooserHeaderState);
}

void MESSDeviceConfigurator::treeViewFileChooser_sectionMoved(int, int, int)
{
	treeViewFileChooser_headerClicked(0);
}

void MESSDeviceConfigurator::treeViewFileChooser_sectionResized(int, int, int)
{
	treeViewFileChooser_headerClicked(0);
}

void MESSDeviceConfigurator::fileModel_rowsInserted(const QModelIndex &, int start, int end)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::fileModel_rowsInserted(const QModelIndex &, int start = ..., int end = ...)");
#endif

	fileModelRowInsertionCounter += end - start;
	if ( fileModelRowInsertionCounter > QMC2_FILECHOOSER_INSERTED_ROWS ) {
		fileModelRowInsertionCounter = 0;
		treeViewFileChooser->setUpdatesEnabled(true);
		treeViewFileChooser->update();
		treeViewFileChooser->setUpdatesEnabled(false);
	}
	lcdNumberFileCounter->display(fileModel->rowCount());
	lcdNumberFileCounter->update();
}

void MESSDeviceConfigurator::fileModel_finished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::fileModel_finished()");
#endif

	lcdNumberFileCounter->display(fileModel->rowCount());
	lcdNumberFileCounter->setSegmentStyle(QLCDNumber::Outline);
	lcdNumberFileCounter->update();
	treeViewFileChooser->setUpdatesEnabled(true);
	treeViewFileChooser->update();
#if QT_VERSION >= 0x050000
	// a sorting-enabled tree-view that's already sorted will not be resorted in case of Qt 5 (that's good, but we need to do it here in order to support "folders first")
	treeViewFileChooser->setSortingEnabled(false);
	treeViewFileChooser->sortByColumn(treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
	treeViewFileChooser->setSortingEnabled(true);
#else
	treeViewFileChooser->sortByColumn(treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
#endif
	toolButtonChooserReload->setEnabled(true);
	if ( comboBoxChooserFilterPatternHadFocus )
		comboBoxChooserFilterPattern->setFocus();
	comboBoxChooserFilterPatternHadFocus = false;
}

void MESSDeviceConfigurator::on_toolButtonChooserReload_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_toolButtonChooserReload_clicked()");
#endif

	fileModelRowInsertionCounter = 0;
	lcdNumberFileCounter->display(0);
	lcdNumberFileCounter->setSegmentStyle(QLCDNumber::Flat);
	lcdNumberFileCounter->update();
	treeViewFileChooser->selectionModel()->clearSelection();
	treeViewFileChooser->selectionModel()->reset();
	treeViewFileChooser->setUpdatesEnabled(false);
	toolButtonChooserReload->setEnabled(false);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
}

void MESSDeviceConfigurator::on_comboBoxChooserFilterPattern_editTextChanged(const QString &)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_comboBoxChooserFilterPattern_editTextChanged(const QString &)");
#endif

	searchTimer.start(QMC2_SEARCH_DELAY * 2);
}

void MESSDeviceConfigurator::comboBoxChooserFilterPattern_editTextChanged_delayed()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::comboBoxChooserFilterPattern_editTextChanged_delayed()");
#endif

	searchTimer.stop();

	if ( fileModel ) {
		fileModel->setSearchPattern(comboBoxChooserFilterPattern->currentText());
		comboBoxChooserFilterPatternHadFocus = comboBoxChooserFilterPattern->hasFocus();
		on_toolButtonChooserReload_clicked();
	}
}

void MESSDeviceConfigurator::on_toolButtonChooserSaveConfiguration_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_toolButtonChooserSaveConfiguration_clicked()");
#endif

	QString instance = comboBoxDeviceInstanceChooser->currentText();
	QModelIndexList indexList = treeViewFileChooser->selectionModel()->selectedIndexes();
	if ( indexList.count() > 0 && instance != tr("No devices available") ) {
		QString file = fileModel->absolutePath(indexList[0]);
		QString targetName;
		bool goOn = false;
		do {
			QFileInfo fi(file);
			QString sourceName = fi.completeBaseName();
			targetName = sourceName;
			int copies = 0;
			while ( configurationMap.contains(targetName) )
				targetName = tr("%1. variant of ").arg(++copies) + sourceName;
			bool ok;
			QString text = QInputDialog::getText(this, tr("Choose a unique configuration name"), tr("Unique configuration name:"), QLineEdit::Normal, targetName, &ok);
			if ( ok && !text.isEmpty() ) {
				if ( configurationMap.contains(text) ) {
					switch ( QMessageBox::question(this, tr("Name conflict"), tr("A configuration named '%1' already exists.\n\nDo you want to choose a different name?").arg(text), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ) {
						case QMessageBox::Yes:
							goOn = true;
							break;
						case QMessageBox::No:
						default:
							goOn = false;
							break;
					}
				} else {
					targetName = text;
					QStringList instances;
					QStringList files;
					instances << instance;
					files << file;

					// save device instances
					configurationMap[targetName].first = instances;
					configurationMap[targetName].second = files;

					// save slot setup
					QList<QTreeWidgetItem *> allSlotItems;
					for (int i = 0; i < treeWidgetSlotOptions->topLevelItemCount(); i++) {
						QTreeWidgetItem *item = treeWidgetSlotOptions->topLevelItem(i);
						allSlotItems << item;
						insertChildItems(item, allSlotItems);
					}
					QStringList slotNames, slotOptions, slotBIOSs;
					foreach (QTreeWidgetItem *item, allSlotItems) {
						QString slotName = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT);
						if ( !slotName.isEmpty() ) {
							QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
							if ( cb ) {
								int defaultIndex = -1;
								if ( slotPreselectionMap.contains(cb) )
									defaultIndex = slotPreselectionMap[cb];
								else if ( nestedSlotPreselectionMap.contains(cb) )
									defaultIndex = nestedSlotPreselectionMap[cb];
								QComboBox *cbBIOS = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
								if ( cbBIOS ) {
									QString biosChoice = cbBIOS->currentText().split(" ", QString::SkipEmptyParts)[0];
									if ( biosChoice == tr("N/A") )
										biosChoice.clear();
									slotBIOSs << biosChoice;
								}
								if ( cb->currentIndex() > 0 && defaultIndex == 0 ) {
									slotNames << slotName;
									slotOptions << cb->currentText().split(" ")[0];
								} else if (cb->currentIndex() == 0 && defaultIndex > 0 ) {
									slotNames << slotName;
									slotOptions << "\"\"";
								} else if ( cb->currentIndex() > 0 && defaultIndex > 0 && cb->currentIndex() != defaultIndex ) {
									slotNames << slotName;
									slotOptions << cb->currentText().split(" ")[0];
								}
							}
						}
					}
					slotMap[targetName].first = slotNames;
					slotMap[targetName].second = slotOptions;
					slotBiosMap[targetName].first = slotNames;
					slotBiosMap[targetName].second = slotBIOSs;

					listWidgetDeviceConfigurations->setSortingEnabled(false);
					int row = listWidgetDeviceConfigurations->count();
					listWidgetDeviceConfigurations->insertItem(row, targetName);
					listWidgetDeviceConfigurations->item(row)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
					listWidgetDeviceConfigurations->setSortingEnabled(true);
					listWidgetDeviceConfigurations->sortItems(Qt::AscendingOrder);
					goOn = false;
				}
			} else
				goOn = false;
		} while ( goOn );
	}
}

void MESSDeviceConfigurator::on_splitterFileChooser_splitterMoved(int, int)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_splitterFileChooser_splitterMoved(int, int)");
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserSplitter", QSize(splitterFileChooser->sizes().at(0), splitterFileChooser->sizes().at(1)));
}

MESSDeviceConfiguratorXmlHandler::MESSDeviceConfiguratorXmlHandler(QTreeWidget *parent)
{
	parentTreeWidget = parent;
}

bool MESSDeviceConfiguratorXmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
	if ( qName == "device" ) {
		deviceType = attributes.value("type");
		deviceTag = attributes.value("tag");
		deviceInstances.clear();
		deviceExtensions.clear();
		deviceBriefName.clear();
	} else if ( qName == "instance" ) {
		deviceInstances << attributes.value("name");
		deviceBriefName = attributes.value("briefname");
	} else if ( qName == "extension" ) {
		deviceExtensions << attributes.value("name");
	} else if ( qName == "slot" ) {
		slotName = attributes.value("name");
		if ( messSystemSlotHash[qmc2MESSDeviceConfigurator->messMachineName]["QMC2_UNUSED_SLOTS"].contains(slotName) )
			return true;
		allSlots << slotName;
		if ( !messSystemSlotHash[qmc2MESSDeviceConfigurator->messMachineName].contains(slotName) )
			newSlots << slotName;
	} else if ( qName == "slotoption" ) {
		if ( !messSystemSlotHash[qmc2MESSDeviceConfigurator->messMachineName].contains(slotName) ) {
			newSlotOptions[slotName] << attributes.value("name");
			newSlotDevices[attributes.value("name")] = attributes.value("devname");
		}
		slotDeviceNames[attributes.value("name")] = attributes.value("devname");
		if ( attributes.value("default") == "yes" )
			defaultSlotOptions[slotName] = attributes.value("name");
	}

	return true;
}

bool MESSDeviceConfiguratorXmlHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName)
{
	if ( qName == "device" ) {
		foreach (QString instance, deviceInstances) {
			if ( !instance.isEmpty() ) {
				QTreeWidgetItem *deviceItem = new QTreeWidgetItem(parentTreeWidget);
				deviceItem->setText(QMC2_DEVCONFIG_COLUMN_NAME, instance);
				if ( !deviceType.isEmpty() )
					deviceItem->setIcon(QMC2_DEVCONFIG_COLUMN_NAME, messDevIconHash[deviceType]);
				deviceItem->setText(QMC2_DEVCONFIG_COLUMN_BRIEF, deviceBriefName);
				deviceItem->setText(QMC2_DEVCONFIG_COLUMN_TYPE, deviceType);
				deviceItem->setText(QMC2_DEVCONFIG_COLUMN_TAG, deviceTag);
				deviceItem->setText(QMC2_DEVCONFIG_COLUMN_EXT, deviceExtensions.join("/"));
				parentTreeWidget->openPersistentEditor(deviceItem, QMC2_DEVCONFIG_COLUMN_FILE);
				deviceItem->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, QString());
			}
		}
	}

	return true;
}

bool MESSDeviceConfiguratorXmlHandler::characters(const QString &)
{
	return true;
}

bool FileChooserKeyEventFilter::eventFilter(QObject *obj, QEvent *event)
{
	if ( event->type() == QEvent::KeyPress ) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		switch ( keyEvent->key() ) {
			case Qt::Key_Right:
			case Qt::Key_Plus:
				emit expandRequested();
				break;
			default:
				break;
		}
	}
	return QObject::eventFilter(obj, event);
}
