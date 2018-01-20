#ifndef DEVICECONFIGURATOR_H
#define DEVICECONFIGURATOR_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QXmlDefaultHandler>
#include <QStringList>
#include <QString>
#include <QMovie>
#include <QList>
#include <QIcon>
#include <QHash>
#include <QMap>

#include "filesystemmodel.h"
#include "aspectratiolabel.h"
#include "ui_deviceconfigurator.h"

class DeviceTreeNode
{
	public:
		explicit DeviceTreeNode(DeviceTreeNode *parent, const QString &name) :
			m_parent(parent),
			m_name(name)
		{
			while ( m_name.startsWith(':') )
				m_name.remove(0, 1);
			m_fullName = m_name;
			DeviceTreeNode *n = m_parent;
			while ( n != 0 ) {
				if ( n->parent() != 0 )
					m_fullName.prepend(n->name() + ':');
				n = n->parent();
			}
		}
		~DeviceTreeNode() { qDeleteAll(children()); }

		DeviceTreeNode *parent() { return m_parent; }
		QList<DeviceTreeNode *> &children() { return m_children; }
		void setName(const QString &name) { m_name = name; }
		QString &name() { return m_name; }
		QString &fullName() { return m_fullName; }
		QStringList &devices() { return m_devices; }
		QStringList allDevices()
		{
			QStringList devs(devices());
			foreach (DeviceTreeNode *child, children())
				devs.append(child->allDevices());
			return devs;
		}
		QStringList &deviceBriefNames() { return m_deviceBriefNames; }
		QStringList allDeviceBriefNames()
		{
			QStringList devBriefNames(deviceBriefNames());
			foreach (DeviceTreeNode *child, children())
				devBriefNames.append(child->allDeviceBriefNames());
			return devBriefNames;
		}
		QStringList &deviceTypes() { return m_deviceTypes; }
		QStringList allDeviceTypes()
		{
			QStringList devTypes(deviceTypes());
			foreach (DeviceTreeNode *child, children())
				devTypes.append(child->allDeviceTypes());
			return devTypes;
		}
		QStringList &deviceTags() { return m_deviceTags; }
		QStringList allDeviceTags()
		{
			QStringList devTags(deviceTags());
			foreach (DeviceTreeNode *child, children())
				devTags.append(child->allDeviceTags());
			return devTags;
		}
		QStringList &deviceInterfaces() { return m_deviceInterfaces; }
		QStringList allDeviceInterfaces()
		{
			QStringList devInterfaces(deviceInterfaces());
			foreach (DeviceTreeNode *child, children())
				devInterfaces.append(child->allDeviceInterfaces());
			return devInterfaces;
		}
		QStringList &deviceExtensions() { return m_deviceExtensions; }
		QStringList allDeviceExtensions()
		{
			QStringList devExtensions(deviceExtensions());
			foreach (DeviceTreeNode *child, children())
				devExtensions.append(child->allDeviceExtensions());
			return devExtensions;
		}
		QStringList &options() { return m_options; }
		QStringList &optionDevices() { return m_optionDevices; }
		QStringList &optionDescriptions() { return m_optionDescriptions; }
		QStringList &optionBioses(const QString &option) { return m_optionBioses[option]; }
		QStringList &optionBiosDescriptions(const QString &option) { return m_optionBiosDescriptions[option]; }
		void setDefaultOptionBios(const QString &optionName, const QString &optionBios) { m_optionBiosDefaults.insert(optionName, optionBios); }
		QString &defaultOptionBios(const QString &option) { return m_optionBiosDefaults[option]; }
		void setDefaultOption(const QString &defaultOption) { m_defaultOption = defaultOption; }
		QString &defaultOption() { return m_defaultOption; }
		void addChild(DeviceTreeNode *child) { m_children.append(child); }
		void addOption(const QString &optionName, const QString &optionDeviceName, const QString &optionDescription)
		{
			m_options.append(optionName);
			m_optionDevices.append(optionDeviceName);
			m_optionDescriptions.append(optionDescription);
		}
		void addOptionBios(const QString &optionName, const QString &optionBios, const QString &optionBiosDescription)
		{
			m_optionBioses[optionName].append(optionBios);
			m_optionBiosDescriptions[optionName].append(optionBiosDescription);
		}
		void addDeviceNode(const QString &device, const QString &briefName, const QString &type, const QString &tagName, const QString &interface, const QString &extensions)
		{
			m_devices.append(device);
			m_deviceBriefNames.append(briefName);
			m_deviceTypes.append(type);
			m_deviceTags.append(tagName);
			m_deviceInterfaces.append(interface);
			m_deviceExtensions.append(extensions);
		}
		DeviceTreeNode *findNode(DeviceTreeNode *node, const QString &fName)
		{
			foreach (DeviceTreeNode *child, node->children()) {
				if ( child->fullName() == fName )
					return child;
			}
			DeviceTreeNode *n = 0;
			foreach (DeviceTreeNode *child, node->children()) {
				n = child->findNode(child, fName);
				if ( n != 0 )
					break;
			}
			return n;
		}

