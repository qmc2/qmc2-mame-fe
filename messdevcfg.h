#ifndef _MESSDEVCFG_H_
#define _MESSDEVCFG_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QXmlDefaultHandler>
#include <QFileSystemModel>
#if defined(QMC2_ALTERNATE_FSM)
#include "filesystemmodel.h"
#endif

#include "ui_messdevcfg.h"

class MESSDeviceFileDelegate : public QItemDelegate
{
	Q_OBJECT

	public:
		MESSDeviceFileDelegate(QObject *parent = 0);

		QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;
		virtual void setEditorData(QWidget *, const QModelIndex &) const;
		virtual void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;
		virtual void updateEditorGeometry(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;

	public slots:
		void dataChanged(QWidget *);

	signals:
		void editorDataChanged(const QString &);
};

class MESSDeviceConfiguratorXmlHandler : public QXmlDefaultHandler
{
	public:
		QTreeWidget *parentTreeWidget;
		QString deviceType;
		QString deviceTag;
		QString deviceBriefName;
		QStringList deviceInstances;
		QStringList deviceExtensions;

		MESSDeviceConfiguratorXmlHandler(QTreeWidget *);
		~MESSDeviceConfiguratorXmlHandler();

		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);
};

class MESSDeviceConfigurator : public QWidget, public Ui::MESSDeviceConfigurator
{
	Q_OBJECT

	public:
		bool dontIgnoreNameChange;
		MESSDeviceFileDelegate fileEditDelegate;
		QString messMachineName;
		QMap<QString, QPair<QStringList, QStringList> > configurationMap;
		QMap<QString, QPair<QStringList, QStringList> > slotMap;
		QMap<QString, QString> extensionInstanceMap;
		QMenu *deviceConfigurationListMenu;
		QMenu *configurationMenu;
		QMenu *deviceContextMenu;
		QMenu *slotContextMenu;
		QMenu *dirChooserContextMenu;
		QMenu *fileChooserContextMenu;
		QAction *actionRemoveConfiguration;
		QByteArray fileChooserHeaderState;
#if defined(QMC2_ALTERNATE_FSM)
		QAction *actionChooserToggleArchive;
		QTimer searchTimer;
		bool comboBoxChooserFilterPatternHadFocus;
		FileSystemModel *fileModel;
		int fileModelRowInsertionCounter;
#else
		QFileSystemModel *fileModel;
#endif
		QFileSystemModel *dirModel;
		QModelIndex modelIndexFileModel;
		QModelIndex modelIndexDirModel;
		bool fileChooserSetup;

		MESSDeviceConfigurator(QString, QWidget *);
		~MESSDeviceConfigurator();

		QString &getXmlData(QString);

	public slots:
		bool readSystemSlots();
		bool load();
		bool save();

		// auto-connected callback functions
		void on_lineEditConfigurationName_textChanged(const QString &);
		void on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem *);
		void on_toolButtonNewConfiguration_clicked();
		void on_toolButtonCloneConfiguration_clicked();
		void on_toolButtonSaveConfiguration_clicked();
		void on_toolButtonRemoveConfiguration_clicked();
		void on_listWidgetDeviceConfigurations_itemActivated(QListWidgetItem *);
		void on_listWidgetDeviceConfigurations_currentTextChanged(const QString &);
		void on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &);
		void on_treeWidgetDeviceSetup_customContextMenuRequested(const QPoint &);
		void on_treeWidgetSlotOptions_customContextMenuRequested(const QPoint &);
		void on_tabWidgetDeviceSetup_currentChanged(int);
		void on_toolButtonChooserFilter_toggled(bool);
		void on_comboBoxDeviceInstanceChooser_activated(const QString &);
		void on_treeViewDirChooser_customContextMenuRequested(const QPoint &);
		void on_treeViewFileChooser_customContextMenuRequested(const QPoint &);
		void on_treeViewFileChooser_activated(const QModelIndex &);
		void on_toolButtonChooserSaveConfiguration_clicked();
#if defined(QMC2_ALTERNATE_FSM)
		void on_toolButtonChooserReload_clicked();
		void on_comboBoxChooserFilterPattern_editTextChanged(const QString &);
#endif
		void on_splitterFileChooser_splitterMoved(int, int);

		// other callbacks
		void actionSelectDefaultDeviceDirectory_triggered();
		void actionSelectFile_triggered();
		void actionRemoveConfiguration_activated();
		void treeViewDirChooser_selectionChanged(const QItemSelection &, const QItemSelection &);
		void treeViewFileChooser_selectionChanged(const QItemSelection &, const QItemSelection &);
		void dirChooserUseCurrentAsDefaultDirectory();
		void dirChooserDelayedInit();
		void fileModel_rowsInserted(const QModelIndex &, int, int);
		void fileModel_finished();
		void treeViewFileChooser_headerClicked(int);
		void treeViewFileChooser_sectionMoved(int, int, int);
		void treeViewFileChooser_sectionResized(int, int, int);
#if defined(QMC2_ALTERNATE_FSM)
		void comboBoxChooserFilterPattern_editTextChanged_delayed();
		void treeViewFileChooser_toggleArchive();
#endif

// misc
		void editorDataChanged(const QString &);
		void setupFileChooser();

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
