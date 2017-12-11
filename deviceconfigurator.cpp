#include <QtGui>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QTextDocument>
#include <QAbstractItemView>
#include <QProcess>
#include <QMap>
#include <QChar>

#include <algorithm> // std::sort()

#include "deviceconfigurator.h"
#include "machinelist.h"
#include "macros.h"
#include "qmc2main.h"
#include "options.h"
#include "fileeditwidget.h"
#include "iconlineedit.h"
#include "fileiconprovider.h"
#include "processmanager.h"

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

QHash<QString, QIcon> DeviceConfigurator::deviceIconHash;
QHash<QString, int> DeviceConfigurator::deviceNameToIndexHash;
QStringList DeviceConfigurator::midiInInterfaces;
QStringList DeviceConfigurator::midiOutInterfaces;
bool DeviceConfigurator::reloadMidiInterfaces = true;

DeviceItemDelegate::DeviceItemDelegate(QObject *parent) :
	QItemDelegate(parent)
{
	// NOP
}

QWidget *DeviceItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
	QModelIndex sibling = index.sibling(index.row(), QMC2_DEVCONFIG_COLUMN_EXT);
	QStringList extensions(sibling.data(Qt::EditRole).toString().split('/', QString::SkipEmptyParts));
	QString filterString(tr("All files") + " (*)");
	if ( extensions.count() > 0 ) {
#if defined(QMC2_OS_WIN)
		filterString = tr("Valid device files") + " (*.zip";
#else
		filterString = tr("Valid device files") + " (*.[zZ][iI][pP]";
#endif
		for (int i = 0; i < extensions.count(); i++) {
			QString ext(extensions.at(i));
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

	QProcess commandProc;
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
		if ( commandProcStarted ) {
			QString buffer(commandProc.readAllStandardOutput());
#if defined(QMC2_OS_WIN)
			buffer.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
			if ( !buffer.isEmpty() ) {
				QStringList lines(buffer.split('\n', QString::SkipEmptyParts));
				QStringList midiInOutMarks(QStringList() << "MIDI input ports:" << "MIDI output ports:");
				bool midiIn = false;
				bool midiOut = false;
				int i = 0;
				while ( i < lines.count() ) {
					QString line(lines.at(i++));
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start emulator executable within a reasonable time frame, giving up") + " (" + tr("error text = %1").arg(ProcessManager::errorText(commandProc.error())) + ")");
}

DeviceConfigurator::DeviceConfigurator(QString machine, QWidget *parent) :
	QWidget(parent),
	m_loadingAnimationOverlay(0),
	m_loadAnimMovie(0),
	m_rootNode(0),
	m_fullyLoaded(false),
	m_fileChooserSetup(false),
	m_foldersFirst(true),
	m_includeFolders(true),
	m_currentMachine(machine),
	m_dirModel(0),
	m_fileModel(0)
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
	configurationRenameItem = 0;
	dontIgnoreNameChange = false;
	updateSlots = true;

	lineEditConfigurationName->blockSignals(true);
	lineEditConfigurationName->setText(tr("Default configuration"));
	comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Enter search string"));
	lineEditConfigurationName->setPlaceholderText(tr("Enter configuration name"));
	lineEditConfigurationName->blockSignals(false);

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
		m_includeFolders = true;
		m_foldersFirst = true;
	} else if ( folderMode == "folders-on" ) {
		toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-on.png")));
		m_includeFolders = true;
		m_foldersFirst = false;
	} else {
		toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-off.png")));
		m_includeFolders = false;
		m_foldersFirst = false;
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
	QAction *action = configurationMenu->addAction(tr("&Default device directory for '%1'...").arg(m_currentMachine));
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
		deviceIconHash.insert("cartridge", QIcon(QString::fromUtf8(":/data/img/dev_cartridge.png")));
		deviceIconHash.insert("cassette", QIcon(QString::fromUtf8(":/data/img/dev_cassette.png")));
		deviceIconHash.insert("cdrom", QIcon(QString::fromUtf8(":/data/img/dev_cdrom.png")));
		deviceIconHash.insert("cylinder", QIcon(QString::fromUtf8(":/data/img/dev_cylinder.png")));
		deviceIconHash.insert("floppydisk", QIcon(QString::fromUtf8(":/data/img/dev_floppydisk.png")));
		deviceIconHash.insert("harddisk", QIcon(QString::fromUtf8(":/data/img/dev_harddisk.png")));
		deviceIconHash.insert("magtape", QIcon(QString::fromUtf8(":/data/img/dev_magtape.png")));
		deviceIconHash.insert("memcard", QIcon(QString::fromUtf8(":/data/img/dev_memcard.png")));
		deviceIconHash.insert("parallel", QIcon(QString::fromUtf8(":/data/img/dev_parallel.png")));
		deviceIconHash.insert("printer", QIcon(QString::fromUtf8(":/data/img/dev_printer.png")));
		deviceIconHash.insert("printout", QIcon(QString::fromUtf8(":/data/img/dev_printer.png")));
		deviceIconHash.insert("punchtape", QIcon(QString::fromUtf8(":/data/img/dev_punchtape.png")));
		deviceIconHash.insert("quickload", QIcon(QString::fromUtf8(":/data/img/dev_quickload.png")));
		deviceIconHash.insert("serial", QIcon(QString::fromUtf8(":/data/img/dev_serial.png")));
		deviceIconHash.insert("snapshot", QIcon(QString::fromUtf8(":/data/img/dev_snapshot.png")));
		deviceIconHash.insert("romimage", QIcon(QString::fromUtf8(":/data/img/dev_rom.png")));
		deviceIconHash.insert("midiin", QIcon(QString::fromUtf8(":/data/img/midi-in.png")));
		deviceIconHash.insert("midiout", QIcon(QString::fromUtf8(":/data/img/midi-out.png")));
		deviceNameToIndexHash.insert("cartridge", QMC2_DEVTYPE_CARTRIDGE);
		deviceNameToIndexHash.insert("cassette", QMC2_DEVTYPE_CASSETTE);
		deviceNameToIndexHash.insert("cdrom", QMC2_DEVTYPE_CDROM);
		deviceNameToIndexHash.insert("cylinder", QMC2_DEVTYPE_CYLINDER);
		deviceNameToIndexHash.insert("floppydisk", QMC2_DEVTYPE_FLOPPYDISK);
		deviceNameToIndexHash.insert("harddisk", QMC2_DEVTYPE_HARDDISK);
		deviceNameToIndexHash.insert("magtape", QMC2_DEVTYPE_MAGTAPE);
		deviceNameToIndexHash.insert("memcard", QMC2_DEVTYPE_MEMCARD);
		deviceNameToIndexHash.insert("parallel", QMC2_DEVTYPE_PARALLEL);
		deviceNameToIndexHash.insert("printer", QMC2_DEVTYPE_PRINTER);
		deviceNameToIndexHash.insert("printout", QMC2_DEVTYPE_PRINTOUT);
		deviceNameToIndexHash.insert("punchtape", QMC2_DEVTYPE_PUNCHTAPE);
		deviceNameToIndexHash.insert("quickload", QMC2_DEVTYPE_QUICKLOAD);
		deviceNameToIndexHash.insert("serial", QMC2_DEVTYPE_SERIAL);
		deviceNameToIndexHash.insert("snapshot", QMC2_DEVTYPE_SNAPSHOT);
		deviceNameToIndexHash.insert("romimage", QMC2_DEVTYPE_ROMIMAGE);
		deviceNameToIndexHash.insert("midiin", QMC2_DEVTYPE_MIDIIN);
		deviceNameToIndexHash.insert("midiout", QMC2_DEVTYPE_MIDIOUT);
	}

	FileChooserKeyEventFilter *eventFilter = new FileChooserKeyEventFilter(this);
	treeViewFileChooser->installEventFilter(eventFilter);
	connect(eventFilter, SIGNAL(expandRequested()), this, SLOT(treeViewFileChooser_expandRequested()));
}

DeviceConfigurator::~DeviceConfigurator()
{
	if ( m_loadingAnimationOverlay ) {
		delete m_loadingAnimationOverlay;
		delete m_loadAnimMovie;
	}
	if ( m_rootNode )
		delete m_rootNode;
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
	if ( m_includeFolders && m_foldersFirst )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FolderMode", "folders-first");
	else if ( m_includeFolders )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FolderMode", "folders-on");
	else
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DeviceConfigurator/FolderMode", "folders-off");
}

QString DeviceConfigurator::getXmlData(const QString &machine)
{
	QString buffer("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + qmc2MachineList->xmlDb()->xml(machine));
	return buffer;
}

void DeviceConfigurator::slotOptionChanged(int index)
{
	QTimer::singleShot(QMC2_SLOTOPTION_CHANGE_DELAY, this, SLOT(refreshDeviceMap()));
}

void DeviceConfigurator::insertChildItems(QTreeWidgetItem *parentItem, QList<QTreeWidgetItem *> &itemList)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		itemList << parentItem->child(i);
		insertChildItems(parentItem->child(i), itemList);
	}
}

void DeviceConfigurator::updateDeviceTree(DeviceTreeNode *node, const QString &machine)
{
	QString xmlBuffer(getXmlData(machine));
	QXmlInputSource xmlInputSource;
	xmlInputSource.setData(xmlBuffer);
	DeviceTreeXmlHandler xmlHandler(node);
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(&xmlHandler);
	if ( !xmlReader.parse(xmlInputSource) )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for '%1'").arg(machine));
}

bool DeviceConfigurator::refreshDeviceMap()
{
	if ( m_rootNode )
		delete m_rootNode;
	m_rootNode = new DeviceTreeNode(0, QString());
	updateDeviceTree(m_rootNode, m_currentMachine);

	treeWidgetSlotOptions->clear();
	foreach (DeviceTreeNode *child, m_rootNode->children())
		traverseDeviceTree(0, child);
	treeWidgetSlotOptions->sortItems(QMC2_SLOTCONFIG_COLUMN_SLOT, Qt::AscendingOrder);
	bool isDev = qmc2MachineList->isDevice(m_currentMachine);
	treeWidgetSlotOptions->setEnabled(isDev ? false : treeWidgetSlotOptions->topLevelItemCount() > 0);
	updateDeviceMappings();
	if ( !isDev )
		if ( comboBoxDeviceInstanceChooser->count() > 0 && comboBoxDeviceInstanceChooser->currentText() != tr("No devices available") )
 			QTimer::singleShot(0, this, SLOT(setupFileChooser()));

	return true;
}

void DeviceConfigurator::traverseDeviceTree(QTreeWidgetItem *parentItem, DeviceTreeNode *node)
{
	QTreeWidgetItem *item = parentItem;
	if ( !node->options().isEmpty() ) {
		if ( parentItem != 0 ) {
			item = new QTreeWidgetItem(parentItem);
			parentItem->setExpanded(true);
		} else
			item = new QTreeWidgetItem(treeWidgetSlotOptions);
		item->setText(QMC2_SLOTCONFIG_COLUMN_SLOT, node->fullName());
		item->setIcon(QMC2_SLOTCONFIG_COLUMN_SLOT, QIcon(QString::fromUtf8(":/data/img/slot.png")));

		// slot options
		QComboBox *cbOption = new QComboBox(treeWidgetSlotOptions);
		cbOption->view()->setTextElideMode(Qt::ElideMiddle);
		int defaultIndex = -1;
		if ( node->defaultOption().isEmpty() ) {
			cbOption->insertItem(0, tr("not used") + " / " + tr("default"), (qulonglong)item);
			defaultIndex = 0;
		} else
			cbOption->insertItem(0, tr("not used"), (qulonglong)item);
		cbOption->insertSeparator(1);
		QMap<QString, QString> sortedOptions;
		for (int i = 0; i < node->options().count(); i++)
			sortedOptions.insert(node->options().at(i), node->optionDescriptions().at(i));
		QMapIterator<QString, QString> it(sortedOptions);
		int index = 2;
		while ( it.hasNext() ) {
			it.next();
			if ( it.key() == node->defaultOption() ) {
				cbOption->insertItem(index, it.key() + " - " + it.value() + " / " + tr("default"), (qulonglong)item);
				defaultIndex = index;
				// FIXME
				DeviceTreeNode *childNode = m_rootNode->findNode(m_rootNode, node->fullName() + ':' + it.key());
				if ( childNode == 0 ) {
					childNode = new DeviceTreeNode(node, it.key());
					node->addChild(childNode);
					int optIndex = node->options().indexOf(it.key());
					if ( optIndex >= 0 ) {
						updateDeviceTree(childNode, node->optionDevices().at(optIndex));
						traverseDeviceTree(item, childNode);
					}
				}
			} else
				cbOption->insertItem(index, it.key() + " - " + it.value(), (qulonglong)item);
			index++;
		}
		treeWidgetSlotOptions->setItemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION, cbOption);
		cbOption->setCurrentIndex(defaultIndex);
		connect(cbOption, SIGNAL(currentIndexChanged(int)), this, SLOT(optionComboBox_currentIndexChanged(int)));

		// BIOS options
		QComboBox *cbBios = new QComboBox(treeWidgetSlotOptions);
		cbBios->view()->setTextElideMode(Qt::ElideMiddle);
		QString currentOption(cbOption->currentText());
		if ( currentOption.startsWith(tr("not used")) ) {
			cbBios->insertItem(0, tr("N/A"));
			cbBios->setCurrentIndex(0);
			cbBios->setEnabled(false);
		} else {
			currentOption.replace(QRegExp(" - .*"), QString());
			if ( node->optionBioses(currentOption).isEmpty() ) {
				cbBios->insertItem(0, tr("N/A"));
				cbBios->setCurrentIndex(0);
				cbBios->setEnabled(false);
			} else {
				defaultIndex = -1;
				if ( node->defaultOptionBios(currentOption).isEmpty() ) {
					cbBios->insertItem(0, tr("not used") + " / " + tr("default"), (qulonglong)item);
					defaultIndex = 0;
				} else
					cbBios->insertItem(0, tr("not used"), (qulonglong)item);
				cbBios->insertSeparator(1);
				QMap<QString, QString> sortedBiosOptions;
				for (int i = 0; i < node->optionBioses(currentOption).count(); i++)
					sortedBiosOptions.insert(node->optionBioses(currentOption).at(i), node->optionBiosDescriptions(currentOption).at(i));
				QMapIterator<QString, QString> it(sortedBiosOptions);
				index = 2;
				while ( it.hasNext() ) {
					it.next();
					if ( it.key() == node->defaultOptionBios(currentOption) ) {
						cbBios->insertItem(index, it.key() + " - " + it.value() + " / " + tr("default"), (qulonglong)item);
						defaultIndex = index;
					} else
						cbBios->insertItem(index, it.key() + " - " + it.value(), (qulonglong)item);
					index++;
				}
				cbBios->setCurrentIndex(defaultIndex);
			}
		}
		treeWidgetSlotOptions->setItemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS, cbBios);
	}

	// recursion
	foreach (DeviceTreeNode *child, node->children())
		traverseDeviceTree(item, child);
}

