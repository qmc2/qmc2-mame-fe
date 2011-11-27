#include <QtGui>

#include "messdevcfg.h"
#include "gamelist.h"
#include "macros.h"
#include "qmc2main.h"
#include "options.h"
#include "fileeditwidget.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern Gamelist *qmc2Gamelist;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern QString qmc2FileEditStartPath;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;

QMap<QString, QString> messXmlDataCache;
QList<FileEditWidget *> messFileEditWidgetList;
QMap<QString, QMap<QString, QStringList> > messSystemSlotMap;
QMap<QString, QString> messSlotNameMap;
QMap<QString, QIcon> messDevIconMap;
bool messSystemSlotsSupported = true;

MESSDeviceFileDelegate::MESSDeviceFileDelegate(QObject *parent)
	: QItemDelegate(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::MESSDeviceFileDelegate(QObject *parent = %1)").arg((qulonglong)parent));
#endif

	messFileEditWidgetList.clear();
}

QWidget *MESSDeviceFileDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::createEditor(QWidget *parent = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)parent));
#endif

	int row = index.row();
	QModelIndex sibling = index.sibling(row, QMC2_DEVCONFIG_COLUMN_EXT);
	QStringList extensions = sibling.model()->data(sibling, Qt::EditRole).toString().split("/", QString::SkipEmptyParts);
	QString filterString = tr("All files") + " (*)";
	if ( extensions.count() > 0 ) {
#if defined(Q_WS_WIN)
		filterString = tr("Valid device files") + " (*.zip";
#else
		filterString = tr("Valid device files") + " (*.[zZ][iI][pP]";
#endif
		for (int i = 0; i < extensions.count(); i++) {
			QString ext = extensions[i];
#if !defined(Q_WS_WIN)
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
	FileEditWidget *fileEditWidget = new FileEditWidget("", filterString, parent, true);
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
	model->setData(index, v);
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

	tabFileChooser->setUpdatesEnabled(false);

	tabWidgetDeviceSetup->setCornerWidget(toolButtonConfiguration, Qt::TopRightCorner);
	setEnabled(false);
	lineEditConfigurationName->setText(tr("Reading slot info, please wait..."));
	lineEditConfigurationName->setPlaceholderText(tr("Enter configuration name"));

#if !defined(Q_WS_X11)
	toolButtonChooserPlayEmbedded->setVisible(false);
#endif

#if !defined(QMC2_ALTERNATE_FSM)
	lcdNumberFileCounter->setVisible(false);
	toolButtonChooserReload->setVisible(false);
	comboBoxChooserFilterPattern->setVisible(false);
	toolButtonChooserClearFilterPattern->setVisible(false);
	toolButtonChooserProcessZIPs->setVisible(false);
	toolButtonChooserFilter->setVisible(false);
#else
	connect(&searchTimer, SIGNAL(timeout()), this, SLOT(comboBoxChooserFilterPattern_editTextChanged_delayed()));
	comboBoxChooserFilterPattern->lineEdit()->setPlaceholderText(tr("Enter search string"));
	comboBoxChooserFilterPatternHadFocus = false;
#endif
	dirModel = NULL;
	fileModel = NULL;
	fileChooserSetup = refreshRunning = dontIgnoreNameChange = false;
	updateSlots = true;

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
	toolButtonChooserSaveConfiguration->setIconSize(iconSize);
	comboBoxDeviceInstanceChooser->setIconSize(iconSize);
	treeWidgetDeviceSetup->setIconSize(iconSize);
	treeWidgetSlotOptions->setIconSize(iconSize);

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
	s = tr("Play selected game");
	action = deviceConfigurationListMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlay_activated()));
#if defined(Q_WS_X11)
	s = tr("Play selected game (embedded)");
	action = deviceConfigurationListMenu->addAction(tr("Play &embedded"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));
#endif
	deviceConfigurationListMenu->addSeparator();
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
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(dirChooserUseCurrentAsDefaultDirectory()));

	// file chooser context menu
	fileChooserContextMenu = new QMenu(this);
	s = tr("Play selected game");
	action = fileChooserContextMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlay_activated()));
#if defined(Q_WS_X11)
	s = tr("Play selected game (embedded)");
	action = fileChooserContextMenu->addAction(tr("Play &embedded"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));
#endif
#if defined(QMC2_ALTERNATE_FSM)
	fileChooserContextMenu->addSeparator();
	action = fileChooserContextMenu->addAction(tr("&Open archive"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/compressed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(treeViewFileChooser_toggleArchive()));
	actionChooserToggleArchive = action;
#endif

	if ( messDevIconMap.isEmpty() ) {
		messDevIconMap["cartridge"] = QIcon(QString::fromUtf8(":/data/img/dev_cartridge.png"));
		messDevIconMap["cassette"] = QIcon(QString::fromUtf8(":/data/img/dev_cassette.png"));
		messDevIconMap["cdrom"] = QIcon(QString::fromUtf8(":/data/img/dev_cdrom.png"));
		messDevIconMap["cylinder"] = QIcon(QString::fromUtf8(":/data/img/dev_cylinder.png"));
		messDevIconMap["floppydisk"] = QIcon(QString::fromUtf8(":/data/img/dev_floppydisk.png"));
		messDevIconMap["harddisk"] = QIcon(QString::fromUtf8(":/data/img/dev_harddisk.png"));
		messDevIconMap["magtape"] = QIcon(QString::fromUtf8(":/data/img/dev_magtape.png"));
		messDevIconMap["memcard"] = QIcon(QString::fromUtf8(":/data/img/dev_memcard.png"));
		messDevIconMap["parallel"] = QIcon(QString::fromUtf8(":/data/img/dev_parallel.png"));
		messDevIconMap["printer"] = QIcon(QString::fromUtf8(":/data/img/dev_printer.png"));
		messDevIconMap["punchtape"] = QIcon(QString::fromUtf8(":/data/img/dev_punchtape.png"));
		messDevIconMap["quickload"] = QIcon(QString::fromUtf8(":/data/img/dev_quickload.png"));
		messDevIconMap["serial"] = QIcon(QString::fromUtf8(":/data/img/dev_serial.png"));
		messDevIconMap["snapshot"] = QIcon(QString::fromUtf8(":/data/img/dev_snapshot.png"));
	}
}