	private:
		DeviceTreeNode *m_parent;
		QList<DeviceTreeNode *> m_children;
		QStringList m_options;
		QStringList m_optionDevices;
		QStringList m_optionDescriptions;
		QHash<QString, QStringList> m_optionBioses;
		QHash<QString, QStringList> m_optionBiosDescriptions;
		QHash<QString, QString> m_optionBiosDefaults;
		QString m_name;
		QString m_fullName;
		QString m_defaultOption;
		QStringList m_devices;
		QStringList m_deviceBriefNames;
		QStringList m_deviceTypes;
		QStringList m_deviceTags;
		QStringList m_deviceInterfaces;
		QStringList m_deviceExtensions;
};

class DeviceTreeXmlHandler : public QXmlDefaultHandler
{
	public:
		DeviceTreeXmlHandler(DeviceTreeNode *devNode) : m_devNode(devNode) {}
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);

		QString lookupDescription(const QString &);
		QString lookupBiosOptions(const QString &, QStringList *, QStringList *);
		QString getXmlData(const QString &);

	private:
		DeviceTreeNode *m_devNode;
		QString m_currentSlot;
		QString m_defaultOption;
		QString m_currentDevice;
		QString m_currentDeviceBriefName;
		QString m_currentDeviceType;
		QString m_currentDeviceTag;
		QString m_currentDeviceInterface;
		QStringList m_currentDeviceExtensions;
		QStringList m_slotOptions;
		QStringList m_slotOptionDevices;
};

class DeviceItemDelegate : public QItemDelegate
{
	Q_OBJECT

	public:
		DeviceItemDelegate(QObject *parent = 0);

		QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;
		virtual void setEditorData(QWidget *, const QModelIndex &) const;
		virtual void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;
		virtual void updateEditorGeometry(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;

		static void loadMidiInterfaces();

	public slots:
		void dataChanged(QWidget *);

	signals:
		void editorDataChanged(const QString &);
};

class DeviceConfigurator : public QWidget, public Ui::DeviceConfigurator
{
	Q_OBJECT

	public:
		bool dontIgnoreNameChange;
		bool updateSlots;
		bool comboBoxChooserFilterPatternHadFocus;
		int fileModelRowInsertionCounter;
		DeviceItemDelegate fileEditDelegate;
		QByteArray fileChooserHeaderState;
		QByteArray dirChooserHeaderState;
		QString currentConfigName;
		QString oldConfigurationName;
		QStringList nestedSlots;
		QStringList allSlots;
		QMap<QString, QPair<QStringList, QStringList> > configurationMap;
		QMap<QString, QPair<QStringList, QStringList> > slotMap;
		QMap<QString, QPair<QStringList, QStringList> > slotBiosMap;
		QMap<QString, QString> extensionInstanceMap;
		QMap<QComboBox *, int> slotPreselectionMap;
		QMap<QComboBox *, int> nestedSlotPreselectionMap;
		QMap<QComboBox *, int> newNestedSlotPreselectionMap;
		QMap<QString, QMap<QString, QString> > nestedSlotOptionMap;
		QMap<QString, QString> slotDeviceNames;
		QMenu *deviceConfigurationListMenu;
		QMenu *configurationMenu;
		QMenu *deviceContextMenu;
		QMenu *dirChooserContextMenu;
		QMenu *fileChooserContextMenu;
		QMenu *folderModeMenu;
		QAction *actionRenameConfiguration;
		QAction *actionRemoveConfiguration;
		QAction *actionChooserToggleArchive;
		QAction *actionChooserViewPdf;
		QAction *actionChooserViewPostscript;
		QAction *actionChooserViewHtml;
		QAction *actionChooserOpenExternally;
		QAction *actionChooserOpenFolder;
		QAction *actionChooserPlay;
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		QAction *actionChooserPlayEmbedded;
#endif
		QTimer searchTimer;
		QModelIndex modelIndexFileModel;
		QModelIndex modelIndexDirModel;
		QListWidgetItem *configurationRenameItem;

		static QHash<QString, QIcon> deviceIconHash;
		static QHash<QString, int> deviceNameToIndexHash;
		static QStringList midiInInterfaces;
		static QStringList midiOutInterfaces;
		static bool reloadMidiInterfaces;