void DeviceConfigurator::makeUnique(QStringList *devNames, QStringList *devBriefNames)
{
	QRegExp removeDigitsRx("\\d+$");
	QHash<QString, int> devNameCounts;
	QHash<QString, int> devBriefNameCounts;
	for (int i = 0; i < devNames->count(); i++) {
		QString s1(devNames->at(i));
		s1.remove(removeDigitsRx);
		devNames->replace(i, s1);
		devNameCounts[devNames->at(i)]++;
		QString s2(devBriefNames->at(i));
		s2.remove(removeDigitsRx);
		devBriefNames->replace(i, s2);
		devBriefNameCounts[devBriefNames->at(i)]++;
	}
	QHash<QString, int> devNameIndex;
	QHash<QString, int> devBriefNameIndex;
	for (int i = 0; i < devNames->count(); i++) {
		if ( devNameCounts.value(devNames->at(i)) > 1 ) {
			devNameIndex[devNames->at(i)]++;
			QString s(devNames->at(i));
			s.append(QString::number(devNameIndex.value(devNames->at(i))));
			devNames->replace(i, s);
		}
		if ( devBriefNameCounts.value(devBriefNames->at(i)) > 1 ) {
			devBriefNameIndex[devBriefNames->at(i)]++;
			QString s(devBriefNames->at(i));
			s.append(QString::number(devBriefNameIndex.value(devBriefNames->at(i))));
			devBriefNames->replace(i, s);
		}
	}
}