MESSDeviceConfigurator::~MESSDeviceConfigurator()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::~MESSDeviceConfigurator()");
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
	if ( !fileChooserHeaderState.isEmpty() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserHeaderState", fileChooserHeaderState);
	if ( comboBoxDeviceInstanceChooser->currentText() != tr("No devices available") )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserDeviceInstance", comboBoxDeviceInstanceChooser->currentText());
}

QString &MESSDeviceConfigurator::getXmlDataWithEnabledSlots(QString machineName, QString configName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::getXmlDataWithEnabledSlots(QString machineName = %1, QString configName = %2)").arg(machineName).arg(configName));
#endif

	static QString xmlBuffer;

	xmlBuffer.clear();

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	QProcess commandProc;
#if defined(QMC2_SDLMESS)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#endif
#if !defined(Q_WS_WIN)
	commandProc.setStandardErrorFile("/dev/null");
#endif

	QStringList args;

	args << machineName;

	QList<QTreeWidgetItem *> allSlotItems = treeWidgetSlotOptions->findItems("*", Qt::MatchWildcard);
	foreach (QTreeWidgetItem *item, allSlotItems) {
		QString slotName = item->data(QMC2_SLOTCONFIG_COLUMN_SLOT, Qt::EditRole).toString();
		if ( !slotName.isEmpty() ) {
			QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
			if ( cb )
				if ( cb->currentIndex() > 0 )
					args << QString("-%1").arg(slotName) << cb->currentText().split(" ")[0];
		}
	}

	args << "-listxml";
	
	qApp->processEvents();

	bool commandProcStarted = false;
	int retries = 0;
	commandProc.start(qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString(), args);
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
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MESS executable within a reasonable time frame, giving up"));
		return xmlBuffer;
	}

#if defined(QMC2_SDLMESS)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLMAME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#endif

	if ( commandProcStarted && qmc2TempXml.open(QFile::ReadOnly) ) {
		QTextStream ts(&qmc2TempXml);
		qApp->processEvents();
		xmlBuffer = ts.readAll();
		qApp->processEvents();
#if defined(Q_WS_WIN)
		xmlBuffer.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
		qmc2TempXml.close();
		qmc2TempXml.remove();
	}

	QStringList xmlLines = xmlBuffer.split("\n");
	qApp->processEvents();
	xmlBuffer.clear();
	if ( !xmlLines.isEmpty() ) {
		int i = 0;
		QString s = "<machine name=\"" + machineName + "\"";
		while ( !xmlLines[i].contains(s) ) i++;
		xmlBuffer = "<?xml version=\"1.0\"?>\n";
		while ( !xmlLines[i].contains("</machine>") )
			xmlBuffer += xmlLines[i++].simplified() + "\n";
		xmlBuffer += "</machine>\n";
	}

	return xmlBuffer;
}

QString &MESSDeviceConfigurator::getXmlData(QString machineName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::getXmlData(QString machineName = %1)").arg(machineName));
#endif

	static QString xmlBuffer;

	xmlBuffer = messXmlDataCache[machineName];

	if ( xmlBuffer.isEmpty() ) {
		int i = 0;
		QString s = "<machine name=\"" + machineName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
		xmlBuffer = "<?xml version=\"1.0\"?>\n";
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") )
			xmlBuffer += qmc2Gamelist->xmlLines[i++].simplified() + "\n";
		xmlBuffer += "</machine>\n";
		messXmlDataCache[machineName] = xmlBuffer;
	}

	return xmlBuffer;
}

bool MESSDeviceConfigurator::readSystemSlots()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::readSystemSlots()");
#endif

	QTime elapsedTime;
	QTime loadTimer;

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	QProcess commandProc;
#if defined(QMC2_SDLMESS)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#endif

#if !defined(Q_WS_WIN)
	commandProc.setStandardErrorFile("/dev/null");
#endif

	QStringList args;
	args << "-listslots";
	
	setEnabled(false);
	lineEditConfigurationName->setText(tr("Reading slot info, please wait..."));

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading available system slots"));
	loadTimer.start();

	qApp->processEvents();

	bool commandProcStarted = false;
	int retries = 0;
	commandProc.start(qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString(), args);
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
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MESS executable within a reasonable time frame, giving up"));
		lineEditConfigurationName->setText(tr("Failed to read slot info"));
		return false;
	}

