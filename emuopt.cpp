#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QHeaderView>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTreeWidgetItem>
#include <QScrollBar>

#include <stdlib.h>
#include "emuopt.h"
#include "options.h"
#include "gamelist.h"
#include "itemselect.h"
#include "qmc2main.h"
#include "fileeditwidget.h"
#include "direditwidget.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern QSettings *qmc2Config;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;
extern EmulatorOptions *qmc2EmulatorOptions;
extern bool qmc2GuiReady;
extern QTreeWidgetItem *qmc2CurrentItem;
extern Gamelist *qmc2Gamelist;
extern bool qmc2ReloadActive;

QMap<QString, QList<EmulatorOption> > EmulatorOptions::templateMap;
QMap<QString, bool> EmulatorOptions::optionExpansionMap;
QMap<QString, bool> EmulatorOptions::sectionExpansionMap;
QMap<QString, QTreeWidgetItem *> EmulatorOptions::sectionItemMap;
int EmulatorOptions::horizontalScrollPosition = 0;
int EmulatorOptions::verticalScrollPosition = 0;

QString optionDescription = "";
int optionType = QMC2_EMUOPT_TYPE_UNKNOWN;
EmulatorOptions *emulatorOptions = NULL;

EmulatorOptionDelegate::EmulatorOptionDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::EmulatorOptionDelegate(QObject *parent = %1)").arg((qulonglong)parent));
#endif

}

void EmulatorOptionDelegate::dataChanged()
{
  QWidget *widget = (QWidget *)sender();

  if ( widget ) {
    emit commitData(widget);
    if ( qmc2GlobalEmulatorOptions && parent() == qmc2GlobalEmulatorOptions )
      qmc2GlobalEmulatorOptions->changed = TRUE;
  }
}

QWidget *EmulatorOptionDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  #define _MIN (-2 * 1024 * 1024)
  #define _MAX (2 * 1024 * 1024)

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::createEditor(QWidget *parent = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)parent));
#endif

  switch ( optionType ) {
    case QMC2_EMUOPT_TYPE_BOOL: {
      QCheckBox *checkBoxEditor = new QCheckBox(parent);
      checkBoxEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      checkBoxEditor->setAccessibleName("checkBoxEditor");
      if ( !optionDescription.isEmpty() )
        checkBoxEditor->setToolTip(optionDescription);
      connect(checkBoxEditor, SIGNAL(toggled(bool)), this, SLOT(dataChanged()));
      return checkBoxEditor;
    }

    case QMC2_EMUOPT_TYPE_INT: {
      QSpinBox *spinBoxEditor = new QSpinBox(parent);
      spinBoxEditor->setRange(_MIN, _MAX);
      spinBoxEditor->setSingleStep(1);
      spinBoxEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      spinBoxEditor->setAccessibleName("spinBoxEditor");
      if ( !optionDescription.isEmpty() )
        spinBoxEditor->setToolTip(optionDescription);
      connect(spinBoxEditor, SIGNAL(valueChanged(int)), this, SLOT(dataChanged()));
      return spinBoxEditor;
    }

    case QMC2_EMUOPT_TYPE_FLOAT: {
      QDoubleSpinBox *doubleSpinBoxEditor = new QDoubleSpinBox(parent);
      doubleSpinBoxEditor->setRange(_MIN, _MAX);
      doubleSpinBoxEditor->setSingleStep(0.1);
      doubleSpinBoxEditor->setDecimals(6);
      doubleSpinBoxEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      doubleSpinBoxEditor->setAccessibleName("doubleSpinBoxEditor");
      if ( !optionDescription.isEmpty() )
        doubleSpinBoxEditor->setToolTip(optionDescription);
      connect(doubleSpinBoxEditor, SIGNAL(valueChanged(double)), this, SLOT(dataChanged()));
      return doubleSpinBoxEditor;
    }

    case QMC2_EMUOPT_TYPE_FILE: {
      FileEditWidget *fileEditor = new FileEditWidget("", tr("All files (*)"), parent);
      fileEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      fileEditor->setAccessibleName("fileEditor");
      if ( !optionDescription.isEmpty() ) {
        fileEditor->lineEditFile->setToolTip(optionDescription);
        fileEditor->pushButtonBrowse->setToolTip(tr("Browse: ") + optionDescription);
      }
      connect(fileEditor, SIGNAL(dataChanged(QWidget *)), this, SLOT(dataChanged()));
      return fileEditor;
    }

    case QMC2_EMUOPT_TYPE_DIRECTORY: {
      DirectoryEditWidget *directoryEditor = new DirectoryEditWidget("", parent);
      directoryEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      directoryEditor->setAccessibleName("directoryEditor");
      if ( !optionDescription.isEmpty() ) {
        directoryEditor->lineEditDirectory->setToolTip(optionDescription);
        directoryEditor->pushButtonBrowse->setToolTip(tr("Browse: ") + optionDescription);
      }
      connect(directoryEditor, SIGNAL(dataChanged(QWidget *)), this, SLOT(dataChanged()));
      return directoryEditor;
    }

    case QMC2_EMUOPT_TYPE_STRING:
    default: {
      QLineEdit *lineEditEditor = new QLineEdit(parent);
      lineEditEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      lineEditEditor->setAccessibleName("lineEditEditor");
      if ( !optionDescription.isEmpty() )
        lineEditEditor->setToolTip(optionDescription);
      connect(lineEditEditor, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged()));
      return lineEditEditor;
    }
  }
}

void EmulatorOptionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::setEditorData(QWidget *editor = %1, const QModelIndex &index)").arg((qulonglong)editor));
#endif

  if ( editor->accessibleName() == "checkBoxEditor" ) {
    bool value = index.model()->data(index, Qt::EditRole).toBool();
    QCheckBox *checkBoxEditor = static_cast<QCheckBox *>(editor);
    checkBoxEditor->setChecked(value);
  } else if ( editor->accessibleName() == "spinBoxEditor" ) {
    int value = index.model()->data(index, Qt::EditRole).toInt();
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->setValue(value);
  } else if ( editor->accessibleName() == "doubleSpinBoxEditor" ) {
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    QDoubleSpinBox *doubleSpinBox = static_cast<QDoubleSpinBox *>(editor);
    doubleSpinBox->setValue(value);
  } else if ( editor->accessibleName() == "fileEditor" ) {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    FileEditWidget *fileEditor = static_cast<FileEditWidget *>(editor);
    fileEditor->lineEditFile->setText(value);
  } else if ( editor->accessibleName() == "directoryEditor" ) {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    DirectoryEditWidget *directoryEditor = static_cast<DirectoryEditWidget *>(editor);
    directoryEditor->lineEditDirectory->setText(value);
  } else {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    lineEdit->setText(value);
  }
}

void EmulatorOptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::setModelData(QWidget *editor = %1, QAbstractItemModel *model = %2, const QModelIndex &index)").arg((qulonglong)editor).arg((qulonglong)model));
#endif

  if ( editor->accessibleName() == "checkBoxEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_BOOL;
    QCheckBox *checkBoxEditor = static_cast<QCheckBox*>(editor);
    bool v = checkBoxEditor->isChecked();
    model->setData(index, QColor(0, 0, 0, 0), Qt::ForegroundRole);
    model->setData(index, QFont("Helvetiva", 1), Qt::FontRole);
    model->setData(index, v);
  } else if ( editor->accessibleName() == "spinBoxEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_INT;
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int v = spinBox->value();
    model->setData(index, v);
  } else if ( editor->accessibleName() == "doubleSpinBoxEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_FLOAT;
    QDoubleSpinBox *doubleSpinBox = static_cast<QDoubleSpinBox*>(editor);
    doubleSpinBox->interpretText();
    double v = doubleSpinBox->value();
    model->setData(index, v);
  } else if ( editor->accessibleName() == "fileEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_FILE;
    FileEditWidget *fileEditor = static_cast<FileEditWidget*>(editor);
    QString v = fileEditor->lineEditFile->text();
    model->setData(index, v);
  } else if ( editor->accessibleName() == "directoryEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_DIRECTORY;
    DirectoryEditWidget *directoryEditor = static_cast<DirectoryEditWidget*>(editor);
    QString v = directoryEditor->lineEditDirectory->text();
    model->setData(index, v);
  } else {
    optionType = QMC2_EMUOPT_TYPE_STRING;
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QString v = lineEdit->text();
    model->setData(index, v);
  }
}

void EmulatorOptionDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::updateEditorGeometry(QWidget *editor = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)editor));
#endif

  editor->setGeometry(option.rect);
  QFontMetrics fm(QApplication::font());
  QSize iconSize(fm.height() - 2, fm.height() - 2);
  if ( editor->accessibleName() == "fileEditor" ) {
    FileEditWidget *fileEditWidget = static_cast<FileEditWidget *>(editor);
    fileEditWidget->pushButtonBrowse->setIconSize(iconSize);
  } else if ( editor->accessibleName() == "directoryEditor" ) {
    DirectoryEditWidget *directoryEditor = static_cast<DirectoryEditWidget *>(editor);
    directoryEditor->pushButtonBrowse->setIconSize(iconSize);
  }
}

EmulatorOptions::EmulatorOptions(QString group, QWidget *parent)
  : QTreeWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::EmulatorOptions(QString group = \"" + group + "\", QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  // initialize
  templateVersion = tr("unknown");
  connect(&searchTimer, SIGNAL(timeout()), this, SLOT(searchTimeout()));
  lineEditSearch = NULL;
  if ( !group.contains("Global") ) {
    emulatorOptions = NULL;
#if defined(QMC2_EMUTYPE_MAME)
    setStatusTip(tr("Game specific emulator configuration"));
#elif defined(QMC2_EMUTYPE_MESS)
    setStatusTip(tr("Machine specific emulator configuration"));
#endif
  } else {
    emulatorOptions = this;
    setStatusTip(tr("Global emulator configuration"));
  }
  loadActive = changed = FALSE;
  settingsGroup = group;
  delegate = new EmulatorOptionDelegate(this);
  setColumnCount(2);  
  setItemDelegateForColumn(1, delegate);
  setAlternatingRowColors(TRUE);
  headerItem()->setText(0, tr("Option / Attribute"));
  headerItem()->setText(1, tr("Value"));
  header()->setClickable(FALSE);
  if ( templateMap.count() == 0 )
    createTemplateMap();
  createMap();
}

EmulatorOptions::~EmulatorOptions()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::~EmulatorOptions()");
#endif

  if ( delegate )
    delete delegate;
}

void EmulatorOptions::pseudoConstructor()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::pseudoConstructor()");
#endif

  if ( emulatorOptions == this ) {
    header()->resizeSections(QHeaderView::Custom);
    header()->resizeSection(0, qmc2Config->value(settingsGroup + "/OptionColumnWidth", 200).toInt());
    header()->restoreState(qmc2Config->value(settingsGroup + "/HeaderState").toByteArray());
  } else {
    header()->setMovable(FALSE);
  }
}

