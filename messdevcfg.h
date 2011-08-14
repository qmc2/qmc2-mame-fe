#ifndef _MESSDEVCFG_H_
#define _MESSDEVCFG_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QXmlDefaultHandler>

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
    QMenu *deviceConfigurationListMenu;
    QMenu *configurationMenu;
    QMenu *deviceContextMenu;
    QMenu *slotContextMenu;
    QAction *actionRemoveConfiguration;

    MESSDeviceConfigurator(QString, QWidget *);
    ~MESSDeviceConfigurator();

    QString &getXmlData(QString);

  public slots:
    bool readSystemSlots();
    bool load();
    bool save();

    // callback functions
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
    void actionSelectDefaultDeviceDirectory_triggered();
    void actionSelectFile_triggered();
    void actionRemoveConfiguration_activated();

    // other
    void editorDataChanged(const QString &);

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
};

#endif