#if defined(QMC2_SDLMESS)
	QFile qmc2TempSlots(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
	QFile qmc2TempSlots(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLMAME)
	QFile qmc2TempSlots(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
	QFile qmc2TempSlots(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#endif

	if ( commandProcStarted && qmc2TempSlots.open(QFile::ReadOnly) ) {
		QTextStream ts(&qmc2TempSlots);
		qApp->processEvents();
		QString s = ts.readAll();
		qApp->processEvents();
		qmc2TempSlots.close();
		qmc2TempSlots.remove();
#if defined(Q_WS_WIN)
		s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
		QStringList slotLines = s.split("\n");
		if ( slotLines.count() > 1 ) {
			// we don't want the first two header lines
			slotLines.removeFirst();
			slotLines.removeFirst();
		}

		QString systemName, slotName, slotOption, slotDeviceName;

		for (int i = 0; i < slotLines.count(); i++) {
			QString slotLine = slotLines[i];
			if ( !slotLine.trimmed().isEmpty() ) {
				if ( !slotLine.startsWith(" ") ) {
					QStringList slotWords = slotLine.trimmed().split(" ", QString::SkipEmptyParts);
					if ( slotWords.count() >= 4 ) {
						systemName = slotWords[0];
						slotName = slotWords[1];
						if ( slotWords.count() > 2 ) {
							slotOption = slotWords[2];
							slotDeviceName = slotLine.trimmed();
							slotDeviceName.remove(QRegExp("^\\S+\\s+\\S+\\s+\\S+\\s+"));
							messSlotNameMap[slotOption] = slotDeviceName;
							messSystemSlotMap[systemName][slotName] << slotOption;
						} else {
							messSystemSlotMap[systemName][slotName].clear();
						}
					} else {
						systemName = slotWords[0];
						messSystemSlotMap[systemName].clear();
					}
				} else {
					QStringList slotWords = slotLine.trimmed().split(" ", QString::SkipEmptyParts);
					if ( slotLine[13] == ' ' ) { // this isn't nice, but I see no other way at the moment...
						slotOption = slotWords[0];
						slotDeviceName = slotLine.trimmed();
						slotDeviceName.remove(QRegExp("^\\S+\\s+"));
						messSystemSlotMap[systemName][slotName] << slotOption;
						messSlotNameMap[slotOption] = slotDeviceName;
					} else {
						slotName = slotWords[0];
						if ( slotWords.count() > 1 ) {
							slotOption = slotWords[1];
							slotDeviceName = slotLine.trimmed();
							slotDeviceName.remove(QRegExp("^\\S+\\s+\\S+\\s+"));
							messSystemSlotMap[systemName][slotName] << slotOption;
							messSlotNameMap[slotOption] = slotDeviceName;
						} else {
							messSystemSlotMap[systemName][slotName].clear();
						}
					}
				}
			}
		}
	}

	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading available system slots, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));

	setEnabled(true);

	return true;
}

void MESSDeviceConfigurator::slotOptionChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::slotOptionChanged(int index = %1)").arg(index));
#endif

	QTimer::singleShot(0, this, SLOT(refreshDeviceMap()));
}

bool MESSDeviceConfigurator::refreshDeviceMap()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::refreshDeviceMap()");
#endif

	if ( refreshRunning )
		return false;

	refreshRunning = true;

	QList<QListWidgetItem *> itemList = listWidgetDeviceConfigurations->selectedItems();
	QString configName;
	if ( !itemList.isEmpty() ) {
		if ( itemList[0]->text() != tr("No devices") )
			configName = itemList[0]->text();
	}

	QString xmlBuffer = getXmlDataWithEnabledSlots(messMachineName, configName);

	if ( xmlBuffer.isEmpty() ) {
		refreshRunning = false;
		return false;
	}

	treeWidgetDeviceSetup->clear();

	QXmlInputSource xmlInputSource;
	xmlInputSource.setData(xmlBuffer);
	MESSDeviceConfiguratorXmlHandler xmlHandler(treeWidgetDeviceSetup);
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(&xmlHandler);
	xmlReader.parse(xmlInputSource);

	comboBoxDeviceInstanceChooser->setUpdatesEnabled(false);

	QList<QTreeWidgetItem *> items = treeWidgetDeviceSetup->findItems("*", Qt::MatchWildcard);
	QStringList instances;

	extensionInstanceMap.clear();
	foreach (QTreeWidgetItem *item, items) {
		QString instance = item->text(QMC2_DEVCONFIG_COLUMN_NAME);
		if ( !instance.isEmpty() )
			instances << instance;
	}

	comboBoxDeviceInstanceChooser->clear();

	if ( instances.count() > 0 ) {
		qSort(instances);
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
				comboBoxDeviceInstanceChooser->addItem(messDevIconMap[devType], instance);
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
		tabFileChooser->setUpdatesEnabled(true);
		tabFileChooser->setEnabled(false);
	}

	comboBoxDeviceInstanceChooser->setUpdatesEnabled(true);

	if ( treeWidgetDeviceSetup->topLevelItemCount() == 1 ) {
		// this avoids delegate-resizing issues when only one item is available
		treeWidgetDeviceSetup->setUpdatesEnabled(false);
		QTreeWidgetItem *dummyItem = new QTreeWidgetItem(treeWidgetDeviceSetup);
		treeWidgetDeviceSetup->openPersistentEditor(dummyItem, QMC2_DEVCONFIG_COLUMN_FILE);
		delete treeWidgetDeviceSetup->takeTopLevelItem(1);
		treeWidgetDeviceSetup->setUpdatesEnabled(true);
	}

	if ( !configName.isEmpty() ) {
		dontIgnoreNameChange = true;
		updateSlots = false;
		on_lineEditConfigurationName_textChanged(configName);
		updateSlots = true;
		dontIgnoreNameChange = false;
	}

	refreshRunning = false;

	return true;
}