void EmulatorOptions::pseudoDestructor()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::pseudoDestructor()");
#endif

  if ( emulatorOptions == this ) {
    if ( header()->sectionSize(0) != 200 )
      qmc2Config->setValue(settingsGroup + "/OptionColumnWidth", header()->sectionSize(0));
    else
      qmc2Config->remove(settingsGroup + "/OptionColumnWidth");
    qmc2Config->setValue(settingsGroup + "/HeaderState", header()->saveState());
  }
}

void EmulatorOptions::load(bool overwrite)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::load(bool overwrite = " + QString(overwrite ? "TRUE" : "FALSE") + ")");
#endif

  loadActive = TRUE;
  qmc2Config->beginGroup(settingsGroup);
  QString sectionTitle;
  foreach (sectionTitle, optionsMap.keys()) {
    EmulatorOption option;
    bool ok;
    int i;
    if ( qmc2GlobalEmulatorOptions != this ) {
      QTreeWidgetItem *item = sectionItemMap[sectionTitle];
      if ( item )
        if ( sectionExpansionMap[sectionTitle] )
          expandItem(item);
    }
    for (i = 0; i < optionsMap[sectionTitle].count(); i++) {
      option = optionsMap[sectionTitle][i];
      if ( qmc2GlobalEmulatorOptions != this )
        if ( optionExpansionMap[option.name] )
          expandItem(option.item);
      switch ( option.type ) {
        case QMC2_EMUOPT_TYPE_INT: {
          optionType = QMC2_EMUOPT_TYPE_INT;
          int v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value.toInt();
            else
              v = qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value.toInt()).toInt(&ok);
          } else
            v = qmc2Config->value(option.name, option.dvalue.toInt()).toInt(&ok);
          if ( ok ) {
            optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
            optionsMap[sectionTitle][i].value.sprintf("%d", v);
            optionsMap[sectionTitle][i].valid = TRUE;
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT: {
          optionType = QMC2_EMUOPT_TYPE_FLOAT;
          double v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value.toDouble();
            else
              v = qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value.toDouble()).toDouble(&ok);
          } else
            v = qmc2Config->value(option.name, option.dvalue.toDouble()).toDouble(&ok);
          if ( ok ) {
            optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
            optionsMap[sectionTitle][i].value.sprintf("%.6f", v);
            optionsMap[sectionTitle][i].valid = TRUE;
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_BOOL: {
          optionType = QMC2_EMUOPT_TYPE_BOOL;
          bool v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = EmulatorOptionDelegate::stringToBool(qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value);
            else
              v = EmulatorOptionDelegate::stringToBool(qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value).toString());
          } else
            v = EmulatorOptionDelegate::stringToBool(qmc2Config->value(option.name, option.dvalue).toString());
          optionsMap[sectionTitle][i].value = EmulatorOptionDelegate::boolToString(v);
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::ForegroundRole, QColor(0, 0, 0, 0));
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::FontRole, QFont("Helvetiva", 1));
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
          optionsMap[sectionTitle][i].valid = TRUE;
          break;
        }

        case QMC2_EMUOPT_TYPE_FILE:
        case QMC2_EMUOPT_TYPE_DIRECTORY:
        case QMC2_EMUOPT_TYPE_STRING:
        default: {
          optionType = option.type;
          QString v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value;
            else
              v = qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value).toString();
          } else
            v = qmc2Config->value(option.name, option.dvalue).toString();
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
          optionsMap[sectionTitle][i].value = v;
          optionsMap[sectionTitle][i].valid = TRUE;
          break;
        }
      }
    }
  }

  qmc2Config->endGroup();

  loadActive = changed = FALSE;
}

