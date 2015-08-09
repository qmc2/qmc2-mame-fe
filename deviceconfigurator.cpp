#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#endif
#include <QMap>
#include <QProcess>

#include <algorithm> // std::sort()

#include "deviceconfigurator.h"
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
extern DeviceConfigurator *qmc2DeviceConfigurator;
extern bool qmc2UseDefaultEmulator;
extern bool qmc2TemplateCheck;
extern Options *qmc2Options;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;

QHash<QString, QHash<QString, QStringList> > DeviceConfigurator::systemSlotHash;
QHash<QString, QString> DeviceConfigurator::slotNameHash;
QHash<QString, QIcon> DeviceConfigurator::deviceIconHash;
QHash<QString, int> DeviceConfigurator::deviceNameToIndexHash;
QStringList DeviceConfigurator::midiInInterfaces;
QStringList DeviceConfigurator::midiOutInterfaces;
bool DeviceConfigurator::reloadMidiInterfaces = true;

DeviceItemDelegate::DeviceItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{
	// NOP
}

QWidget *DeviceItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
	QModelIndex sibling = index.sibling(index.row(), QMC2_DEVCONFIG_COLUMN_EXT);
	QStringList extensions = sibling.data(Qt::EditRole).toString().split("/", QString::SkipEmptyParts);
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
	FileEditWidget *fileEditWidget;
	switch ( DeviceConfigurator::deviceNameToIndexHash[index.sibling(index.row(), QMC2_DEVCONFIG_COLUMN_TYPE).data(Qt::EditRole).toString()] ) {
		case QMC2_DEVTYPE_MIDIIN:
			if ( DeviceConfigurator::reloadMidiInterfaces )
				loadMidiInterfaces();
			fileEditWidget = new FileEditWidget(QString(), filterString, QString(), parent, true, QString(), 0, true);
			fileEditWidget->comboBox->addItems(DeviceConfigurator::midiInInterfaces);
			break;
		case QMC2_DEVTYPE_MIDIOUT:
			if ( DeviceConfigurator::reloadMidiInterfaces )
				loadMidiInterfaces();
			fileEditWidget = new FileEditWidget(QString(), filterString, QString(), parent, true, QString(), 0, true);
			fileEditWidget->comboBox->addItems(DeviceConfigurator::midiOutInterfaces);
			break;
		default:
			fileEditWidget = new FileEditWidget(QString(), filterString, QString(), parent, true);
			break;
	}
	fileEditWidget->installEventFilter(const_cast<DeviceItemDelegate*>(this));
	connect(fileEditWidget, SIGNAL(dataChanged(QWidget *)), this, SLOT(dataChanged(QWidget *)));
	return fileEditWidget;
}

void DeviceItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QString value = index.model()->data(index, Qt::EditRole).toString();
	FileEditWidget *fileEditWidget = static_cast<FileEditWidget *>(editor);
	if ( fileEditWidget->comboBoxMode() ) {
		int cPos = fileEditWidget->comboBox->lineEdit()->cursorPosition();
		fileEditWidget->comboBox->lineEdit()->setText(value);
		fileEditWidget->comboBox->lineEdit()->setCursorPosition(cPos);
	} else {
		int cPos = fileEditWidget->lineEditFile->cursorPosition();
		fileEditWidget->lineEditFile->setText(value);
		fileEditWidget->lineEditFile->setCursorPosition(cPos);
	}
}

void DeviceItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	FileEditWidget *fileEditWidget = static_cast<FileEditWidget*>(editor);
	QString v;
	if ( fileEditWidget->comboBoxMode() )
		v = fileEditWidget->comboBox->lineEdit()->text();
	else
		v = fileEditWidget->lineEditFile->text();
	model->setData(index, v, Qt::EditRole);
}

void DeviceItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
	editor->setGeometry(option.rect);
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	FileEditWidget *fileEditWidget = static_cast<FileEditWidget *>(editor);
	fileEditWidget->toolButtonBrowse->setIconSize(iconSize);
}

void DeviceItemDelegate::dataChanged(QWidget *widget)
{
	emit commitData(widget);
	FileEditWidget *fileEditWidget = static_cast<FileEditWidget*>(widget);
	if ( fileEditWidget->comboBoxMode() )
		emit editorDataChanged(fileEditWidget->comboBox->lineEdit()->text());
	else
		emit editorDataChanged(fileEditWidget->lineEditFile->text());
}

void DeviceItemDelegate::loadMidiInterfaces()
{
	DeviceConfigurator::midiInInterfaces.clear();
	DeviceConfigurator::midiOutInterfaces.clear();

	QString userScopePath = Options::configPath();
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
	args << "-listmidi";
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
		while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
			qApp->processEvents();
			commandProcRunning = (commandProc.state() == QProcess::Running);
		}