bool MESSDeviceConfigurator::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::load()");
#endif

	refreshRunning = true;

	if ( messSystemSlotMap.isEmpty() && messSystemSlotsSupported )
		if ( !readSystemSlots() ) {
			tabFileChooser->setUpdatesEnabled(true);
			refreshRunning = false;
			return false;
		}

	setEnabled(true);

	QString xmlBuffer = getXmlData(messMachineName);
  
	QXmlInputSource xmlInputSource;
	xmlInputSource.setData(xmlBuffer);
	MESSDeviceConfiguratorXmlHandler xmlHandler(treeWidgetDeviceSetup);
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(&xmlHandler);
	xmlReader.parse(xmlInputSource);

	comboBoxDeviceInstanceChooser->setUpdatesEnabled(false);
	QList<QTreeWidgetItem *> items = treeWidgetDeviceSetup->findItems("*", Qt::MatchWildcard);
	QStringList instances;

	extensionInstanceMap.clear();
	foreach (QTreeWidgetItem *item, items) {
		QString instance = item->text(QMC2_DEVCONFIG_COLUMN_NAME);
		if ( !instance.isEmpty() )
			instances << instance;
	}

	if ( instances.count() > 0 ) {
		qSort(instances);
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
				comboBoxDeviceInstanceChooser->addItem(messDevIconMap[devType], instance);
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
		tabFileChooser->setUpdatesEnabled(true);
		tabFileChooser->setEnabled(false);
	}

	comboBoxDeviceInstanceChooser->setUpdatesEnabled(true);

	if ( treeWidgetDeviceSetup->topLevelItemCount() == 1 ) {
		// this avoids delegate-resizing issues when only one item is available
		treeWidgetDeviceSetup->setUpdatesEnabled(false);
		QTreeWidgetItem *dummyItem = new QTreeWidgetItem(treeWidgetDeviceSetup);
		treeWidgetDeviceSetup->openPersistentEditor(dummyItem, QMC2_DEVCONFIG_COLUMN_FILE);
		delete treeWidgetDeviceSetup->takeTopLevelItem(1);
		treeWidgetDeviceSetup->setUpdatesEnabled(true);
	}

	QMapIterator<QString, QStringList> it(messSystemSlotMap[messMachineName]);
	while ( it.hasNext() ) {
		it.next();
		QString slotName = it.key();
		QStringList slotOptions;
		foreach (QString s, it.value())
			slotOptions << QString("%1 (%2)").arg(s).arg(messSlotNameMap[s]);
		QComboBox *cb = new QComboBox(0);
		cb->setAutoFillBackground(true);
		cb->insertItem(0, tr("not used"));
		if ( slotOptions.count() > 0 )
			cb->insertItems(1, slotOptions);
		QTreeWidgetItem *slotItem = new QTreeWidgetItem(treeWidgetSlotOptions);
		slotItem->setText(QMC2_SLOTCONFIG_COLUMN_SLOT, slotName);
		slotItem->setIcon(QMC2_SLOTCONFIG_COLUMN_SLOT, QIcon(QString::fromUtf8(":/data/img/slot.png")));
		treeWidgetSlotOptions->setItemWidget(slotItem, QMC2_SLOTCONFIG_COLUMN_OPTION, cb);
		connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOptionChanged(int)));
	}

	configurationMap.clear();
	slotMap.clear();

	qmc2Config->beginGroup(QString("MESS/Configuration/Devices/%1").arg(messMachineName));
	QString selectedConfiguration = qmc2Config->value("SelectedConfiguration").toString();
	QStringList configurationList = qmc2Config->childGroups();

	foreach (QString configName, configurationList) {
		configurationMap[configName].first = qmc2Config->value(QString("%1/Instances").arg(configName)).toStringList();
		configurationMap[configName].second = qmc2Config->value(QString("%1/Files").arg(configName)).toStringList();
		slotMap[configName].first = qmc2Config->value(QString("%1/Slots").arg(configName), QStringList()).toStringList();
		slotMap[configName].second = qmc2Config->value(QString("%1/SlotOptions").arg(configName), QStringList()).toStringList();
		QListWidgetItem *item = new QListWidgetItem(configName, listWidgetDeviceConfigurations);
		if ( selectedConfiguration == configName )
			listWidgetDeviceConfigurations->setCurrentItem(item);
	}

	qmc2FileEditStartPath = qmc2Config->value("DefaultDeviceDirectory").toString();

	qmc2Config->endGroup();

	// use the 'general software folder' as fall-back, if applicable
	if ( qmc2FileEditStartPath.isEmpty() ) {
		qmc2FileEditStartPath = qmc2Config->value("MESS/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(qmc2FileEditStartPath + "/" + messMachineName);
		if ( machineSoftwareFolder.exists() )
			qmc2FileEditStartPath = machineSoftwareFolder.canonicalPath();
	}

	QListWidgetItem *noDeviceItem = new QListWidgetItem(tr("No devices"), listWidgetDeviceConfigurations);
	if ( listWidgetDeviceConfigurations->currentItem() == NULL ) {
		dontIgnoreNameChange = true;
		listWidgetDeviceConfigurations->setCurrentItem(noDeviceItem);
		dontIgnoreNameChange = false;
	} else {
		refreshRunning = false;
		QTimer::singleShot(0, this, SLOT(refreshDeviceMap()));
	}

	refreshRunning = false;
	return true;
}