void EmulatorOptions::save()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::save()");
#endif

  if ( loadActive )
    return;

  if ( qmc2GlobalEmulatorOptions != this ) {
    horizontalScrollPosition = horizontalScrollBar()->sliderPosition();
    verticalScrollPosition = verticalScrollBar()->sliderPosition();
  }

  qmc2Config->beginGroup(settingsGroup);
  QString sectionTitle;
  foreach (sectionTitle, optionsMap.keys()) {
    QString vs;
    int i;
    if ( qmc2GlobalEmulatorOptions != this ) {
      QTreeWidgetItem *item = sectionItemMap[sectionTitle];
      if ( item )
        sectionExpansionMap[sectionTitle] = item->isExpanded();
    }
    for (i = 0; i < optionsMap[sectionTitle].count(); i++) {
      if ( qmc2GlobalEmulatorOptions != this )
        optionExpansionMap[optionsMap[sectionTitle][i].name] = optionsMap[sectionTitle][i].item->isExpanded();
      switch ( optionsMap[sectionTitle][i].type ) {
        case QMC2_EMUOPT_TYPE_INT: {
          int v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toInt();
          int gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toInt();
          vs.sprintf("%d", v);
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( v != optionsMap[sectionTitle][i].dvalue.toInt() ) {
              optionsMap[sectionTitle][i].value = vs;
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( v != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = vs;
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT: {
          double v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toDouble();
          double gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toDouble();
          vs.sprintf("%.6f", v);
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( v != optionsMap[sectionTitle][i].dvalue.toDouble() ) {
              optionsMap[sectionTitle][i].value = vs;
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( v != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = vs;
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_BOOL: {
          bool v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toBool();
          bool gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toBool();
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( v != EmulatorOptionDelegate::stringToBool(optionsMap[sectionTitle][i].dvalue) ) {
              optionsMap[sectionTitle][i].value = EmulatorOptionDelegate::boolToString(v);
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, EmulatorOptionDelegate::boolToString(v));
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( v != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = EmulatorOptionDelegate::boolToString(v);
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, EmulatorOptionDelegate::boolToString(v));
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_FILE:
        case QMC2_EMUOPT_TYPE_DIRECTORY:
        case QMC2_EMUOPT_TYPE_STRING:
        default: {
          QString vs = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
          QString gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( vs != optionsMap[sectionTitle][i].dvalue ) {
              optionsMap[sectionTitle][i].value = vs;
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( vs != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = vs;
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }
      }
    }
  }
  qmc2Config->endGroup();
  changed = FALSE;
}

void EmulatorOptions::createMap()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::createMap()");
#endif

  optionsMap.clear();
  sectionItemMap.clear();
  QString sectionTitle;

  foreach ( sectionTitle, templateMap.keys() ) {
    QTreeWidgetItem *sectionItem = new QTreeWidgetItem(this);
    sectionItemMap[sectionTitle] = sectionItem;
    sectionItem->setText(0, sectionTitle);
    optionsMap[sectionTitle] = templateMap[sectionTitle];
    int i;
    for (i = 0; i < optionsMap[sectionTitle].count(); i++ ) {
      optionsMap[sectionTitle][i].value = optionsMap[sectionTitle][i].dvalue;
      EmulatorOption emulatorOption = optionsMap[sectionTitle].at(i);
      QTreeWidgetItem *optionItem = new QTreeWidgetItem(sectionItem);
      optionsMap[sectionTitle][i].item = optionItem;
      optionItem->setText(0, emulatorOption.name);
      optionType = emulatorOption.type;
      QTreeWidgetItem *childItem;
      childItem = new QTreeWidgetItem(optionItem);
      childItem->setText(0, tr("Type"));
      switch ( emulatorOption.type ) {
        case QMC2_EMUOPT_TYPE_BOOL:
           childItem->setText(1, tr("bool"));
           break;

        case QMC2_EMUOPT_TYPE_INT:
           childItem->setText(1, tr("int"));
           break;

        case QMC2_EMUOPT_TYPE_FLOAT:
           childItem->setText(1, tr("float"));
           break;

        case QMC2_EMUOPT_TYPE_FILE:
           childItem->setText(1, tr("file"));
           break;

        case QMC2_EMUOPT_TYPE_DIRECTORY:
           childItem->setText(1, tr("directory"));
           break;

        case QMC2_EMUOPT_TYPE_STRING:
           childItem->setText(1, tr("string"));
           break;

        default:
           childItem->setText(1, tr("unknown"));
           break;
      }
      if ( !emulatorOption.shortname.isEmpty() ) {
        childItem = new QTreeWidgetItem(optionItem);
        childItem->setText(0, tr("Short name"));
        childItem->setText(1, emulatorOption.shortname);
      }
      if ( !emulatorOption.dvalue.isEmpty() ) {
        childItem = new QTreeWidgetItem(optionItem);
        childItem->setText(0, tr("Default"));
        if ( emulatorOption.type == QMC2_EMUOPT_TYPE_BOOL ) {
          if ( emulatorOption.dvalue == "true" )
            childItem->setText(1, tr("true"));
          else
            childItem->setText(1, tr("false"));
        } else {
          childItem->setText(1, emulatorOption.dvalue);
        }
      }
      if ( !emulatorOption.description.isEmpty() ) {
        optionDescription = emulatorOption.description;
        optionItem->setToolTip(0, optionDescription);
        childItem = new QTreeWidgetItem(optionItem);
        childItem->setText(0, tr("Description"));
        childItem->setText(1, emulatorOption.description);
      } else
        optionDescription = "";
      openPersistentEditor(optionItem, 1);
    }
  } 
}

QString EmulatorOptions::readDescription(QXmlStreamReader *xmlReader, QString lang, bool *readNext)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::readDescription(...)");
#endif

  static QString translatedDescription;

  QMap<QString, QString> translations;

  while ( !xmlReader->atEnd() && *readNext ) {
    xmlReader->readNext();
    if ( !xmlReader->hasError() ) {
      if ( xmlReader->isStartElement() ) {
        QString elementType = xmlReader->name().toString();
        if ( elementType == "description" ) {
          QXmlStreamAttributes attributes = xmlReader->attributes();
          QString language = attributes.value("lang").toString();
          QString description = attributes.value("text").toString();
          translations[language] = description;
        } else
          *readNext = FALSE;
      }
    } else
      *readNext = FALSE;
  }

  if ( translations.contains(lang) )
    translatedDescription = translations[lang];
  else if ( translations.contains("us") )
    translatedDescription = translations["us"];
  else
    translatedDescription = tr("unknown");

  return translatedDescription;
}

void EmulatorOptions::createTemplateMap()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::createTemplateMap()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("creating template configuration map"));
  
  sectionExpansionMap.clear();
  optionExpansionMap.clear();
  templateMap.clear();
  templateEmulator = tr("unknown");
  templateVersion = tr("unknown");
  templateFormat = tr("unknown");
#if defined(QMC2_EMUTYPE_MAME)
  QString templateFile = qmc2Config->value("MAME/FilesAndDirectories/OptionsTemplateFile").toString();
#elif defined(QMC2_EMUTYPE_MESS)
  QString templateFile = qmc2Config->value("MESS/FilesAndDirectories/OptionsTemplateFile").toString();
#endif
  if ( templateFile.isEmpty() )
#if defined(QMC2_SDLMAME)
    templateFile = QMC2_DEFAULT_DATA_PATH + "/opt/SDLMAME/template.xml";
#elif defined(QMC2_SDLMESS)
    templateFile = QMC2_DEFAULT_DATA_PATH + "/opt/SDLMESS/template.xml";
#elif defined(QMC2_MAME)
    templateFile = QMC2_DEFAULT_DATA_PATH + "/opt/MAME/template.xml";
#elif defined(QMC2_MESS)
    templateFile = QMC2_DEFAULT_DATA_PATH + "/opt/MESS/template.xml";
#endif
  QFile qmc2TemplateFile(templateFile);
  QString lang = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language").toString();
  if ( lang.isEmpty() )
    lang = "us";
  if ( qmc2TemplateFile.open(QFile::ReadOnly) ) {
    QXmlStreamReader xmlReader(&qmc2TemplateFile);
    QString sectionTitle;
    bool readNext = TRUE;
    while ( !xmlReader.atEnd() ) {
      if ( readNext )
        xmlReader.readNext();
      else
        readNext = TRUE;
      if ( xmlReader.hasError() ) {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: XML error reading template: '%1' in file '%2' at line %3, column %4").
                            arg(xmlReader.errorString()).arg(templateFile).arg(xmlReader.lineNumber()).arg(xmlReader.columnNumber()));
      } else {
        if ( xmlReader.isStartElement() ) {
          QString elementType = xmlReader.name().toString();
          QXmlStreamAttributes attributes = xmlReader.attributes();
          QString name = attributes.value("name").toString();
          if ( elementType == "section" ) {
            sectionTitle = readDescription(&xmlReader, lang, &readNext);
            templateMap[sectionTitle].clear();
#ifdef QMC2_DEBUG
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: elementType = [%1], name = [%2], description = [%3]").
                                arg(elementType).arg(name).arg(sectionTitle));
#endif
          } else if ( elementType == "option" ) {
            QString type = attributes.value("type").toString();
            QString defaultValue;
            if ( attributes.hasAttribute(QString("default.%1").arg(XSTR(ARCH))) )
              defaultValue = attributes.value(QString("default.%1").arg(XSTR(ARCH))).toString();
            else
              defaultValue = attributes.value("default").toString();
            QString optionDescription = readDescription(&xmlReader, lang, &readNext);
            templateMap[sectionTitle].append(EmulatorOption(name, "", type, defaultValue, optionDescription, QString::null, NULL, FALSE));
#ifdef QMC2_DEBUG
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: elementType = [%1], name = [%2], type = [%3], default = [%4], description = [%5]").
                                arg(elementType).arg(name).arg(type).arg(defaultValue).arg(optionDescription));
#endif
          } else if ( elementType == "template" ) {
            templateEmulator = attributes.value("emulator").toString();
            templateVersion = attributes.value("version").toString();
            templateFormat = attributes.value("format").toString();
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("template info: emulator = %1, version = %2, format = %3").arg(templateEmulator).arg(templateVersion).arg(templateFormat));
          }
        }
      }
    }
    qmc2TemplateFile.close();
  } else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open options template file"));

  if ( templateEmulator == tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator type of template"));
  if ( templateVersion == tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine template version"));
  if ( templateFormat == tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine template format"));
}

void EmulatorOptions::checkTemplateMap()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::checkTemplateMap()");
#endif

  if ( qmc2ReloadActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return;
  }

  QString userScopePath = QMC2_DOT_PATH;

  int diffCount = 0;
  QMap <QString, QString> emuOptions;

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking template configuration map against selected emulator"));

  QStringList args;
  QProcess commandProc;
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile").toString());
#if !defined(Q_WS_WIN)
  commandProc.setStandardErrorFile("/dev/null");
#endif

  args << "-noreadconfig" << "-showconfig";
  qApp->processEvents();
#if defined(QMC2_EMUTYPE_MAME)
  commandProc.start(qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString(), args);
#elif defined(QMC2_EMUTYPE_MESS)
  commandProc.start(qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString(), args);
#endif
  bool commandProcStarted = FALSE;
  if ( commandProc.waitForStarted() ) {
    commandProcStarted = TRUE;
    bool commandProcRunning = (commandProc.state() == QProcess::Running);
    while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
      qApp->processEvents();
      commandProcRunning = (commandProc.state() == QProcess::Running);
    }
  }

#if defined(QMC2_SDLMAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#else
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif
  if ( commandProcStarted && qmc2Temp.open(QFile::ReadOnly) ) {
    QTextStream ts(&qmc2Temp);
    QString s = ts.readAll();
    qmc2Temp.close();
    qmc2Temp.remove();
#if !defined(Q_WS_WIN)
    QStringList sl = s.split("\n");
#else
    QStringList sl = s.split("\r\n");
#endif
    foreach (QString line, sl) {
      QString l = line.simplified();
      if ( l.isEmpty() ) continue;
      if ( l.startsWith("#") ) continue;
      if ( l.startsWith("<") ) continue;
      if ( l.startsWith("readconfig") ) continue;
#if defined(QMC2_SDLMESS)
      // this is a 'non-SDL Windows MESS'-only option and is ignored for all SDLMESS builds
      if ( l.startsWith("newui") ) continue;
#endif
      QStringList wl;
      QRegExp rx("(\\S+|\\\".*\\\")");
      int pos = 0;
      while ( (pos = rx.indexIn(l, pos)) != -1 ) {
        QString s = rx.cap(1);
        s = s.remove("\"");
        wl << s;
        pos += rx.matchedLength();
      }
      if ( wl.count() == 2 )
        emuOptions[wl[0]] = wl[1];
      else if ( wl.count() == 1 )
        emuOptions[wl[0]] = "";
      else
        continue;
#ifdef QMC2_DEBUG
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: option [%1], default value [%2]").arg(wl[0]).arg(emuOptions[wl[0]]));
#endif
    }
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create temporary file, please check emulator executable and permissions"));
  }
  
  QStringList templateOptions;
  foreach (QString sectionTitle, optionsMap.keys()) {
    EmulatorOption option;
    int i;
    for (i = 0; i < optionsMap[sectionTitle].count(); i++) {
      option = optionsMap[sectionTitle][i];
      templateOptions << option.name;
      if ( emuOptions.contains(option.name) ) {
        switch ( option.type ) {
          case QMC2_EMUOPT_TYPE_INT:
            if ( option.dvalue.toInt() != emuOptions[option.name].toInt() ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue.toInt()).arg(emuOptions[option.name].toInt()).arg(tr("int")));
            }
            break;

          case QMC2_EMUOPT_TYPE_FLOAT:
            if ( option.dvalue.toDouble() != emuOptions[option.name].toDouble() ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue.toDouble()).arg(emuOptions[option.name].toDouble()).arg(tr("float")));
            }
            break;

          case QMC2_EMUOPT_TYPE_BOOL: {
            QString emuOpt = "true";
            if ( emuOptions[option.name] == "0" )
              emuOpt = "false";
            if ( option.dvalue != emuOpt ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue).arg(emuOptions[option.name]).arg(tr("float")));
            }
            break;
          }

          case QMC2_EMUOPT_TYPE_FILE:
          case QMC2_EMUOPT_TYPE_DIRECTORY:
          case QMC2_EMUOPT_TYPE_STRING:
          default:
            if ( option.dvalue.replace("$HOME", "~") != emuOptions[option.name].replace("$HOME", "~") ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue).arg(emuOptions[option.name]).arg(tr("float")));
            }
            break;
        }
      } else if ( option.name != "readconfig" ) {
        diffCount++;
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("template option '%1' is unknown to the emulator").arg(option.name));
      }
    }
  }

  QMapIterator<QString, QString> it(emuOptions);
  while ( it.hasNext() ) {
    it.next();
    if ( !templateOptions.contains(it.key()) ) {
      diffCount++;
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator option '%1' with default value '%2' is unknown to the template").arg(it.key()).arg(it.value()));
    }
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking template configuration map against selected emulator)"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check results: %n difference(s)", "", diffCount));
}