#if defined(QMC2_SDLMAME)
		QFile qmc2TempFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
		QFile qmc2TempFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#endif
		if ( commandProcStarted && qmc2TempFile.open(QFile::ReadOnly) ) {
			QTextStream ts(&qmc2TempFile);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			QString buffer = ts.readAll();
#if defined(QMC2_OS_WIN)
			buffer.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
			qmc2TempFile.close();
			qmc2TempFile.remove();
			if ( !buffer.isEmpty() ) {
				QStringList lines = buffer.split("\n", QString::SkipEmptyParts);
				QStringList midiInOutMarks = QStringList() << "MIDI input ports:" << "MIDI output ports:";
				bool midiIn = false;
				bool midiOut = false;
				int i = 0;
				while ( i < lines.count() ) {
					QString line = lines[i++];
					line = line.replace("(default)", "").trimmed();
					if ( midiInOutMarks.contains(line) ) {
						midiIn = midiInOutMarks.indexOf(line) == 0;
						midiOut = midiInOutMarks.indexOf(line) == 1;
						continue;
					}
					if ( midiIn )
						DeviceConfigurator::midiInInterfaces << line;
					if ( midiOut )
						DeviceConfigurator::midiOutInterfaces << line;
				}
			}
		}

		DeviceConfigurator::reloadMidiInterfaces = false;
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start emulator executable within a reasonable time frame, giving up"));
}

DeviceConfigurator::DeviceConfigurator(QString machineName, QWidget *parent)
	: QWidget(parent)
{
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
	comboBoxChooserFilterPatternHadFocus = false;
	dirModel = NULL;
	fileModel = NULL;
	configurationRenameItem = NULL;
	fileChooserSetup = refreshRunning = dontIgnoreNameChange = isLoading = isManualSlotOptionChange = fullyLoaded = forceQuit = false;
	updateSlots = true;

	lineEditConfigurationName->blockSignals(true);
	if ( systemSlotHash.isEmpty() ) {
		lineEditConfigurationName->setText(tr("Reading slot info, please wait..."));
		comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Reading slot info, please wait..."));
	} else {
		lineEditConfigurationName->setText(tr("Default configuration"));
		comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Enter search string"));
	}
	lineEditConfigurationName->setPlaceholderText(tr("Enter configuration name"));
	lineEditConfigurationName->blockSignals(false);

	currentMachineName = machineName;
	treeWidgetDeviceSetup->setItemDelegateForColumn(QMC2_DEVCONFIG_COLUMN_FILE, &fileEditDelegate);
	connect(&fileEditDelegate, SIGNAL(editorDataChanged(const QString &)), this, SLOT(editorDataChanged(const QString &)));
	tabWidgetDeviceSetup->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/DeviceSetupTab", 0).toInt());
	treeWidgetDeviceSetup->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/DeviceSetupHeaderState").toByteArray());
	treeWidgetSlotOptions->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/SlotSetupHeaderState").toByteArray());
	toolButtonChooserAutoSelect->blockSignals(true);
	toolButtonChooserAutoSelect->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserAutoSelect", false).toBool());
	toolButtonChooserAutoSelect->blockSignals(false);
	toolButtonChooserFilter->blockSignals(true);
	toolButtonChooserFilter->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserFilter", false).toBool());
	toolButtonChooserFilter->blockSignals(false);
	toolButtonChooserProcessZIPs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserProcessZIPs", false).toBool());
	toolButtonChooserMergeMaps->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserMergeMaps", false).toBool());
	QString folderMode = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FolderMode", "folders-off").toString();
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
	QSize splitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserSplitter").toSize();
	if ( splitterSize.width() > 0 || splitterSize.height() > 0 )
		splitterSizes << splitterSize.width() << splitterSize.height();
	else
		splitterSizes << 30 << 70;
	splitterFileChooser->setSizes(splitterSizes);

	QList<int> vSplitterSizes;
	QSize vSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/vSplitter").toSize();
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
	QAction *action = configurationMenu->addAction(tr("&Default device directory for '%1'...").arg(currentMachineName));
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

	if ( deviceIconHash.isEmpty() ) {
		deviceIconHash["cartridge"] = QIcon(QString::fromUtf8(":/data/img/dev_cartridge.png"));
		deviceIconHash["cassette"] = QIcon(QString::fromUtf8(":/data/img/dev_cassette.png"));
		deviceIconHash["cdrom"] = QIcon(QString::fromUtf8(":/data/img/dev_cdrom.png"));
		deviceIconHash["cylinder"] = QIcon(QString::fromUtf8(":/data/img/dev_cylinder.png"));
		deviceIconHash["floppydisk"] = QIcon(QString::fromUtf8(":/data/img/dev_floppydisk.png"));
		deviceIconHash["harddisk"] = QIcon(QString::fromUtf8(":/data/img/dev_harddisk.png"));
		deviceIconHash["magtape"] = QIcon(QString::fromUtf8(":/data/img/dev_magtape.png"));
		deviceIconHash["memcard"] = QIcon(QString::fromUtf8(":/data/img/dev_memcard.png"));
		deviceIconHash["parallel"] = QIcon(QString::fromUtf8(":/data/img/dev_parallel.png"));
		deviceIconHash["printer"] = QIcon(QString::fromUtf8(":/data/img/dev_printer.png"));
		deviceIconHash["punchtape"] = QIcon(QString::fromUtf8(":/data/img/dev_punchtape.png"));
		deviceIconHash["quickload"] = QIcon(QString::fromUtf8(":/data/img/dev_quickload.png"));
		deviceIconHash["serial"] = QIcon(QString::fromUtf8(":/data/img/dev_serial.png"));
		deviceIconHash["snapshot"] = QIcon(QString::fromUtf8(":/data/img/dev_snapshot.png"));
		deviceIconHash["romimage"] = QIcon(QString::fromUtf8(":/data/img/rom.png"));
		deviceIconHash["midiin"] = QIcon(QString::fromUtf8(":/data/img/midi-in.png"));
		deviceIconHash["midiout"] = QIcon(QString::fromUtf8(":/data/img/midi-out.png"));
		deviceNameToIndexHash["cartridge"] = QMC2_DEVTYPE_CARTRIDGE;
		deviceNameToIndexHash["cassette"] = QMC2_DEVTYPE_CASSETTE;
		deviceNameToIndexHash["cdrom"] = QMC2_DEVTYPE_CDROM;
		deviceNameToIndexHash["cylinder"] = QMC2_DEVTYPE_CYLINDER;
		deviceNameToIndexHash["floppydisk"] = QMC2_DEVTYPE_FLOPPYDISK;
		deviceNameToIndexHash["harddisk"] = QMC2_DEVTYPE_HARDDISK;
		deviceNameToIndexHash["magtape"] = QMC2_DEVTYPE_MAGTAPE;
		deviceNameToIndexHash["memcard"] = QMC2_DEVTYPE_MEMCARD;
		deviceNameToIndexHash["parallel"] = QMC2_DEVTYPE_PARALLEL;
		deviceNameToIndexHash["printer"] = QMC2_DEVTYPE_PRINTER;
		deviceNameToIndexHash["punchtape"] = QMC2_DEVTYPE_PUNCHTAPE;
		deviceNameToIndexHash["quickload"] = QMC2_DEVTYPE_QUICKLOAD;
		deviceNameToIndexHash["serial"] = QMC2_DEVTYPE_SERIAL;
		deviceNameToIndexHash["snapshot"] = QMC2_DEVTYPE_SNAPSHOT;
		deviceNameToIndexHash["romimage"] = QMC2_DEVTYPE_ROMIMAGE;
		deviceNameToIndexHash["midiin"] = QMC2_DEVTYPE_MIDIIN;
		deviceNameToIndexHash["midiout"] = QMC2_DEVTYPE_MIDIOUT;
	}

	FileChooserKeyEventFilter *eventFilter = new FileChooserKeyEventFilter(this);
	treeViewFileChooser->installEventFilter(eventFilter);
	connect(eventFilter, SIGNAL(expandRequested()), this, SLOT(treeViewFileChooser_expandRequested()));
}