bool MESSDeviceConfigurator::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::save()");
#endif

	QString group = QString("MESS/Configuration/Devices/%1").arg(messMachineName);
	QString devDir = qmc2Config->value(QString("%1/DefaultDeviceDirectory").arg(group), "").toString();

	qmc2Config->remove(group);
	qmc2Config->beginGroup(group);

	if ( configurationMap.count() > 0 ) {
		foreach (QString configName, configurationMap.keys()) {
			QPair<QStringList, QStringList> config = configurationMap[configName];
			qmc2Config->setValue(QString("%1/Instances").arg(configName), config.first);
			qmc2Config->setValue(QString("%1/Files").arg(configName), config.second);
			QPair<QStringList, QStringList> slotConfig = slotMap[configName];
			if ( slotConfig.first.isEmpty() ) {
				qmc2Config->remove(QString("%1/Slots").arg(configName));
				qmc2Config->remove(QString("%1/SlotOptions").arg(configName));
			} else {
				qmc2Config->setValue(QString("%1/Slots").arg(configName), slotConfig.first);
				qmc2Config->setValue(QString("%1/SlotOptions").arg(configName), slotConfig.second);
			}
		}
	}

	if ( !devDir.isEmpty() )
		qmc2Config->setValue("DefaultDeviceDirectory", devDir);

	QListWidgetItem *curItem = listWidgetDeviceConfigurations->currentItem();
	if ( curItem != NULL ) {
		if ( curItem->text() == tr("No devices") )
			qmc2Config->remove("SelectedConfiguration");
		else
			qmc2Config->setValue("SelectedConfiguration", curItem->text());
	}

	qmc2Config->endGroup();

	return true;
}

void MESSDeviceConfigurator::on_toolButtonNewConfiguration_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_toolButtonNewConfiguration_clicked()");
#endif

	dontIgnoreNameChange = true;
	lineEditConfigurationName->clear();
	toolButtonCloneConfiguration->setEnabled(false);
	toolButtonSaveConfiguration->setEnabled(false);
	toolButtonRemoveConfiguration->setEnabled(false);
	treeWidgetDeviceSetup->setEnabled(true);
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
	listWidgetDeviceConfigurations->insertItem(listWidgetDeviceConfigurations->count(), targetName);

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
		QList<QTreeWidgetItem *> allItems = treeWidgetDeviceSetup->findItems("*", Qt::MatchWildcard);
		QStringList instances, files;
		foreach (QTreeWidgetItem *item, allItems) {
			QString fileName = item->data(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole).toString();
			if ( !fileName.isEmpty() ) {
				instances << item->data(QMC2_DEVCONFIG_COLUMN_NAME, Qt::EditRole).toString();
				files << fileName;
			}
		}
		configurationMap[cfgName].first = instances;
		configurationMap[cfgName].second = files;

		// save slot setup
		QList<QTreeWidgetItem *> allSlotItems = treeWidgetSlotOptions->findItems("*", Qt::MatchWildcard);
		QStringList slotNames, slotOptions;
		foreach (QTreeWidgetItem *item, allSlotItems) {
			QString slotName = item->data(QMC2_SLOTCONFIG_COLUMN_SLOT, Qt::EditRole).toString();
			if ( !slotName.isEmpty() ) {
				QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
				if ( cb )
					if ( cb->currentIndex() > 0 ) {
						slotNames << slotName;
						slotOptions << cb->currentText().split(" ")[0];
					}
			}
		}
		slotMap[cfgName].first = slotNames;
		slotMap[cfgName].second = slotOptions;
	} else {
		// add new device configuration
		listWidgetDeviceConfigurations->insertItem(listWidgetDeviceConfigurations->count(), cfgName);
		dontIgnoreNameChange = true;
		on_toolButtonSaveConfiguration_clicked();
	}

	on_lineEditConfigurationName_textChanged(lineEditConfigurationName->text());
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

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
}