void DeviceConfigurator::updateDeviceMappings()
{
	QStringList devices(m_rootNode->allDevices());
	QStringList deviceBriefNames(m_rootNode->allDeviceBriefNames());
	QStringList deviceTypes(m_rootNode->allDeviceTypes());
	QStringList deviceTags(m_rootNode->allDeviceTags());
	QStringList deviceExtensions(m_rootNode->allDeviceExtensions());
	QStringList seenTags;
	QList<int> indexesToBeRemoved;
	for (int i = deviceTags.count() - 1; i >= 0; i--) {
		if ( seenTags.contains(deviceTags.at(i)) )
			indexesToBeRemoved.append(i);
		else
			seenTags.append(deviceTags.at(i));
	}
	foreach (int index, indexesToBeRemoved) {
		devices.removeAt(index);
		deviceBriefNames.removeAt(index);
		deviceTypes.removeAt(index);
		deviceTags.removeAt(index);
		deviceExtensions.removeAt(index);
	}
	makeUnique(&devices, &deviceBriefNames);

	// FIXME: debug start
	if ( !devices.isEmpty() ) {
		QMC2_PRINT_TXT(-----);
		QMC2_PRINT_STRLST(devices);
	}
	// FIXME: debug end

	treeWidgetDeviceSetup->clear();
	comboBoxDeviceInstanceChooser->clear();
	for (int i = 0; i < devices.count(); i++) {
		QTreeWidgetItem *deviceItem = new QTreeWidgetItem();
		deviceItem->setText(QMC2_DEVCONFIG_COLUMN_NAME, devices.at(i));
		if ( !deviceTypes.at(i).isEmpty() )
			deviceItem->setIcon(QMC2_DEVCONFIG_COLUMN_NAME, DeviceConfigurator::deviceIconHash.value(deviceTypes.at(i)));
		deviceItem->setText(QMC2_DEVCONFIG_COLUMN_BRIEF, deviceBriefNames.at(i));
		deviceItem->setText(QMC2_DEVCONFIG_COLUMN_TYPE, deviceTypes.at(i));
		deviceItem->setText(QMC2_DEVCONFIG_COLUMN_EXT, deviceExtensions.at(i));
		deviceItem->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, QString());
		treeWidgetDeviceSetup->insertTopLevelItem(i, deviceItem);
		treeWidgetDeviceSetup->openPersistentEditor(deviceItem, QMC2_DEVCONFIG_COLUMN_FILE);
	}
	treeWidgetDeviceSetup->sortItems(QMC2_DEVCONFIG_COLUMN_NAME, Qt::AscendingOrder);
	devices.sort();
	comboBoxDeviceInstanceChooser->insertItems(0, devices);
	if ( treeWidgetDeviceSetup->topLevelItemCount() > 0 ) {
		bool isDev = qmc2MachineList->isDevice(m_currentMachine);
		treeWidgetDeviceSetup->setEnabled(!isDev);
		tabWidgetDeviceSetup->widget(QMC2_DEVSETUP_TAB_FILECHOOSER)->setEnabled(!isDev);
	} else {
		treeWidgetDeviceSetup->setEnabled(false);
		comboBoxDeviceInstanceChooser->insertItem(0, tr("No devices available"));
		comboBoxDeviceInstanceChooser->setCurrentIndex(0);
		tabWidgetDeviceSetup->widget(QMC2_DEVSETUP_TAB_FILECHOOSER)->setEnabled(false);
	}
}