DeviceConfigurator::~DeviceConfigurator()
{
	// NOP
}

void DeviceConfigurator::saveSetup()
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/DeviceSetupHeaderState", treeWidgetDeviceSetup->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/SlotSetupHeaderState", treeWidgetSlotOptions->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/DeviceSetupTab", tabWidgetDeviceSetup->currentIndex());
	if ( tabWidgetDeviceSetup->currentIndex() != QMC2_DEVSETUP_TAB_FILECHOOSER )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/vSplitter", QSize(vSplitter->sizes().at(0), vSplitter->sizes().at(1)));
	else
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserSplitter", QSize(splitterFileChooser->sizes().at(0), splitterFileChooser->sizes().at(1)));
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserFilter", toolButtonChooserFilter->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserAutoSelect", toolButtonChooserAutoSelect->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserProcessZIPs", toolButtonChooserProcessZIPs->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserMergeMaps", toolButtonChooserMergeMaps->isChecked());
	if ( !fileChooserHeaderState.isEmpty() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserHeaderState", fileChooserHeaderState);
	if ( !dirChooserHeaderState.isEmpty() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/DirChooserHeaderState", dirChooserHeaderState);
	if ( comboBoxDeviceInstanceChooser->currentText() != tr("No devices available") )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserDeviceInstance", comboBoxDeviceInstanceChooser->currentText());
	if ( includeFolders && foldersFirst )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FolderMode", "folders-first");
	else if ( includeFolders )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FolderMode", "folders-on");
	else
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FolderMode", "folders-off");
}

bool DeviceConfigurator::checkParentSlot(QTreeWidgetItem *item, QString &slotName)
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

QString &DeviceConfigurator::getXmlDataWithEnabledSlots(QString machineName)
{
	qmc2CriticalSection = true;
	slotXmlBuffer.clear();

	QString userScopePath = Options::configPath();
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
			bool isNestedSlot = !systemSlotHash[currentMachineName].contains(slotName);
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
				QMC2_PRINT_STRTXT(QString("DeviceConfigurator::getXmlDataWithEnabledSlots(): slotName = %1, isNested = %2, defaultIndex = %3, addArg = %4").arg(slotName).arg(isNestedSlot ? "true" : "false").arg(defaultIndex).arg(addArg ? "true" : "false"));
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
	QMC2_PRINT_STRTXT(QString("DeviceConfigurator::getXmlDataWithEnabledSlots(): args = %1").arg(args.join(" ")));
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
		qmc2TempXml.close();
		qmc2TempXml.remove();
		/*
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
		*/
	}

	qmc2CriticalSection = false;
	return slotXmlBuffer;
}

QString &DeviceConfigurator::getXmlData(QString machineName)
{
	normalXmlBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	normalXmlBuffer += qmc2MachineList->xmlDb()->xml(machineName);
	return normalXmlBuffer;
}