void MESSDeviceConfigurator::on_lineEditConfigurationName_textChanged(const QString &text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_lineEditConfigurationName_textChanged(const QString &text = %1)").arg(text));
#endif

	toolButtonSaveConfiguration->setEnabled(false);
	if ( text == tr("No devices") ) {
		toolButtonCloneConfiguration->setEnabled(false);
		toolButtonSaveConfiguration->setEnabled(false);
		toolButtonRemoveConfiguration->setEnabled(false);
		treeWidgetDeviceSetup->setEnabled(false);
	} else if ( !text.isEmpty() ) {
		QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(text, Qt::MatchExactly);
		if ( matchedItemList.count() > 0 ) {
			toolButtonRemoveConfiguration->setEnabled(true);
			toolButtonSaveConfiguration->setEnabled(true);
			toolButtonCloneConfiguration->setEnabled(true);
		} else {
			toolButtonRemoveConfiguration->setEnabled(false);
			toolButtonSaveConfiguration->setEnabled(true);
			toolButtonCloneConfiguration->setEnabled(false);
		}
		treeWidgetDeviceSetup->setEnabled(true);
	} else {
		toolButtonCloneConfiguration->setEnabled(false);
		toolButtonSaveConfiguration->setEnabled(false);
		toolButtonRemoveConfiguration->setEnabled(false);
		treeWidgetDeviceSetup->setEnabled(true);
	}

	if ( dontIgnoreNameChange ) {
		QList<QTreeWidgetItem *> setupItemList = treeWidgetDeviceSetup->findItems("*", Qt::MatchWildcard);
		foreach (QTreeWidgetItem *setupItem, setupItemList)
			setupItem->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, QString());

		QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(text, Qt::MatchExactly);
		if ( matchedItemList.count() > 0 ) {
			matchedItemList[0]->setSelected(true);
			listWidgetDeviceConfigurations->setCurrentItem(matchedItemList[0]);
			listWidgetDeviceConfigurations->scrollToItem(matchedItemList[0]);
			QString configName = matchedItemList[0]->text();
			if ( configurationMap.contains(configName) ) {
				QPair<QStringList, QStringList> valuePair = configurationMap[configName];
				for (int i = 0; i < valuePair.first.count(); i++) {
					QList<QTreeWidgetItem *> itemList = treeWidgetDeviceSetup->findItems(valuePair.first[i], Qt::MatchExactly);
					if ( itemList.count() > 0 )
						itemList[0]->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, valuePair.second[i]);
				}
			}
			if ( updateSlots ) {
				QList<QTreeWidgetItem *> itemList = treeWidgetSlotOptions->findItems("*", Qt::MatchWildcard);
				foreach (QTreeWidgetItem *item, itemList) {
					QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
					if ( cb ) {
						cb->blockSignals(true);
						cb->setCurrentIndex(0);
						cb->blockSignals(false);
					}
				}
				if ( slotMap.contains(configName) ) {
					QPair<QStringList, QStringList> valuePair = slotMap[configName];
					for (int i = 0; i < valuePair.first.count(); i++) {
						QList<QTreeWidgetItem *> itemList = treeWidgetSlotOptions->findItems(valuePair.first[i], Qt::MatchExactly);
						if ( itemList.count() > 0 ) {
							QComboBox *cb = (QComboBox *)treeWidgetSlotOptions->itemWidget(itemList[0], QMC2_SLOTCONFIG_COLUMN_OPTION);
							if ( cb ) {
								int index = cb->findText(QString("%1 (%2)").arg(valuePair.second[i]).arg(messSlotNameMap[valuePair.second[i]]));
								if ( index >= 0 ) {
									cb->blockSignals(true);
									cb->setCurrentIndex(index);
									cb->blockSignals(false);
								}
							}
						}
					}
				}
			}
		} else {
			listWidgetDeviceConfigurations->clearSelection();
			toolButtonRemoveConfiguration->setEnabled(false);
			toolButtonCloneConfiguration->setEnabled(false);
		}
		refreshDeviceMap();
	}
	dontIgnoreNameChange = false;
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

	dontIgnoreNameChange = true;
	lineEditConfigurationName->setText(text);
	dontIgnoreNameChange = false;
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem * = %1)").arg((qulonglong)item));
#endif

	dontIgnoreNameChange = true;
	lineEditConfigurationName->setText(item->text());
	dontIgnoreNameChange = false;
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &point)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &point = (%1, %2))").arg(point.x()).arg(point.y()));
#endif

	QListWidgetItem *item = listWidgetDeviceConfigurations->itemAt(point);
	if ( item ) {
		if ( item->text() == tr("No devices") )
			actionRemoveConfiguration->setVisible(false);
		else
			actionRemoveConfiguration->setVisible(true);
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
		path = qmc2Config->value("MESS/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(path + "/" + messMachineName);
		if ( machineSoftwareFolder.exists() )
			path = machineSoftwareFolder.canonicalPath();
	}

	qmc2Config->beginGroup(group);

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose default device directory for '%1'").arg(messMachineName), path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if ( !s.isEmpty() )
		qmc2Config->setValue("DefaultDeviceDirectory", s);
	qmc2FileEditStartPath = qmc2Config->value("DefaultDeviceDirectory").toString();

	qmc2Config->endGroup();

	if ( qmc2FileEditStartPath.isEmpty() ) {
		qmc2FileEditStartPath = qmc2Config->value("MESS/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
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
			frameConfiguration->show();
			groupBoxAvailableDeviceConfigurations->show();
			groupBoxActiveDeviceConfiguration->setTitle(tr("Active device configuration"));
			break;
		case QMC2_DEVSETUP_TAB_SLOTCONFIG:
			frameConfiguration->show();
			groupBoxAvailableDeviceConfigurations->show();
			groupBoxActiveDeviceConfiguration->setTitle(tr("Active device configuration"));
			break;
		case QMC2_DEVSETUP_TAB_FILECHOOSER:
			frameConfiguration->hide();
			groupBoxAvailableDeviceConfigurations->hide();
			groupBoxActiveDeviceConfiguration->setTitle("");
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
		tabFileChooser->setUpdatesEnabled(true);
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
		path = qmc2Config->value("MESS/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
		QDir machineSoftwareFolder(path + "/" + messMachineName);
		if ( machineSoftwareFolder.exists() )
			path = machineSoftwareFolder.canonicalPath();
	}

	if ( path.isEmpty() )
		path = QDir::rootPath();

	dirModel = new QFileSystemModel(this);
	dirModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Drives | QDir::CaseSensitive);
	dirModel->setRootPath(QDir::rootPath());
	treeViewDirChooser->setModel(dirModel);
	treeViewDirChooser->setCurrentIndex(dirModel->index(path));
	for (int i = treeViewDirChooser->header()->count(); i > 0; i--) treeViewDirChooser->setColumnHidden(i, true);
	treeViewDirChooser->setHeaderHidden(true);

#if !defined(QMC2_ALTERNATE_FSM)
	fileModel = new QFileSystemModel(this);
	connect(fileModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(fileModel_rowsInserted(const QModelIndex &, int, int)));
	fileModel->setFilter(QDir::Files);
	fileModel->setNameFilterDisables(false);
	treeViewFileChooser->setModel(fileModel);
	on_toolButtonChooserFilter_toggled(toolButtonChooserFilter->isChecked());

	QFileInfo fi(path);
	if ( fi.isReadable() )
		treeViewFileChooser->setRootIndex(fileModel->setRootPath(path));

#else
	fileModelRowInsertionCounter = 0;
	lcdNumberFileCounter->display(0);
	lcdNumberFileCounter->setSegmentStyle(QLCDNumber::Flat);
	lcdNumberFileCounter->update();
	fileModel = new FileSystemModel(this);
	fileModel->setCurrentPath(path, false);
	connect(fileModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(fileModel_rowsInserted(const QModelIndex &, int, int)));
	connect(fileModel, SIGNAL(finished()), this, SLOT(fileModel_finished()));
	treeViewFileChooser->setModel(fileModel);
  	treeViewFileChooser->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/FileChooserHeaderState").toByteArray());
	connect(treeViewFileChooser->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeViewFileChooser_headerClicked(int)));
	connect(treeViewFileChooser->header(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(treeViewFileChooser_sectionMoved(int, int, int)));
	connect(treeViewFileChooser->header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(treeViewFileChooser_sectionResized(int, int, int)));
	on_toolButtonChooserFilter_toggled(toolButtonChooserFilter->isChecked());
#endif

	connect(treeViewDirChooser->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(treeViewDirChooser_selectionChanged(const QItemSelection &, const QItemSelection &)));
	connect(treeViewFileChooser->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(treeViewFileChooser_selectionChanged(const QItemSelection &, const QItemSelection &)));

	connect(toolButtonChooserPlay, SIGNAL(clicked()), qmc2MainWindow, SLOT(on_actionPlay_activated()));
	connect(toolButtonChooserPlayEmbedded, SIGNAL(clicked()), qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));

	QTimer::singleShot(QMC2_DIRCHOOSER_INIT_WAIT, this, SLOT(dirChooserDelayedInit()));

	tabFileChooser->setUpdatesEnabled(true);
	tabFileChooser->setEnabled(true);
}