void EmulatorOptions::keyPressEvent(QKeyEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::keyPressEvent(QKeyPressEvent *e)");
#endif

  if ( e->key() == Qt::Key_Escape ) {
    if ( lineEditSearch ) {
      if ( lineEditSearch->isVisible() )
        searchTimeout();
      else
        QAbstractItemView::keyPressEvent(e);
    } else
      QAbstractItemView::keyPressEvent(e);
    return;
  }

  if ( !e->text().isEmpty() ) {
    if ( !lineEditSearch )
      lineEditSearch = new QLineEdit(this);
    lineEditSearch->show();
    lineEditSearch->raise();
    lineEditSearch->event(e);
    searchTimer.start(QMC2_SEARCH_TIMEOUT);
    clearSelection();
    if ( !lineEditSearch->text().isEmpty() ) {
      int i;
      bool madeCurrent = FALSE;
      QList<QTreeWidgetItem *> foundItems = findItems(lineEditSearch->text(), Qt::MatchRecursive | Qt::MatchContains, 0);
      for (i = 0; i < foundItems.count(); i++) {
        QTreeWidgetItem *p = foundItems[i];
        if ( p->parent() ) {
          if ( !p->parent()->parent() ) {
            p->setSelected(TRUE);
            p->parent()->setSelected(TRUE);
            if ( !madeCurrent ) {
              scrollToItem(p);
              setCurrentItem(p);
              madeCurrent = TRUE;
            }
          }
        }
      }
    }
    e->accept();
  } else {
    QAbstractItemView::keyPressEvent(e);
  }
}