		DeviceConfigurator(QString, QWidget *);
		~DeviceConfigurator();

		QString getXmlData(const QString &);
		void insertChildItems(QTreeWidgetItem *, QList<QTreeWidgetItem *> &);

		// NEW stuff
		void makeUnique(QStringList *, QStringList *);
		void updateDeviceTree(DeviceTreeNode *, const QString &);
		void traverseDeviceTree(QTreeWidgetItem *, DeviceTreeNode *);

		void setCurrentMachine(const QString &machine) { m_currentMachine = machine; }
		QString &currentMachine() { return m_currentMachine; }

		FileSystemModel *fileModel() { return m_fileModel; }
		DirectoryModel *dirModel() { return m_dirModel; }

	public slots:
		bool refreshDeviceMap();
		bool load();
		bool save();
		void saveSetup();

		// auto-connected callback functions
		void on_lineEditConfigurationName_textChanged(const QString &);
		void on_toolButtonNewConfiguration_clicked();
		void on_toolButtonCloneConfiguration_clicked();
		void on_toolButtonSaveConfiguration_clicked();
		void on_toolButtonRemoveConfiguration_clicked();
		void on_listWidgetDeviceConfigurations_itemActivated(QListWidgetItem *);
		void on_listWidgetDeviceConfigurations_currentTextChanged(const QString &);
		void on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem *);
		void on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &);
		void on_treeWidgetDeviceSetup_customContextMenuRequested(const QPoint &);
		void on_tabWidgetDeviceSetup_currentChanged(int);
		void on_toolButtonChooserFilter_toggled(bool);
		void on_comboBoxDeviceInstanceChooser_activated(const QString &);
		void on_treeViewDirChooser_customContextMenuRequested(const QPoint &);
		void on_treeViewFileChooser_customContextMenuRequested(const QPoint &);
		void on_treeViewFileChooser_activated(const QModelIndex &);
		void on_treeViewFileChooser_clicked(const QModelIndex &);
		void on_toolButtonChooserSaveConfiguration_clicked();
		void on_toolButtonChooserReload_clicked();
		void on_comboBoxChooserFilterPattern_editTextChanged(const QString &);
		void on_splitterFileChooser_splitterMoved(int, int);

		// other callbacks
		void actionSelectDefaultDeviceDirectory_triggered();
		void actionSelectFile_triggered();
		void actionRenameConfiguration_activated();
		void actionRemoveConfiguration_activated();
		void treeViewDirChooser_selectionChanged(const QItemSelection &, const QItemSelection &);
		void treeViewFileChooser_selectionChanged(const QItemSelection &, const QItemSelection &);
		void dirChooserUseCurrentAsDefaultDirectory();
		void dirChooserDelayedInit();
		void fileModel_rowsInserted(const QModelIndex &, int, int);
		void fileModel_finished();
		void treeViewDirChooser_headerClicked(int);
		void treeViewFileChooser_headerClicked(int);
		void treeViewFileChooser_sectionMoved(int, int, int);
		void treeViewFileChooser_sectionResized(int, int, int);
		void comboBoxChooserFilterPattern_editTextChanged_delayed();
		void treeViewFileChooser_toggleArchive();
		void treeViewFileChooser_viewPdf();
		void treeViewFileChooser_viewHtml();
		void treeViewFileChooser_expandRequested();
		void treeViewFileChooser_openFolder();
		void treeViewFileChooser_openFileExternally();
		void slotOptionChanged(int);
		void folderModeMenu_foldersOff();
		void folderModeMenu_foldersOn();
		void folderModeMenu_foldersFirst();

		// misc
		void editorDataChanged(const QString &);
		void setupFileChooser();
		void configurationItemChanged(QListWidgetItem *);

		// NEW stuff
		void updateDeviceMappings();
		void optionComboBox_currentIndexChanged(int);

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void resizeEvent(QResizeEvent *);

	private:
		AspectRatioLabel *m_loadingAnimationOverlay;
		QMovie *m_loadAnimMovie;
		DeviceTreeNode *m_rootNode;
		bool m_fullyLoaded;
		bool m_fileChooserSetup;
		bool m_foldersFirst;
		bool m_includeFolders;
		QString m_currentMachine;
		FileSystemModel *m_fileModel;
		DirectoryModel *m_dirModel;
};

class FileChooserKeyEventFilter : public QObject
{
	Q_OBJECT

	public:
		FileChooserKeyEventFilter(QObject *parent = 0) : QObject(parent) { ; }

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	signals:
		void expandRequested();
};

#endif