void MESSDeviceConfigurator::dirChooserDelayedInit()
{
	treeViewDirChooser->scrollTo(treeViewDirChooser->currentIndex(), qmc2CursorPositioningMode);
}

void MESSDeviceConfigurator::treeViewDirChooser_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewDirChooser_selectionChanged(constQItemSelection &selected = ..., const QItemSelection &deselected = ...)");
#endif

	toolButtonChooserPlay->setEnabled(false);
	toolButtonChooserPlayEmbedded->setEnabled(false);
	toolButtonChooserSaveConfiguration->setEnabled(false);

	QString path = dirModel->fileInfo(selected.indexes()[0]).absoluteFilePath();

#if !defined(QMC2_ALTERNATE_FSM)
	QFileInfo fi(path);
	if ( fi.isReadable() ) {
		QAbstractItemModel *model = treeViewFileChooser->model();
		QItemSelectionModel *selectionModel = treeViewFileChooser->selectionModel();
		fileModel = new QFileSystemModel(this);
		connect(fileModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(fileModel_rowsInserted(const QModelIndex &, int, int)));
		fileModel->setFilter(QDir::Files);
		fileModel->setNameFilterDisables(false);
		on_toolButtonChooserFilter_toggled(toolButtonChooserFilter->isChecked());
		treeViewFileChooser->setModel(fileModel);
		delete model;
		delete selectionModel;
		treeViewFileChooser->setRootIndex(fileModel->setRootPath(path));
		connect(treeViewFileChooser->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(treeViewFileChooser_selectionChanged(const QItemSelection &, const QItemSelection &)));
	}
#else
	fileModel->setCurrentPath(path, false);
	on_toolButtonChooserFilter_toggled(toolButtonChooserFilter->isChecked());
#endif
}

void MESSDeviceConfigurator::treeViewFileChooser_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_selectionChanged(constQItemSelection &selected = ..., const QItemSelection &deselected = ...)");
#endif

	if ( selected.indexes().count() > 0 ) {
		toolButtonChooserPlay->setEnabled(true);
		toolButtonChooserPlayEmbedded->setEnabled(true);
		toolButtonChooserSaveConfiguration->setEnabled(true);
		if ( toolButtonChooserAutoSelect->isChecked() ) {
#if !defined(QMC2_ALTERNATE_FSM)
			QString instance = extensionInstanceMap[fileModel->fileInfo(selected.indexes().first()).suffix().toLower()];
#else
			QFileInfo fi(fileModel->absolutePath(selected.indexes().first()));
			QString instance = extensionInstanceMap[fi.suffix().toLower()];
#endif

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

#if defined(QMC2_ALTERNATE_FSM)
	fileModelRowInsertionCounter = 0;
	lcdNumberFileCounter->display(0);
	lcdNumberFileCounter->setSegmentStyle(QLCDNumber::Flat);
	lcdNumberFileCounter->update();
	treeViewFileChooser->selectionModel()->clearSelection();
	treeViewFileChooser->selectionModel()->reset();
	treeViewFileChooser->setUpdatesEnabled(false);
	toolButtonChooserReload->setEnabled(false);
	QTimer::singleShot(0, fileModel, SLOT(refresh()));
	//treeViewFileChooser->setRootIndex(fileModel->rootIndex());
#endif
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
	if ( modelIndexFileModel.isValid() ) {
		if ( fileModel->isZip(modelIndexFileModel) ) {
			actionChooserToggleArchive->setText(treeViewFileChooser->isExpanded(modelIndexFileModel) ? tr("&Close archive") : tr("&Open archive"));
			actionChooserToggleArchive->setVisible(true);
		} else
			actionChooserToggleArchive->setVisible(false);
		fileChooserContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeViewFileChooser->viewport()->mapToGlobal(p), fileChooserContextMenu));
		fileChooserContextMenu->show();
	}
}