void EmulatorOptions::searchTimeout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::searchTimeout()");
#endif

  searchTimer.stop();
  lineEditSearch->hide();
  lineEditSearch->close();
  delete lineEditSearch;
  lineEditSearch = NULL;
}

void EmulatorOptions::exportToIni(bool global, QString useFileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptions::exportToIni(QString useFileName = %1)").arg(useFileName));
#endif

  static QBrush redBrush(QColor(255, 0, 0));
  static QBrush greenBrush(QColor(0, 255, 0));

#if defined(QMC2_EMUTYPE_MAME)
  QStringList iniPaths = qmc2Config->value("MAME/Configuration/Global/inipath").toString().split(";");
#elif defined(QMC2_EMUTYPE_MESS)
  QStringList iniPaths = qmc2Config->value("MESS/Configuration/Global/inipath").toString().split(";");
#endif
  QStringList writableIniPaths;
  QStringList fileNames;

  // determine list of writable ini-paths
  foreach (QString path, iniPaths) {
    QString pathCopy(path);
    QFileInfo dirInfo(pathCopy.replace("~", QDir::homePath()).replace("$HOME", QDir::homePath()));
    if ( dirInfo.isWritable() && dirInfo.isExecutable() )
      writableIniPaths << path;
  }

  // get correct filename(s) for ini-export
  QString fileName;
  if ( useFileName.isEmpty() ) {
    int i;
    if ( writableIniPaths.count() == 1 )
      iniPaths = writableIniPaths;
    else if ( writableIniPaths.count() < 1 )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ini-export: no writable ini-paths found"));
    if ( iniPaths.count() > 1 ) {
      // multiple ini-paths detected - let user select one or more ini-paths to export to, pre-select all dirs
      ItemSelector itemSelector(this, iniPaths);
      itemSelector.setWindowTitle(tr("Path selection"));
      itemSelector.labelMessage->setText(tr("Multiple ini-paths detected. Select path(s) to export to:"));
      itemSelector.listWidgetItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
      foreach (QListWidgetItem *item, itemSelector.listWidgetItems->findItems("*", Qt::MatchWildcard)) {
        if ( writableIniPaths.contains(item->text()) ) {
          item->setForeground(greenBrush);
          item->setSelected(TRUE);
        } else {
          item->setForeground(redBrush);
          item->setSelected(FALSE);
        }
      }
      if ( itemSelector.exec() == QDialog::Rejected )
        return;
      QList<QListWidgetItem *> itemList = itemSelector.listWidgetItems->selectedItems();
      iniPaths.clear();
      for (i = 0; i < itemList.count(); i++)
        iniPaths << itemList[i]->text();
    }
    if ( iniPaths.count() < 1 ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ini-export: no path selected (or invalid inipath)"));
      return;
    }
    if ( qmc2GlobalEmulatorOptions == this ) {
#if defined(QMC2_EMUTYPE_MAME)
      fileName = "/mame.ini";
#elif defined(QMC2_EMUTYPE_MESS)
      fileName = "/mess.ini";
#endif
    } else {
      if ( !qmc2CurrentItem )
        return;
      if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
        return;
      fileName = "/" + qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON) + ".ini";
    }
    for (i = 0; i < iniPaths.count(); i++) {
      QString fn(fileName);
      fn = fn.prepend(iniPaths[i]);
      fileNames << fn.replace("~", QDir::homePath()).replace("$HOME", QDir::homePath());
    }
  } else {
    fileNames << useFileName;
  }
  
  foreach (fileName, fileNames) {
    QFile iniFile(fileName);
    if ( !iniFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open export file for writing, path = %1").arg(fileName));
      continue;
    }
    QTime elapsedTime;
    miscTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting %1 MAME configuration to %2").arg(global ? tr("global") : tr("game-specific")).arg(fileName));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting %1 MESS configuration to %2").arg(global ? tr("global") : tr("machine-specific")).arg(fileName));