void DeviceConfigurator::optionComboBox_currentIndexChanged(int index)
{
	QComboBox *cb = (QComboBox *)sender();
	QString currentOption(cb->itemText(index));
	currentOption.replace(QRegExp(" - .*"), QString());
	QTreeWidgetItem *item = (QTreeWidgetItem *)cb->itemData(index).toULongLong();
	QString currentSlot(item->text(QMC2_SLOTCONFIG_COLUMN_SLOT));
	DeviceTreeNode *parentNode = m_rootNode->findNode(m_rootNode, currentSlot);
	if ( parentNode ) {
		for (int i = item->childCount(); i >= 0; i--) {
			QTreeWidgetItem *it = item->child(i);
			item->removeChild(it);
			delete it;
		}
		qDeleteAll(parentNode->children());
		parentNode->children().clear();
		QComboBox *cbBios = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
		if ( !currentOption.startsWith(tr("not used")) ) {
			DeviceTreeNode *childNode = new DeviceTreeNode(parentNode, currentOption);
			parentNode->addChild(childNode);
			int index = parentNode->options().indexOf(currentOption);
			if ( index >= 0 ) {
				updateDeviceTree(childNode, parentNode->optionDevices().at(index));
				traverseDeviceTree(item, childNode);
			}
			cbBios->clear();
			if ( parentNode->optionBioses(currentOption).isEmpty() ) {
				cbBios->insertItem(0, tr("N/A"));
				cbBios->setCurrentIndex(0);
				cbBios->setEnabled(false);
			} else {
				int defaultIndex = -1;
				if ( parentNode->defaultOptionBios(currentOption).isEmpty() ) {
					cbBios->insertItem(0, tr("not used") + " / " + tr("default"), (qulonglong)item);
					defaultIndex = 0;
				} else
					cbBios->insertItem(0, tr("not used"), (qulonglong)item);
				cbBios->insertSeparator(1);
				QMap<QString, QString> sortedBiosOptions;
				for (int i = 0; i < parentNode->optionBioses(currentOption).count(); i++)
					sortedBiosOptions.insert(parentNode->optionBioses(currentOption).at(i), parentNode->optionBiosDescriptions(currentOption).at(i));
				QMapIterator<QString, QString> it(sortedBiosOptions);
				int i = 2;
				while ( it.hasNext() ) {
					it.next();
					if ( it.key() == parentNode->defaultOptionBios(currentOption) ) {
						cbBios->insertItem(i, it.key() + " - " + it.value() + " / " + tr("default"), (qulonglong)item);
						defaultIndex = i;
					} else
						cbBios->insertItem(i, it.key() + " - " + it.value(), (qulonglong)item);
					i++;
				}
				cbBios->setCurrentIndex(defaultIndex);
				cbBios->setEnabled(true);
			}
		} else {
			cbBios->clear();
			cbBios->insertItem(0, tr("N/A"));
			cbBios->setCurrentIndex(0);
			cbBios->setEnabled(false);
		}
		updateDeviceMappings();
	}
	treeWidgetSlotOptions->sortItems(QMC2_SLOTCONFIG_COLUMN_SLOT, Qt::AscendingOrder);
	cb->setFocus();
}

bool DeviceConfigurator::load()
{
	m_fullyLoaded = false;
	refreshDeviceMap();
	// FIXME
	m_fullyLoaded = true;
	return true;
}