void MESSDeviceConfigurator::on_treeViewFileChooser_activated(const QModelIndex &index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_treeViewFileChooser_activated(const QModelIndex &index = ...)");
#endif

	if ( toolButtonChooserProcessZIPs->isChecked() && fileModel->isZip(index) ) {
		fileModel->openZip(index);
	} else {
		QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
	}
}

void MESSDeviceConfigurator::treeViewFileChooser_toggleArchive()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::treeViewFileChooser_toggleArchive()");
#endif

	QModelIndexList selected = treeViewFileChooser->selectionModel()->selectedIndexes();

	if ( selected.count() > 0 ) {
		if ( treeViewFileChooser->isExpanded(selected[0]) ) {
			treeViewFileChooser->setExpanded(selected[0], false);
		} else {
			fileModel->openZip(selected[0]);
			treeViewFileChooser->setExpanded(selected[0], true);
		}
	}
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

#if defined(QMC2_ALTERNATE_FSM)
	fileModelRowInsertionCounter += end - start;
	if ( fileModelRowInsertionCounter > 1000 ) {
		fileModelRowInsertionCounter = 0;
		treeViewFileChooser->setUpdatesEnabled(true);
		treeViewFileChooser->update();
		treeViewFileChooser->setUpdatesEnabled(false);
	}
	lcdNumberFileCounter->display(fileModel->rowCount());
	lcdNumberFileCounter->update();
#else
	treeViewFileChooser->update();
#endif
}

void MESSDeviceConfigurator::fileModel_finished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::fileModel_finished()");
#endif

#if defined(QMC2_ALTERNATE_FSM)
	lcdNumberFileCounter->display(fileModel->rowCount());
	lcdNumberFileCounter->setSegmentStyle(QLCDNumber::Outline);
	lcdNumberFileCounter->update();
	treeViewFileChooser->setUpdatesEnabled(true);
	treeViewFileChooser->update();
	treeViewFileChooser->sortByColumn(treeViewFileChooser->header()->sortIndicatorSection(), treeViewFileChooser->header()->sortIndicatorOrder());
	toolButtonChooserReload->setEnabled(true);
	if ( comboBoxChooserFilterPatternHadFocus )
		comboBoxChooserFilterPattern->setFocus();
	comboBoxChooserFilterPatternHadFocus = false;
#endif
}

#if defined(QMC2_ALTERNATE_FSM)
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
#endif

void MESSDeviceConfigurator::on_toolButtonChooserSaveConfiguration_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_toolButtonChooserSaveConfiguration_clicked()");
#endif

	QString instance = comboBoxDeviceInstanceChooser->currentText();
	QModelIndexList indexList = treeViewFileChooser->selectionModel()->selectedIndexes();
	if ( indexList.count() > 0 && instance != tr("No devices available") ) {
#if !defined(QMC2_ALTERNATE_FSM)
		QString file = fileModel->fileInfo(indexList[0]).absoluteFilePath();
#else
		QString file = fileModel->absolutePath(indexList[0]);
#endif

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
					configurationMap[targetName].first = instances;
					configurationMap[targetName].second = files;
					listWidgetDeviceConfigurations->insertItem(listWidgetDeviceConfigurations->count(), targetName);
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfiguratorXmlHandler::MESSDeviceConfiguratorXmlHandler(QTreeWidget *parent = %1)").arg((qulonglong) parent));
#endif

	parentTreeWidget = parent;
}

MESSDeviceConfiguratorXmlHandler::~MESSDeviceConfiguratorXmlHandler()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfiguratorXmlHandler::~MESSDeviceConfiguratorXmlHandler()");
#endif

}

bool MESSDeviceConfiguratorXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfiguratorXmlHandler::startElement(...)");
#endif

	if ( qName == "device" ) {
		deviceType = attributes.value("type");
		deviceTag = attributes.value("tag");
		deviceInstances.clear();
		deviceExtensions.clear();
		deviceBriefName.clear();
	} else if ( qName == "instance" ) {
		deviceInstances << attributes.value("name");
		deviceBriefName = attributes.value("briefname");
	} else if ( qName == "extension" )
		deviceExtensions << attributes.value("name");

	return true;
}

bool MESSDeviceConfiguratorXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfiguratorXmlHandler::endElement(...)");
#endif

	if ( qName == "device" ) {
		foreach (QString instance, deviceInstances) {
			if ( !instance.isEmpty() ) {
				QTreeWidgetItem *deviceItem = new QTreeWidgetItem(parentTreeWidget);
				deviceItem->setText(QMC2_DEVCONFIG_COLUMN_NAME, instance);
				if ( !deviceType.isEmpty() )
					deviceItem->setIcon(QMC2_DEVCONFIG_COLUMN_NAME, messDevIconMap[deviceType]);
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

bool MESSDeviceConfiguratorXmlHandler::characters(const QString &str)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfiguratorXmlHandler::characters(const QString &str = %1)").arg(str));
#endif

	return true;
}