bool DeviceConfigurator::readSystemSlots()
{
	QTime elapsedTime(0, 0, 0, 0);
	QTime loadTimer;

	QString userScopePath = Options::configPath();
	QString slotInfoCachePath;
	slotInfoCachePath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SlotInfoCacheFile", userScopePath + "/mame.sic").toString();

	setEnabled(false);
	lineEditConfigurationName->blockSignals(true);
	lineEditConfigurationName->setText(tr("Reading slot info, please wait..."));
	comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Reading slot info, please wait..."));
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
			comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Failed to read slot info"));
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
			comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Failed to read slot info"));
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
								slotNameHash[slotOption] = slotDeviceName;
								systemSlotHash[systemName][slotName] << slotOption;
							} else
								systemSlotHash[systemName][strUnused] << slotName;
						} else
							systemSlotHash[systemName].remove(slotName);
					} else {
						systemName = slotWords[0];
						systemSlotHash[systemName].clear();
					}
				} else {
					QStringList slotWords = slotLineTrimmed.split(" ", QString::SkipEmptyParts);
					if ( slotLine[13] == ' ' ) { // this isn't nice, but I see no other way at the moment...
						if ( slotName.split(":", QString::SkipEmptyParts).count() < 3 ) {
							slotOption = slotWords[0];
							if ( slotOption != strNone ) {
								slotDeviceName = slotLineTrimmed;
								slotDeviceName.remove(rxSlotDev2);
								slotNameHash[slotOption] = slotDeviceName;
								systemSlotHash[systemName][slotName] << slotOption;
							} else
								systemSlotHash[systemName][strUnused] << slotName;
						}
					} else {
						slotName = slotWords[0];
						if ( slotName.split(":", QString::SkipEmptyParts).count() < 3 && slotWords.count() > 1 ) {
							slotOption = slotWords[1];
							if ( slotOption != strNone ) {
								slotDeviceName = slotLineTrimmed;
								slotDeviceName.remove(rxSlotDev3);
								slotNameHash[slotOption] = slotDeviceName;
								systemSlotHash[systemName][slotName] << slotOption;
							} else
								systemSlotHash[systemName][strUnused] << slotName;
						} else
							systemSlotHash[systemName].remove(slotName);
					}
				}
			}
		}
		slotInfoFile.close();
	} else {
		lineEditConfigurationName->blockSignals(true);
		lineEditConfigurationName->setText(tr("Failed to read slot info"));
		lineEditConfigurationName->blockSignals(false);
		comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Failed to read slot info"));
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

void DeviceConfigurator::slotOptionChanged(int index)
{
	isManualSlotOptionChange = true;
	QTimer::singleShot(QMC2_SLOTOPTION_CHANGE_DELAY, this, SLOT(refreshDeviceMap()));
}

void DeviceConfigurator::addNestedSlot(QString slotName, QStringList slotOptionNames, QStringList slotOptionDescriptions, QString defaultSlotOption)
{
	if ( nestedSlots.contains(slotName) )
		return;

	QStringList slotOptions;
	QStringList slotOptionsShort;
	int count = 0;
	foreach (QString s, slotOptionNames) {
		if ( slotOptionDescriptions[count].isEmpty() )
			slotOptions << s;
		else
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
	QMC2_PRINT_STRTXT(QString("DeviceConfigurator::addNestedSlot(): slotName = %1, parentItem = %2").arg(slotName).arg((qulonglong)parentItem));
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

void DeviceConfigurator::insertChildItems(QTreeWidgetItem *parentItem, QList<QTreeWidgetItem *> &itemList)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		itemList << parentItem->child(i);
		insertChildItems(parentItem->child(i), itemList);
	}
}

void DeviceConfigurator::checkRemovedSlots(QTreeWidgetItem *parentItem)
{
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
				QMC2_PRINT_STRTXT(QString("DeviceConfigurator::checkRemovedSlots(): removedSlot = %1").arg(slotName));
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

QComboBox *DeviceConfigurator::comboBoxByName(QString slotName, QTreeWidgetItem **returnItem)
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

bool DeviceConfigurator::refreshDeviceMap()
{
	bool wasManualSlotOptionChange = isManualSlotOptionChange;
	isManualSlotOptionChange = false;

	if ( refreshRunning || forceQuit )
		return false;

	refreshRunning = true;

	QString xmlBuffer = getXmlDataWithEnabledSlots(currentMachineName);

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
	DeviceConfiguratorXmlHandler xmlHandler(treeWidgetDeviceSetup, currentMachineName);
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(&xmlHandler);
	if ( !xmlReader.parse(xmlInputSource) ) {
		refreshRunning = false;
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for '%1'").arg(currentMachineName));
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
			QMC2_PRINT_STRTXT(QString("DeviceConfigurator::refreshDeviceMap(): newSlot = %1").arg(newSlot));
#endif
			QStringList newSlotOptionDescriptions;
			foreach (QString newSlotOption, xmlHandler.newSlotOptions[newSlot]) {
				QString xmlHandlerNewSlotOption = xmlHandler.newSlotDevices[newSlotOption];
				QTreeWidgetItem *item = qmc2MachineListItemHash[xmlHandlerNewSlotOption];
				if ( item ) {
					QString slotOptionDescription = item->text(QMC2_MACHINELIST_COLUMN_MACHINE);
#ifdef QMC2_DEBUG
					QMC2_PRINT_STRTXT(QString("DeviceConfigurator::refreshDeviceMap():     newSlotOption = %1 [%2], default = %3").arg(newSlotOption).arg(slotOptionDescription).arg(xmlHandler.defaultSlotOptions[newSlot] == newSlotOption ? "yes" : "no"));
#endif
					newSlotOptionDescriptions << slotOptionDescription;
				} else if ( xmlHandler.newDevices.contains(xmlHandlerNewSlotOption) ) {
					QString slotOptionDescription = xmlHandler.newDevices[xmlHandlerNewSlotOption];
#ifdef QMC2_DEBUG
					QMC2_PRINT_STRTXT(QString("DeviceConfigurator::refreshDeviceMap():     newSlotOption = %1 [%2], default = %3").arg(newSlotOption).arg(slotOptionDescription).arg(xmlHandler.defaultSlotOptions[newSlot] == newSlotOption ? "yes" : "no"));
#endif
					newSlotOptionDescriptions << slotOptionDescription;

				} else {
#ifdef QMC2_DEBUG
					QMC2_PRINT_STRTXT(QString("DeviceConfigurator::refreshDeviceMap():     newSlotOption = %1, default = %2").arg(newSlotOption).arg(xmlHandler.defaultSlotOptions[newSlot] == newSlotOption ? "yes" : "no"));
#endif
					newSlotOptionDescriptions << QString();
				}
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
				comboBoxDeviceInstanceChooser->addItem(deviceIconHash[devType], instance);
			}
		}
		QString oldFileChooserDeviceInstance = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserDeviceInstance", QString()).toString();
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
				if ( w1->comboBoxMode() )
					treeWidgetDeviceSetup->setTabOrder(w1->comboBox->lineEdit(), w1->toolButtonBrowse);
				else
					treeWidgetDeviceSetup->setTabOrder(w1->lineEditFile, w1->toolButtonBrowse);
				treeWidgetDeviceSetup->setTabOrder(w1->toolButtonBrowse, w1->toolButtonClear);
				if ( w2->comboBoxMode() )
					treeWidgetDeviceSetup->setTabOrder(w1->toolButtonClear, w2->comboBox->lineEdit());
				else
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

void DeviceConfigurator::updateSlotBiosSelections()
{
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
				biosChoices.removeDuplicates();
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

void DeviceConfigurator::preselectNestedSlots()
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

void DeviceConfigurator::preselectSlots()
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

bool DeviceConfigurator::load()
{
	refreshRunning = true;
	fullyLoaded = false;

	if ( systemSlotHash.isEmpty() )
		if ( !readSystemSlots() ) {
			listWidgetDeviceConfigurations->setUpdatesEnabled(true);
			refreshRunning = false;
			return false;
		}

	comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Enter search string"));

	isLoading = true;

	setEnabled(qmc2UseDefaultEmulator);
	tabSlotOptions->setUpdatesEnabled(false);
	listWidgetDeviceConfigurations->setUpdatesEnabled(false);
	listWidgetDeviceConfigurations->setSortingEnabled(false);
	listWidgetDeviceConfigurations->clear();

	QString xmlBuffer = getXmlData(currentMachineName);
  
	QXmlInputSource xmlInputSource;
	xmlInputSource.setData(xmlBuffer);
	DeviceConfiguratorXmlHandler xmlHandler(treeWidgetDeviceSetup, currentMachineName);
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
				comboBoxDeviceInstanceChooser->addItem(deviceIconHash[devType], instance);
			}
		}
		QString oldFileChooserDeviceInstance = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserDeviceInstance", QString()).toString();
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
		// base system has no slot-devices so disable the device configurator *completely*
		setEnabled(false);
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

	QHashIterator<QString, QStringList> it(systemSlotHash[currentMachineName]);
	slotPreselectionMap.clear();
	nestedSlotPreselectionMap.clear();
	while ( it.hasNext() ) {
		it.next();
		QString slotName = it.key();
		if ( slotName == "QMC2_UNUSED_SLOTS" )
			continue;
		bool isNestedSlot = !systemSlotHash[currentMachineName].contains(slotName);
		QStringList slotOptions;
		QStringList slotOptionsShort;
		foreach (QString s, it.value()) {
			slotOptions << QString("%1 - %2").arg(s).arg(slotNameHash[s]);
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

	QString group = QString("MAME/Configuration/Devices/%1").arg(currentMachineName);
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
		QDir machineSoftwareFolder(qmc2FileEditStartPath + "/" + currentMachineName);
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

bool DeviceConfigurator::save()
{
	if ( !fullyLoaded )
		return false;

	QString group = QString("MAME/Configuration/Devices/%1").arg(currentMachineName);
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

void DeviceConfigurator::on_toolButtonNewConfiguration_clicked()
{
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

void DeviceConfigurator::on_toolButtonCloneConfiguration_clicked()
{
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

void DeviceConfigurator::on_toolButtonSaveConfiguration_clicked()
{
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

void DeviceConfigurator::on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem *item)
{
	item->setSelected(true);
	dontIgnoreNameChange = updateSlots = true;
	on_lineEditConfigurationName_textChanged(item->text());
	dontIgnoreNameChange = updateSlots = false;
	lineEditConfigurationName->blockSignals(true);
	lineEditConfigurationName->setText(item->text());
	lineEditConfigurationName->blockSignals(false);
}

void DeviceConfigurator::on_toolButtonRemoveConfiguration_clicked()
{
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

void DeviceConfigurator::actionRenameConfiguration_activated()
{
	configurationRenameItem = NULL;
	QList<QListWidgetItem *> sl = listWidgetDeviceConfigurations->selectedItems();
	if ( sl.count() > 0 ) {
		configurationRenameItem = sl[0];
		oldConfigurationName = configurationRenameItem->text();
		listWidgetDeviceConfigurations->editItem(configurationRenameItem);
	}
}

void DeviceConfigurator::configurationItemChanged(QListWidgetItem *item)
{
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

void DeviceConfigurator::actionRemoveConfiguration_activated()
{
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

void DeviceConfigurator::on_listWidgetDeviceConfigurations_itemActivated(QListWidgetItem *item)
{
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

void DeviceConfigurator::on_lineEditConfigurationName_textChanged(const QString &text)
{
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
								bool isNestedSlot = !systemSlotHash[currentMachineName].contains(valuePair.first[i]);
								if ( valuePair.second[i] != "\"\"" ) {
									if ( isNestedSlot )
										index = cb->findText(QString("%1 - %2").arg(valuePair.second[i]).arg(nestedSlotOptionMap[valuePair.first[i]][valuePair.second[i]]));
									else
										index = cb->findText(QString("%1 - %2").arg(valuePair.second[i]).arg(slotNameHash[valuePair.second[i]]));
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
						if ( few ) {
							if ( few->comboBoxMode() )
								few->comboBox->lineEdit()->setText(data);
							else
								few->lineEditFile->setText(data);
						}
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
				if ( few ) {
					if ( few->comboBoxMode() )
						few->comboBox->lineEdit()->clear();
					else
						few->lineEditFile->clear();
				}
			}
			toolButtonRemoveConfiguration->setEnabled(false);
			toolButtonCloneConfiguration->setEnabled(false);
		}
	}
	dontIgnoreNameChange = false;
	qmc2CriticalSection = false;
}

void DeviceConfigurator::editorDataChanged(const QString &text)
{
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

void DeviceConfigurator::on_listWidgetDeviceConfigurations_currentTextChanged(const QString &text)
{
	dontIgnoreNameChange = updateSlots = true;
	lineEditConfigurationName->setText(text);
	dontIgnoreNameChange = updateSlots = false;
}

void DeviceConfigurator::on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &point)
{
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

void DeviceConfigurator::on_treeWidgetDeviceSetup_customContextMenuRequested(const QPoint &p)
{
	if ( treeWidgetDeviceSetup->itemAt(p) ) {
		deviceContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetDeviceSetup->viewport()->mapToGlobal(p), deviceContextMenu));
		deviceContextMenu->show();
	}
}

void DeviceConfigurator::on_treeWidgetSlotOptions_customContextMenuRequested(const QPoint &p)
{
	if ( treeWidgetSlotOptions->itemAt(p) ) {
		slotContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSlotOptions->viewport()->mapToGlobal(p), slotContextMenu));
		slotContextMenu->show();
	}
}

void DeviceConfigurator::actionSelectFile_triggered()
{
	QTreeWidgetItem *item = treeWidgetDeviceSetup->currentItem();
	if ( item ) {
		FileEditWidget *few = (FileEditWidget *)treeWidgetDeviceSetup->itemWidget(item, QMC2_DEVCONFIG_COLUMN_FILE);
		if ( few )
			QTimer::singleShot(0, few, SLOT(on_toolButtonBrowse_clicked()));
	}
}

void DeviceConfigurator::actionSelectDefaultDeviceDirectory_triggered()
{
	QString group = QString("MAME/Configuration/Devices/%1").arg(currentMachineName);
	QString path = qmc2Config->value(group + "/DefaultDeviceDirectory", "").toString();

	if ( path.isEmpty() ) {
		path = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(path + "/" + currentMachineName);
		if ( machineSoftwareFolder.exists() )
			path = machineSoftwareFolder.canonicalPath();
	}

	qmc2Config->beginGroup(group);

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose default device directory for '%1'").arg(currentMachineName), path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isEmpty() )
		qmc2Config->setValue("DefaultDeviceDirectory", s);
	qmc2FileEditStartPath = qmc2Config->value("DefaultDeviceDirectory").toString();

	qmc2Config->endGroup();

	if ( qmc2FileEditStartPath.isEmpty() ) {
		qmc2FileEditStartPath = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(qmc2FileEditStartPath + "/" + currentMachineName);
		if ( machineSoftwareFolder.exists() )
			qmc2FileEditStartPath = machineSoftwareFolder.canonicalPath();
	}
}

void DeviceConfigurator::closeEvent(QCloseEvent *e)
{
	if ( e )
		e->accept();
}

void DeviceConfigurator::hideEvent(QHideEvent *e)
{
	save();
	if ( e )
		e->accept();
}

void DeviceConfigurator::showEvent(QShowEvent *e)
{
	if ( e )
		e->accept();
}

void DeviceConfigurator::on_tabWidgetDeviceSetup_currentChanged(int index)
{
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

void DeviceConfigurator::setupFileChooser()
{
	if ( fileChooserSetup ) {
		tabFileChooser->setEnabled(true);
		return;
	}

	fileChooserSetup = true;

	toolButtonChooserPlay->setEnabled(false);
	toolButtonChooserPlayEmbedded->setEnabled(false);
	toolButtonChooserSaveConfiguration->setEnabled(false);

	QString group = QString("MAME/Configuration/Devices/%1").arg(currentMachineName);
	QString path = qmc2Config->value(group + "/DefaultDeviceDirectory", "").toString();

	if ( path.isEmpty() ) {
		path = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(path + "/" + currentMachineName);
		if ( machineSoftwareFolder.exists() )
			path = machineSoftwareFolder.canonicalPath();
	}

	if ( path.isEmpty() )
		path = QDir::rootPath();

	treeViewDirChooser->setUpdatesEnabled(false);
	dirModel = new DirectoryModel(this);
	dirModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Drives | QDir::CaseSensitive | QDir::Hidden);
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
  	treeViewDirChooser->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/DirChooserHeaderState").toByteArray());
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
  	treeViewFileChooser->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserHeaderState").toByteArray());
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

void DeviceConfigurator::dirChooserDelayedInit()
{
	treeViewDirChooser->scrollTo(treeViewDirChooser->currentIndex(), qmc2CursorPositioningMode);
	treeViewFileChooser->setUpdatesEnabled(true);
	treeViewDirChooser->setUpdatesEnabled(true);
}

void DeviceConfigurator::treeViewDirChooser_selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
	if ( !selected.isEmpty() ) {
		toolButtonChooserPlay->setEnabled(false);
		toolButtonChooserPlayEmbedded->setEnabled(false);
		toolButtonChooserSaveConfiguration->setEnabled(false);
		QString path = dirModel->fileInfo(selected.indexes()[0]).absoluteFilePath();
		fileModel->setCurrentPath(path, false);
		on_toolButtonChooserFilter_toggled(toolButtonChooserFilter->isChecked());
	}
}

void DeviceConfigurator::treeViewFileChooser_selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
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

void DeviceConfigurator::on_toolButtonChooserFilter_toggled(bool enabled)
{
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

void DeviceConfigurator::folderModeMenu_foldersOff()
{
	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-off.png")));
	includeFolders = false;
	foldersFirst = false;
	fileModel->setIncludeFolders(includeFolders);
	fileModel->setFoldersFirst(foldersFirst);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
}

void DeviceConfigurator::folderModeMenu_foldersOn()
{
	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-on.png")));
	includeFolders = true;
	foldersFirst = false;
	fileModel->setIncludeFolders(includeFolders);
	fileModel->setFoldersFirst(foldersFirst);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
}

void DeviceConfigurator::folderModeMenu_foldersFirst()
{
	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-first.png")));
	includeFolders = true;
	foldersFirst = true;
	fileModel->setIncludeFolders(includeFolders);
	fileModel->setFoldersFirst(foldersFirst);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
}

void DeviceConfigurator::on_comboBoxDeviceInstanceChooser_activated(const QString &text)
{
	if ( toolButtonChooserFilter->isChecked() )
		on_toolButtonChooserFilter_toggled(true);
}

void DeviceConfigurator::on_treeViewDirChooser_customContextMenuRequested(const QPoint &p)
{
	modelIndexDirModel = treeViewDirChooser->indexAt(p);
	if ( modelIndexDirModel.isValid() ) {
		dirChooserContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeViewDirChooser->viewport()->mapToGlobal(p), dirChooserContextMenu));
		dirChooserContextMenu->show();
	}
}

void DeviceConfigurator::dirChooserUseCurrentAsDefaultDirectory()
{
	if ( modelIndexDirModel.isValid() ) {
		QString path = dirModel->fileInfo(modelIndexDirModel).absoluteFilePath();
		if ( !path.isEmpty() )
			 qmc2Config->setValue(QString("MAME/Configuration/Devices/%1/DefaultDeviceDirectory").arg(currentMachineName), path);
	}
}

void DeviceConfigurator::on_treeViewFileChooser_customContextMenuRequested(const QPoint &p)
{
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

void DeviceConfigurator::on_treeViewFileChooser_clicked(const QModelIndex &index)
{
	treeViewFileChooser_selectionChanged(QItemSelection(index, index), QItemSelection());
}

void DeviceConfigurator::on_treeViewFileChooser_activated(const QModelIndex &index)
{
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

void DeviceConfigurator::treeViewFileChooser_toggleArchive()
{
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

void DeviceConfigurator::treeViewFileChooser_viewPdf()
{
	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 )
		qmc2MainWindow->viewPdf(fileModel->fileName(selected[0]));
}

void DeviceConfigurator::treeViewFileChooser_viewHtml()
{
	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 )
		qmc2MainWindow->viewHtml(fileModel->fileName(selected[0]));
}

void DeviceConfigurator::treeViewFileChooser_openFileExternally()
{
	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 )
		QDesktopServices::openUrl(QUrl::fromUserInput(fileModel->fileName(selected[0])));
}

void DeviceConfigurator::treeViewFileChooser_openFolder()
{
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

void DeviceConfigurator::treeViewFileChooser_expandRequested()
{
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

void DeviceConfigurator::treeViewDirChooser_headerClicked(int)
{
	dirChooserHeaderState = treeViewDirChooser->header()->saveState();
  	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/DirChooserHeaderState", dirChooserHeaderState);
}

void DeviceConfigurator::treeViewFileChooser_headerClicked(int)
{
	fileChooserHeaderState = treeViewFileChooser->header()->saveState();
  	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserHeaderState", fileChooserHeaderState);
}

void DeviceConfigurator::treeViewFileChooser_sectionMoved(int, int, int)
{
	treeViewFileChooser_headerClicked(0);
}

void DeviceConfigurator::treeViewFileChooser_sectionResized(int, int, int)
{
	treeViewFileChooser_headerClicked(0);
}

void DeviceConfigurator::fileModel_rowsInserted(const QModelIndex &, int start, int end)
{
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

void DeviceConfigurator::fileModel_finished()
{
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

void DeviceConfigurator::on_toolButtonChooserReload_clicked()
{
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

void DeviceConfigurator::on_comboBoxChooserFilterPattern_editTextChanged(const QString &)
{
	searchTimer.start(QMC2_SEARCH_DELAY * 2);
}

void DeviceConfigurator::comboBoxChooserFilterPattern_editTextChanged_delayed()
{
	searchTimer.stop();

	if ( fileModel ) {
		fileModel->setSearchPattern(comboBoxChooserFilterPattern->currentText());
		comboBoxChooserFilterPatternHadFocus = comboBoxChooserFilterPattern->hasFocus();
		on_toolButtonChooserReload_clicked();
	}
}

void DeviceConfigurator::on_toolButtonChooserSaveConfiguration_clicked()
{
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

void DeviceConfigurator::on_splitterFileChooser_splitterMoved(int, int)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FileChooserSplitter", QSize(splitterFileChooser->sizes().at(0), splitterFileChooser->sizes().at(1)));
}

DeviceConfiguratorXmlHandler::DeviceConfiguratorXmlHandler(QTreeWidget *parent, QString machineName)
{
	parentTreeWidget = parent;
	currentMachineName = machineName;
	isCurrentMachine = false;
}

bool DeviceConfiguratorXmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
	if ( qName == "machine" ) {
		isCurrentMachine = (attributes.value("name") == currentMachineName);
		if ( !isCurrentMachine && attributes.value("isdevice") == "yes" )
			newDeviceName = attributes.value("name");
		else
			newDeviceName.clear();
	}
	if ( isCurrentMachine ) {
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
			if ( DeviceConfigurator::systemSlotHash[qmc2DeviceConfigurator->currentMachineName]["QMC2_UNUSED_SLOTS"].contains(slotName) )
				return true;
			allSlots << slotName;
			if ( !DeviceConfigurator::systemSlotHash[qmc2DeviceConfigurator->currentMachineName].contains(slotName) )
				newSlots << slotName;
		} else if ( qName == "slotoption" ) {
			if ( !DeviceConfigurator::systemSlotHash[qmc2DeviceConfigurator->currentMachineName].contains(slotName) ) {
				newSlotOptions[slotName] << attributes.value("name");
				newSlotDevices[attributes.value("name")] = attributes.value("devname");
			}
			slotDeviceNames[attributes.value("name")] = attributes.value("devname");
			if ( attributes.value("default") == "yes" )
				defaultSlotOptions[slotName] = attributes.value("name");
		}
	} else {
		if ( qName == "description" )
			currentText.clear();
	}

	return true;
}

bool DeviceConfiguratorXmlHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName)
{
	if ( isCurrentMachine ) {
		if ( qName == "device" ) {
			foreach (QString instance, deviceInstances) {
				if ( !instance.isEmpty() ) {
					QTreeWidgetItem *deviceItem = new QTreeWidgetItem(parentTreeWidget);
					deviceItem->setText(QMC2_DEVCONFIG_COLUMN_NAME, instance);
					if ( !deviceType.isEmpty() )
						deviceItem->setIcon(QMC2_DEVCONFIG_COLUMN_NAME, DeviceConfigurator::deviceIconHash[deviceType]);
					deviceItem->setText(QMC2_DEVCONFIG_COLUMN_BRIEF, deviceBriefName);
					deviceItem->setText(QMC2_DEVCONFIG_COLUMN_TYPE, deviceType);
					deviceItem->setText(QMC2_DEVCONFIG_COLUMN_TAG, deviceTag);
					deviceItem->setText(QMC2_DEVCONFIG_COLUMN_EXT, deviceExtensions.join("/"));
					parentTreeWidget->openPersistentEditor(deviceItem, QMC2_DEVCONFIG_COLUMN_FILE);
					deviceItem->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, QString());
				}
			}
		}
	} else {
		if ( qName == "description" ) {
			if ( !newDeviceName.isEmpty() )
				newDevices.insert(newDeviceName, currentText);
		}
	}

	return true;
}

bool DeviceConfiguratorXmlHandler::characters(const QString &text)
{
	currentText += text;
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