#endif
    QTextStream ts(&iniFile);
    QString sectionTitle;
    qmc2Config->beginGroup(settingsGroup);
    foreach ( sectionTitle, optionsMap.keys() ) {
      QString vs;
      int i;
      for (i = 0; i < optionsMap[sectionTitle].count(); i++) {
        switch ( optionsMap[sectionTitle][i].type ) {
          case QMC2_EMUOPT_TYPE_INT: {
            int v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toInt();
            if ( qmc2GlobalEmulatorOptions == this )
              ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            else
              ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            break;
          }

          case QMC2_EMUOPT_TYPE_FLOAT: {
            double v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toDouble();
            if ( qmc2GlobalEmulatorOptions == this )
              ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            else
              ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            break;
          }

          case QMC2_EMUOPT_TYPE_BOOL: {
            bool v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toBool();
            if ( qmc2GlobalEmulatorOptions == this )
              ts << optionsMap[sectionTitle][i].name << " " << (v ? "1" : "0") << "\n";
            else
              ts << optionsMap[sectionTitle][i].name << " " << (v ? "1" : "0") << "\n";
            break;
          }

          case QMC2_EMUOPT_TYPE_FILE:
          case QMC2_EMUOPT_TYPE_DIRECTORY:
          case QMC2_EMUOPT_TYPE_STRING:
          default: {
            QString v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
            v = v.replace("~", "$HOME");
            if ( qmc2GlobalEmulatorOptions == this )
              ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            else
              ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            break;
          }
        }
      }
    }
    qmc2Config->endGroup();
    iniFile.close();
    elapsedTime = elapsedTime.addMSecs(miscTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting %1 MAME configuration to %2, elapsed time = %3)").arg(global ? tr("global") : tr("game-specific")).arg(fileName).arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting %1 MESS configuration to %2, elapsed time = %3)").arg(global ? tr("global") : tr("machine-specific")).arg(fileName).arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  }
}