bool DeviceConfigurator::save()
{
	if ( !m_fullyLoaded )
		return false;

	QString group(QString("MAME/Configuration/Devices/%1").arg(m_currentMachine));
	QString devDir(qmc2Config->value(QString("%1/DefaultDeviceDirectory").arg(group), "").toString());

	qmc2Config->remove(group);

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

	if ( !devDir.isEmpty() )
		qmc2Config->setValue(group + "/DefaultDeviceDirectory", devDir);

	QListWidgetItem *curItem = listWidgetDeviceConfigurations->currentItem();
	if ( curItem != 0 ) {
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
	QString cfgName(lineEditConfigurationName->text());
	if ( cfgName.isEmpty() )
		return;
	QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(cfgName, Qt::MatchExactly);
	treeWidgetDeviceSetup->setUpdatesEnabled(false);
	if ( matchedItemList.count() <= 0 ) {
		// add new device configuration
		listWidgetDeviceConfigurations->setSortingEnabled(false);
		int row = listWidgetDeviceConfigurations->count();
		listWidgetDeviceConfigurations->insertItem(row, cfgName);
		listWidgetDeviceConfigurations->item(row)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		listWidgetDeviceConfigurations->setSortingEnabled(true);
		listWidgetDeviceConfigurations->sortItems(Qt::AscendingOrder);
		dontIgnoreNameChange = true;
	}
	// save device configuration
	QStringList instances, files;
	for (int i = 0; i < treeWidgetDeviceSetup->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidgetDeviceSetup->topLevelItem(i);
		QString fileName(item->data(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole).toString());
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
	QChar splitChar(' ');
	foreach (QTreeWidgetItem *item, allSlotItems) {
		QString slotName(item->text(QMC2_SLOTCONFIG_COLUMN_SLOT));
		if ( !slotName.isEmpty() ) {
			QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
			if ( cb ) {
				int defaultIndex = -1;
				if ( slotPreselectionMap.contains(cb) )
					defaultIndex = slotPreselectionMap.value(cb);
				else if ( nestedSlotPreselectionMap.contains(cb) )
					defaultIndex = nestedSlotPreselectionMap.value(cb);
				QComboBox *cbBIOS = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
				if ( cb->currentIndex() > 0 && defaultIndex == 0 ) {
					slotNames << slotName;
					slotOptions << cb->currentText().split(splitChar).at(0);
					if ( cbBIOS ) {
						QString biosChoice(cbBIOS->currentText().split(splitChar, QString::SkipEmptyParts).at(0));
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
					slotOptions << cb->currentText().split(splitChar).at(0);
					if ( cbBIOS ) {
						QString biosChoice(cbBIOS->currentText().split(splitChar, QString::SkipEmptyParts).at(0));
						if ( biosChoice == tr("N/A") )
							biosChoice.clear();
						slotBIOSs << biosChoice;
					} else
						slotBIOSs << QString();
				} else {
					slotNames << slotName;
					if ( cbBIOS ) {
						bool isDefaultBiosChoice = cbBIOS->currentText().endsWith(" / " + tr("default"));
						QString biosChoice(cbBIOS->currentText().split(splitChar, QString::SkipEmptyParts).at(0));
						if ( biosChoice == tr("N/A") )
							biosChoice.clear();
						slotBIOSs << biosChoice;
						if ( !biosChoice.isEmpty() && !isDefaultBiosChoice )
							slotOptions << cb->currentText().split(splitChar).at(0);
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
	treeWidgetDeviceSetup->setUpdatesEnabled(true);
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
		int row = listWidgetDeviceConfigurations->row(matchedItemList.first());
		QListWidgetItem *prevItem = 0;
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
	configurationRenameItem = 0;
	QList<QListWidgetItem *> sl = listWidgetDeviceConfigurations->selectedItems();
	if ( sl.count() > 0 ) {
		configurationRenameItem = sl.first();
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
	configurationRenameItem = 0;
	oldConfigurationName.clear();
}

void DeviceConfigurator::actionRemoveConfiguration_activated()
{
	QList<QListWidgetItem *> sl = listWidgetDeviceConfigurations->selectedItems();

	if ( sl.count() > 0 ) {
		QListWidgetItem *item = sl.first();
		configurationMap.remove(item->text());
		slotMap.remove(item->text());
		slotBiosMap.remove(item->text());
		int row = listWidgetDeviceConfigurations->row(item);
		QListWidgetItem *prevItem = 0;
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
	// FIXME
	return;

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
			listWidgetDeviceConfigurations->setCurrentItem(matchedItemList.first());
			listWidgetDeviceConfigurations->scrollToItem(matchedItemList.first());
			QString configName = matchedItemList.first()->text();
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
							index = slotPreselectionMap.value(cb);
						else if ( nestedSlotPreselectionMap.contains(cb) )
							index = nestedSlotPreselectionMap.value(cb);
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
								/*
								bool isNestedSlot = !systemSlotHash.value(m_currentMachine).contains(valuePair.first[i]);
								if ( valuePair.second[i] != "\"\"" ) {
									if ( isNestedSlot )
										index = cb->findText(QString("%1 - %2").arg(valuePair.second[i]).arg(nestedSlotOptionMap[valuePair.first[i]][valuePair.second[i]]));
									else
										index = cb->findText(QString("%1 - %2").arg(valuePair.second[i]).arg(slotNameHash[valuePair.second[i]]));
								} else
									index = 0;
								*/

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
						QTreeWidgetItem *item = itemList.first();
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
				listWidgetDeviceConfigurations->setCurrentItem(matchedItemList.first());
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
	QString group = QString("MAME/Configuration/Devices/%1").arg(m_currentMachine);
	QString path = qmc2Config->value(group + "/DefaultDeviceDirectory", "").toString();

	if ( path.isEmpty() ) {
		path = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(path + "/" + m_currentMachine);
		if ( machineSoftwareFolder.exists() )
			path = machineSoftwareFolder.canonicalPath();
	}

	qmc2Config->beginGroup(group);

	QString s(QFileDialog::getExistingDirectory(this, tr("Choose default device directory for '%1'").arg(m_currentMachine), path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isEmpty() )
		qmc2Config->setValue("DefaultDeviceDirectory", s);
	qmc2FileEditStartPath = qmc2Config->value("DefaultDeviceDirectory").toString();

	qmc2Config->endGroup();

	if ( qmc2FileEditStartPath.isEmpty() ) {
		qmc2FileEditStartPath = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(qmc2FileEditStartPath + "/" + m_currentMachine);
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

void DeviceConfigurator::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
	if ( m_loadingAnimationOverlay ) {
		m_loadingAnimationOverlay->resize(size());
		if ( m_loadingAnimationOverlay->movie() )
			m_loadingAnimationOverlay->adjustMovieSize();
	}
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
			if ( !qmc2MachineList->isDevice(m_currentMachine) )
				if ( comboBoxDeviceInstanceChooser->count() > 0 && comboBoxDeviceInstanceChooser->currentText() != tr("No devices available") )
		  			QTimer::singleShot(0, this, SLOT(setupFileChooser()));
			break;
		default:
			break;
	}
}

void DeviceConfigurator::setupFileChooser()
{
	QString group(QString("MAME/Configuration/Devices/%1").arg(currentMachine()));
	QString path(qmc2Config->value(group + "/DefaultDeviceDirectory", QString()).toString());
	if ( path.isEmpty() ) {
		path = qmc2Config->value("MAME/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(path + "/" + m_currentMachine);
		if ( machineSoftwareFolder.exists() )
			path = machineSoftwareFolder.canonicalPath();
	}

	if ( m_fileChooserSetup ) {
		if ( path != fileModel()->currentPath() ) {
			treeViewDirChooser->setCurrentIndex(m_dirModel->index(path));
			fileModel()->setCurrentPath(path, false);
			QTimer::singleShot(0, fileModel(), SLOT(refresh()));
		}
		return;
	}
	m_fileChooserSetup = true;

	toolButtonChooserPlay->setEnabled(false);
	toolButtonChooserPlayEmbedded->setEnabled(false);
	toolButtonChooserSaveConfiguration->setEnabled(false);

	if ( path.isEmpty() )
		path = QDir::rootPath();

	treeViewDirChooser->setUpdatesEnabled(false);
	m_dirModel = new DirectoryModel(this);
	m_dirModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Drives | QDir::CaseSensitive | QDir::Hidden);
#if defined(QMC2_OS_WIN)
	m_dirModel->setRootPath(m_dirModel->myComputer().toString());
#else
	m_dirModel->setRootPath("/");
#endif
	treeViewDirChooser->setModel(m_dirModel);
	treeViewDirChooser->setCurrentIndex(m_dirModel->index(path));
	for (int i = treeViewDirChooser->header()->count(); i > 0; i--)
		treeViewDirChooser->setColumnHidden(i, true);
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
	lcdNumberFileCounter->update();
	m_fileModel = new FileSystemModel(this);
	m_fileModel->setIncludeFolders(m_includeFolders);
	m_fileModel->setFoldersFirst(m_foldersFirst);
	m_fileModel->setCurrentPath(path, false);
	connect(m_fileModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(fileModel_rowsInserted(const QModelIndex &, int, int)));
	connect(m_fileModel, SIGNAL(finished()), this, SLOT(fileModel_finished()));
	treeViewFileChooser->setUpdatesEnabled(false);
	treeViewFileChooser->setModel(m_fileModel);
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
		QString path(m_dirModel->fileInfo(selected.indexes().first()).absoluteFilePath());
		m_fileModel->setCurrentPath(path, false);
		on_toolButtonChooserFilter_toggled(toolButtonChooserFilter->isChecked());
	}
}

void DeviceConfigurator::treeViewFileChooser_selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
	if ( selected.indexes().count() > 0 ) {
		toolButtonChooserPlay->setEnabled(true);
		toolButtonChooserPlayEmbedded->setEnabled(true);
		toolButtonChooserSaveConfiguration->setEnabled(!m_fileModel->isFolder(selected.indexes().first()));
		if ( toolButtonChooserAutoSelect->isChecked() ) {
			QFileInfo fi(m_fileModel->absolutePath(selected.indexes().first()));
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
		QStringList extensions = items.first()->text(QMC2_DEVCONFIG_COLUMN_EXT).split("/", QString::SkipEmptyParts);
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
		m_fileModel->setNameFilters(extensions);
	} else
		m_fileModel->setNameFilters(QStringList());

	fileModelRowInsertionCounter = 0;
	lcdNumberFileCounter->display(0);
	lcdNumberFileCounter->update();
	treeViewFileChooser->selectionModel()->clearSelection();
	treeViewFileChooser->selectionModel()->reset();
	treeViewFileChooser->setUpdatesEnabled(false);
	toolButtonChooserReload->setEnabled(false);
	QTimer::singleShot(0, m_fileModel, SLOT(refresh()));
}

void DeviceConfigurator::folderModeMenu_foldersOff()
{
	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-off.png")));
	m_includeFolders = false;
	m_foldersFirst = false;
	m_fileModel->setIncludeFolders(m_includeFolders);
	m_fileModel->setFoldersFirst(m_foldersFirst);
	QTimer::singleShot(0, m_fileModel, SLOT(refresh()));
}

void DeviceConfigurator::folderModeMenu_foldersOn()
{
	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-on.png")));
	m_includeFolders = true;
	m_foldersFirst = false;
	m_fileModel->setIncludeFolders(m_includeFolders);
	m_fileModel->setFoldersFirst(m_foldersFirst);
	QTimer::singleShot(0, m_fileModel, SLOT(refresh()));
}

void DeviceConfigurator::folderModeMenu_foldersFirst()
{
	toolButtonFolderMode->setIcon(QIcon(QString::fromUtf8(":/data/img/folders-first.png")));
	m_includeFolders = true;
	m_foldersFirst = true;
	m_fileModel->setIncludeFolders(m_includeFolders);
	m_fileModel->setFoldersFirst(m_foldersFirst);
	QTimer::singleShot(0, m_fileModel, SLOT(refresh()));
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
		QString path(m_dirModel->fileInfo(modelIndexDirModel).absoluteFilePath());
		if ( !path.isEmpty() )
			 qmc2Config->setValue(QString("MAME/Configuration/Devices/%1/DefaultDeviceDirectory").arg(m_currentMachine), path);
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
		actionChooserViewPdf->setVisible(m_fileModel->isPdf(modelIndexFileModel));
		actionChooserViewPostscript->setVisible(m_fileModel->isPostscript(modelIndexFileModel));
		actionChooserViewHtml->setVisible(m_fileModel->isHtml(modelIndexFileModel));
		if ( m_fileModel->isZip(modelIndexFileModel) ) {
			actionChooserToggleArchive->setText(treeViewFileChooser->isExpanded(modelIndexFileModel) ? tr("&Close archive") : tr("&Open archive"));
			actionChooserToggleArchive->setVisible(true);
		} else
			actionChooserToggleArchive->setVisible(false);
		if ( m_fileModel->isFolder(modelIndexFileModel) ) {
			actionChooserPlay->setVisible(false);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			actionChooserPlayEmbedded->setVisible(false);
#endif
			actionChooserOpenFolder->setVisible(true);
			actionChooserOpenExternally->setVisible(false);
		} else {
			actionChooserOpenFolder->setVisible(false);
			actionChooserOpenExternally->setVisible(!m_fileModel->isZipContent(modelIndexFileModel));
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
	if ( toolButtonChooserProcessZIPs->isChecked() && m_fileModel->isZip(index) ) {
		if ( treeViewFileChooser->isExpanded(index) ) {
			treeViewFileChooser->setExpanded(index, false);
		} else {
			treeViewFileChooser->setExpanded(index, true);
			m_fileModel->openZip(index);
			m_fileModel->sortOpenZip(index, treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
		}
	} else if ( m_fileModel->isFolder(index) ) {
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
	QModelIndexList selected(treeViewFileChooser->selectionModel()->selectedIndexes());
	if ( selected.count() > 0 ) {
		QModelIndex index(selected.first());
		if ( treeViewFileChooser->isExpanded(index) ) {
			treeViewFileChooser->setExpanded(index, false);
		} else {
			treeViewFileChooser->setExpanded(index, true);
			m_fileModel->openZip(index);
			m_fileModel->sortOpenZip(index, treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
		}
	}
}

void DeviceConfigurator::treeViewFileChooser_viewPdf()
{
	QModelIndexList selected(treeViewFileChooser->selectionModel()->selectedIndexes());
	if ( selected.count() > 0 )
		qmc2MainWindow->viewPdf(m_fileModel->fileName(selected.first()));
}

void DeviceConfigurator::treeViewFileChooser_viewHtml()
{
	QModelIndexList selected(treeViewFileChooser->selectionModel()->selectedIndexes());
	if ( selected.count() > 0 )
		qmc2MainWindow->viewHtml(m_fileModel->fileName(selected.first()));
}

void DeviceConfigurator::treeViewFileChooser_openFileExternally()
{
	QModelIndexList selected(treeViewFileChooser->selectionModel()->selectedIndexes());
	if ( selected.count() > 0 )
		QDesktopServices::openUrl(QUrl::fromUserInput(m_fileModel->fileName(selected.first())));
}

void DeviceConfigurator::treeViewFileChooser_openFolder()
{
	QModelIndexList selected(treeViewFileChooser->selectionModel()->selectedIndexes());
	if ( selected.count() > 0 ) {
		QModelIndex index(selected.first());
		QString folderPath(m_fileModel->absolutePath(index));
		QModelIndex dirIndex(m_dirModel->index(folderPath));
		if ( dirIndex.isValid() )
			treeViewDirChooser->setCurrentIndex(dirIndex);
		else
			treeViewDirChooser->setCurrentIndex(m_dirModel->index(m_dirModel->rootPath()));
	}
}

void DeviceConfigurator::treeViewFileChooser_expandRequested()
{
	QModelIndexList selected(treeViewFileChooser->selectionModel()->selectedIndexes());
	if ( selected.count() > 0 ) {
		QModelIndex index(selected.first());
		if ( !treeViewFileChooser->isExpanded(index) ) {
			if ( toolButtonChooserProcessZIPs->isChecked() && m_fileModel->isZip(index) ) {
				treeViewFileChooser->setExpanded(index, true);
				m_fileModel->openZip(index);
				m_fileModel->sortOpenZip(index, treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
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
	lcdNumberFileCounter->display(m_fileModel->rowCount());
	lcdNumberFileCounter->update();
}

void DeviceConfigurator::fileModel_finished()
{
	lcdNumberFileCounter->display(m_fileModel->rowCount());
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
	lcdNumberFileCounter->update();
	treeViewFileChooser->selectionModel()->clearSelection();
	treeViewFileChooser->selectionModel()->reset();
	treeViewFileChooser->setUpdatesEnabled(false);
	toolButtonChooserReload->setEnabled(false);
	QTimer::singleShot(0, m_fileModel, SLOT(refresh()));
}

void DeviceConfigurator::on_comboBoxChooserFilterPattern_editTextChanged(const QString &)
{
	searchTimer.start(QMC2_SEARCH_DELAY * 2);
}

void DeviceConfigurator::comboBoxChooserFilterPattern_editTextChanged_delayed()
{
	searchTimer.stop();

	if ( m_fileModel ) {
		m_fileModel->setSearchPattern(comboBoxChooserFilterPattern->currentText());
		comboBoxChooserFilterPatternHadFocus = comboBoxChooserFilterPattern->hasFocus();
		on_toolButtonChooserReload_clicked();
	}
}

void DeviceConfigurator::on_toolButtonChooserSaveConfiguration_clicked()
{
	QString instance = comboBoxDeviceInstanceChooser->currentText();
	QModelIndexList indexList = treeViewFileChooser->selectionModel()->selectedIndexes();
	if ( indexList.count() > 0 && instance != tr("No devices available") ) {
		QString file(m_fileModel->absolutePath(indexList.first()));
		QString targetName;
		bool goOn = false;
		do {
			QFileInfo fi(file);
			QString sourceName(fi.completeBaseName());
			targetName = sourceName;
			int copies = 0;
			while ( configurationMap.contains(targetName) )
				targetName = tr("%1. variant of ").arg(++copies) + sourceName;
			bool ok;
			QString text(QInputDialog::getText(this, tr("Choose a unique configuration name"), tr("Unique configuration name:"), QLineEdit::Normal, targetName, &ok));
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
					QChar splitChar(' ');
					foreach (QTreeWidgetItem *item, allSlotItems) {
						QString slotName = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT);
						if ( !slotName.isEmpty() ) {
							QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
							if ( cb ) {
								int defaultIndex = -1;
								if ( slotPreselectionMap.contains(cb) )
									defaultIndex = slotPreselectionMap.value(cb);
								else if ( nestedSlotPreselectionMap.contains(cb) )
									defaultIndex = nestedSlotPreselectionMap.value(cb);
								QComboBox *cbBIOS = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
								if ( cbBIOS ) {
									QString biosChoice = cbBIOS->currentText().split(" ", QString::SkipEmptyParts)[0];
									if ( biosChoice == tr("N/A") )
										biosChoice.clear();
									slotBIOSs << biosChoice;
								}
								if ( cb->currentIndex() > 0 && defaultIndex == 0 ) {
									slotNames << slotName;
									slotOptions << cb->currentText().split(splitChar)[0];
								} else if (cb->currentIndex() == 0 && defaultIndex > 0 ) {
									slotNames << slotName;
									slotOptions << "\"\"";
								} else if ( cb->currentIndex() > 0 && defaultIndex > 0 && cb->currentIndex() != defaultIndex ) {
									slotNames << slotName;
									slotOptions << cb->currentText().split(splitChar)[0];
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

bool DeviceTreeXmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
	if ( qName == "slot" ) {
		m_currentSlot = attributes.value("name");
	} else if ( qName == "slotoption" ) {
		m_slotOptions.append(attributes.value("name"));
		m_slotOptionDevices.append(attributes.value("devname"));
		if ( attributes.value("default") == "yes" )
			m_defaultOption = attributes.value("name");
	} else if ( qName == "device" ) {
		m_currentDeviceType = attributes.value("type");
		m_currentDeviceTag = attributes.value("tag");
		if ( m_currentDeviceTag.startsWith(':') )
			m_currentDeviceTag.prepend(m_devNode->fullName());
		m_currentDeviceInterface = attributes.value("interface");
	} else if ( qName == "instance" ) {
		m_currentDevice = attributes.value("name");
		m_currentDeviceBriefName = attributes.value("briefname");
	} else if ( qName == "extension" ) {
		m_currentDeviceExtensions.append(attributes.value("name"));
	}
	return true;
}

bool DeviceTreeXmlHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName)
{
	bool rc = true;
	if ( qName == "slot" ) {
		if ( !m_slotOptions.isEmpty() ) {
			DeviceTreeNode *childNode = new DeviceTreeNode(m_devNode, m_currentSlot);
			for (int i = 0; i < m_slotOptions.count(); i++) {
				childNode->addOption(m_slotOptions.at(i), m_slotOptionDevices.at(i), lookupDescription(m_slotOptionDevices.at(i)));
				QStringList optionBioses;
				QStringList optionBiosDescriptions;
				QString defaultBiosOption(lookupBiosOptions(m_slotOptionDevices.at(i), &optionBioses, &optionBiosDescriptions));
				for (int j = 0; j < optionBioses.count(); j++)
					childNode->addOptionBios(m_slotOptionDevices.at(i), optionBioses.at(j), optionBiosDescriptions.at(j));
				childNode->setDefaultOptionBios(m_slotOptionDevices.at(i), defaultBiosOption);
			}
			childNode->setDefaultOption(m_defaultOption);
			m_devNode->addChild(childNode);
			if ( !m_defaultOption.isEmpty() ) {
				DeviceTreeNode *deviceNode = new DeviceTreeNode(childNode, m_defaultOption);
				childNode->addChild(deviceNode);
				QString xmlBuffer(getXmlData(m_slotOptionDevices.at(m_slotOptions.indexOf(m_defaultOption))));
				QXmlInputSource xmlInputSource;
				xmlInputSource.setData(xmlBuffer);
				DeviceTreeXmlHandler xmlHandler(deviceNode);
				QXmlSimpleReader xmlReader;
				xmlReader.setContentHandler(&xmlHandler);
				xmlReader.parse(xmlInputSource);
			}
		}
		m_currentSlot.clear();
		m_slotOptions.clear();
		m_slotOptionDevices.clear();
		m_defaultOption.clear();
	} else if ( qName == "device" ) {
		m_devNode->addDevice(m_currentDevice, m_currentDeviceBriefName, m_currentDeviceType, m_currentDeviceTag, m_currentDeviceInterface, m_currentDeviceExtensions.join("/"));
		m_currentDevice.clear();
		m_currentDeviceBriefName.clear();
		m_currentDeviceType.clear();
		m_currentDeviceTag.clear();
		m_currentDeviceInterface.clear();
		m_currentDeviceExtensions.clear();
	}
	return rc;
}

QString DeviceTreeXmlHandler::getXmlData(const QString &machine)
{
	QString buffer("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + qmc2MachineList->xmlDb()->xml(machine));
	return buffer;
}

QString DeviceTreeXmlHandler::lookupDescription(const QString &machine)
{
	QStringList xmlLines(qmc2MachineList->xmlDb()->xml(machine).split('\n'));
	if ( xmlLines.count() > 1 ) {
		int index = 0;
		while ( index < xmlLines.count() && !xmlLines.at(index).startsWith("<description>") )
			index++;
		QString description(xmlLines.at(index));
		description.remove("<description>").remove("</description>");
		QTextDocument doc;
		doc.setHtml(description);
		return doc.toPlainText();
	} else
		return QString();
}

QString DeviceTreeXmlHandler::lookupBiosOptions(const QString &machine, QStringList *bioses, QStringList *biosDescriptions)
{
	QStringList xmlLines(qmc2MachineList->xmlDb()->xml(machine).split('\n'));
	if ( xmlLines.count() > 1 ) {
		int index = 0;
		QString defaultOption;
		while ( index < xmlLines.count() ) {
			if ( xmlLines.at(index).startsWith("<biosset ") ) {
				QString name;
				QString description;
				int startIndex = xmlLines.at(index).indexOf("name=\"");
				if ( startIndex >= 0 ) {
					startIndex += 6;
					int endIndex = xmlLines.at(index).indexOf("\"", startIndex);
					name = xmlLines.at(index).mid(startIndex, endIndex - startIndex);
				}
				startIndex = xmlLines.at(index).indexOf("description=\"");
				if ( startIndex >= 0 ) {
					startIndex += 13;
					int endIndex = xmlLines.at(index).indexOf("\"", startIndex);
					description = xmlLines.at(index).mid(startIndex, endIndex - startIndex);
					QTextDocument doc;
					doc.setHtml(description);
					description = doc.toPlainText();
				}
				if ( !name.isEmpty() && !description.isEmpty() ) {
					bioses->append(name);
					biosDescriptions->append(description);
					if ( xmlLines.at(index).indexOf("default=\"yes\"") >= 0 )
						defaultOption = name;
				}
			}
			index++;
		}
		return defaultOption;
	} else
		return QString();
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
