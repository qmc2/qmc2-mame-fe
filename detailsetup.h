#ifndef _DETAILSETUP_H_
#define _DETAILSETUP_H_

#include <QMap>
#include "ui_detailsetup.h"

class DetailSetup : public QDialog, public Ui::DetailSetup
{
  Q_OBJECT

  public:
    QMap<int, QString> shortTitleMap;
    QMap<int, QString> longTitleMap;
    QList<int> activeDetailList;

    DetailSetup(QWidget *parent = 0);
    ~DetailSetup();

  public slots:
    void loadDetail();
    void saveDetail();

    // callbacks
    void on_listWidgetAvailableDetails_itemSelectionChanged(); 
    void on_listWidgetActiveDetails_itemSelectionChanged(); 
};

#endif