void EmulatorOptions::importFromIni(bool global, QString useFileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptions::importFromIni(QString useFileName = %1)").arg(useFileName));
#endif

  static QBrush redBrush(QColor(255, 0, 0));
  static QBrush greenBrush(QColor(0, 255, 0));

#if defined(QMC2_EMUTYPE_MAME)
  QStringList iniPaths = qmc2Config->value("MAME/Configuration/Global/inipath").toString().split(";");
#elif defined(QMC2_EMUTYPE_MESS)
  QStringList iniPaths = qmc2Config->value("MESS/Configuration/Global/inipath").toString().split(";");
#endif
  QStringList readableIniPaths;
  QStringList fileNames;

  // determine list of readable ini-paths
  foreach (QString path, iniPaths) {
    QString pathCopy(path);
    QFileInfo dirInfo(pathCopy.replace("~", QDir::homePath()).replace("$HOME", QDir::homePath()));
    if ( dirInfo.isReadable() && dirInfo.isExecutable() )
      readableIniPaths << path;
  }

  // get correct filename for ini-import
  QString fileName;
  if ( useFileName.isEmpty() ) {
    int i;
    if ( readableIniPaths.count() == 1 )
      iniPaths = readableIniPaths;
    else if ( readableIniPaths.count() < 1 )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ini-import: no readable ini-paths found"));
    if ( iniPaths.count() > 1 ) {
      // multiple ini-paths detected - let user select the ini-path to import from, pre-select first dir
      ItemSelector itemSelector(this, iniPaths);
      itemSelector.setWindowTitle(tr("Path selection"));
      itemSelector.labelMessage->setText(tr("Multiple ini-paths detected. Select path(s) to import from:"));
      itemSelector.listWidgetItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
      foreach (QListWidgetItem *item, itemSelector.listWidgetItems->findItems("*", Qt::MatchWildcard)) {
        if ( readableIniPaths.contains(item->text()) ) {
          item->setForeground(greenBrush);
          item->setSelected(TRUE);
        } else {
          item->setForeground(redBrush);
          item->setSelected(FALSE);
        }
      }
      if ( itemSelector.exec() == QDialog::Rejected )
        return;
      QList<QListWidgetItem *> itemList = itemSelector.listWidgetItems->selectedItems();
      iniPaths.clear();
      for (i = 0; i < itemList.count(); i++)
        iniPaths << itemList[i]->text();
    }
    if ( iniPaths.count() < 1 ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ini-import: no path selected (or invalid inipath)"));
      return;
    }
    if ( qmc2GlobalEmulatorOptions == this ) {
#if defined(QMC2_EMUTYPE_MAME)
      fileName = "/mame.ini";
#elif defined(QMC2_EMUTYPE_MESS)
      fileName = "/mess.ini";
#endif
    } else {
      if ( !qmc2CurrentItem )
        return;
      if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
        return;
      QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
      fileName = "/" + gameName + ".ini";
    }
    for (i = 0; i < iniPaths.count(); i++) {
      QString fn(fileName);
      fn = fn.prepend(iniPaths[i]);
      fileNames << fn.replace("~", QDir::homePath()).replace("$HOME", QDir::homePath());
    }
  } else {
    fileNames << useFileName;
  }
  
  foreach (fileName, fileNames) {
    QFile iniFile(fileName);
    if ( !iniFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open import file for reading, path = %1").arg(fileName));
      continue;
    }
    QTime elapsedTime;
    miscTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("importing %1 MAME configuration from %2").arg(global ? tr("global") : tr("game-specific")). arg(fileName));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("importing %1 MESS configuration from %2").arg(global ? tr("global") : tr("machine-specific")). arg(fileName));
#endif
    QTextStream ts(&iniFile);

    // read ini-file and set corresponding emulator options to their values
    int lineCounter = 0;
    while ( !ts.atEnd() ) {
      qApp->processEvents();
      QString line = ts.readLine();
      QString lineSimple = line.simplified();
      lineCounter++;
      if ( !line.isEmpty() && !lineSimple.startsWith("#") ) {
        QStringList words = lineSimple.split(" ");
        if ( words.count() > 0 ) {
          QString option = words[0];

          // lookup option in map
          QString sectionTitle, sectionTitleFound = "";
          EmulatorOption mameOpt;
          int optionPos, optionPosFound = -1;
          foreach ( sectionTitle, optionsMap.keys() ) {
            for (optionPos = 0; optionPos < optionsMap[sectionTitle].count() && optionPosFound == -1; optionPos++ ) {
              if ( optionsMap[sectionTitle][optionPos].name == option ) {
                sectionTitleFound = sectionTitle;
                optionPosFound = optionPos;
              }
            }
            if ( optionPosFound != -1 )
              break;
          }

          if ( optionPosFound != -1 && words.count() > 1 ) {
            QString value = words[1];
            switch ( optionsMap[sectionTitleFound][optionPosFound].type ) {
              case QMC2_EMUOPT_TYPE_INT: {
                if ( qmc2GlobalEmulatorOptions == this )
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.toInt());
                else
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.toInt());
                break;
              }

              case QMC2_EMUOPT_TYPE_FLOAT: {
                if ( qmc2GlobalEmulatorOptions == this )
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.toDouble());
                else
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.toDouble());
                break;
              }

              case QMC2_EMUOPT_TYPE_BOOL: {
                if ( qmc2GlobalEmulatorOptions == this ) {
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::ForegroundRole, QColor(0, 0, 0, 0));
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::FontRole, QFont("Helvetiva", 1));
                  if ( value == "0" )
                    qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, FALSE);
                  else
                    qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, TRUE);
                } else {
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::ForegroundRole, QColor(0, 0, 0, 0));
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::FontRole, QFont("Helvetiva", 1));
                  if ( value == "0" )
                    qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, FALSE);
                  else
                    qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, TRUE);
                }
                break;
              }

              case QMC2_EMUOPT_TYPE_FILE:
              case QMC2_EMUOPT_TYPE_DIRECTORY:
              case QMC2_EMUOPT_TYPE_STRING:
              default: {
                if ( qmc2GlobalEmulatorOptions == this )
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.replace("$HOME", "~"));
                else
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.replace("$HOME", "~"));
                break;
              }
            }
          } else if ( optionPosFound == -1 ) {
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: unknown option '%1' at line %2 (%3) ignored").arg(option).arg(lineCounter).arg(fileName));
          }
        } else {
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: invalid syntax at line %1 (%2) ignored").arg(lineCounter).arg(fileName));
        }
      }
    }
    iniFile.close();
    elapsedTime = elapsedTime.addMSecs(miscTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (importing %1 MAME configuration from %2, elapsed time = %3)").arg(global ? tr("global") : tr("game-specific")).arg(fileName).arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (importing %1 MESS configuration from %2, elapsed time = %3)").arg(global ? tr("global") : tr("machine-specific")).arg(fileName).arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  }
}
