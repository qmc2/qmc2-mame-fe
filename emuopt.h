#ifndef _EMUOPT_H_
#define _EMUOPT_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QComboBox>
#include <QTreeWidget>
#include <QSettings>
#include <QMap>
#include <QTimer>
#include <QTime>
#include <QKeyEvent>
#include <QXmlStreamReader>

#include "macros.h"

class EmulatorOptionDelegate : public QItemDelegate
{
  Q_OBJECT

  public:
    EmulatorOptionDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;
    virtual void setEditorData(QWidget *, const QModelIndex &) const;
    virtual void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;
    virtual void updateEditorGeometry(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;

  public slots:
    void dataChanged();
};

class EmulatorOption
{
  public:
    QString name, shortname, dvalue, description, value;
    bool valid;
    int type;
    QTreeWidgetItem *item;

    EmulatorOption() {
      valid = FALSE;
    };
    EmulatorOption(QString n, QString sn, QString t, QString dv, QString d, QString v, QTreeWidgetItem *i, bool va) {
      name = n;
      shortname = sn;
      switch ( t.at(0).toAscii() ) {
        case 'b': type = QMC2_EMUOPT_TYPE_BOOL; break;
        case 'i': type = QMC2_EMUOPT_TYPE_INT; break;
        case 'f': if ( t.at(1).toAscii() == 'l' ) type = QMC2_EMUOPT_TYPE_FLOAT; else type = QMC2_EMUOPT_TYPE_FILE; break;
        case 's': type = QMC2_EMUOPT_TYPE_STRING; break;
        case 'd': type = QMC2_EMUOPT_TYPE_DIRECTORY; break;
        default: type = QMC2_EMUOPT_TYPE_UNKNOWN; break;
      }
      dvalue = dv;
      description = d;
      value = v;
      item = i;
      valid = va;
    }
};

class EmulatorOptions : public QTreeWidget
{
  Q_OBJECT

  public:
    QTimer searchTimer;
    QTime miscTimer;
    QLineEdit *lineEditSearch;
    EmulatorOptionDelegate *delegate;
    QString settingsGroup;
    QString templateEmulator;
    QString templateVersion;
    QString templateFormat;
    QMap<QString, QList<EmulatorOption> > optionsMap;
    static QMap<QString, QList<EmulatorOption> > templateMap;
    bool loadActive;
    bool changed;

    EmulatorOptions(QString, QWidget *parent = 0);
    ~EmulatorOptions();

    void pseudoConstructor();
    void pseudoDestructor();
    QString readDescription(QXmlStreamReader *, QString, bool *);

  public slots:
    void load(bool overwrite = FALSE);
    void save();
    void createTemplateMap();
    void createMap();
    void searchTimeout();
    void exportToIni(bool global, QString useFileName = QString());
    void importFromIni(bool global, QString useFileName = QString());

  protected:
    virtual void keyPressEvent(QKeyEvent *);
};

#endif
