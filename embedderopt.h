#ifndef _EMBEDDEROPT_H_
#define _EMBEDDEROPT_H_

#include "ui_embedderopt.h"

class EmbedderOptions : public QWidget, public Ui::EmbedderOptions
{
  Q_OBJECT

  public:
    EmbedderOptions(QWidget *parent = 0);
    ~EmbedderOptions();

  public slots:
};

#endif
