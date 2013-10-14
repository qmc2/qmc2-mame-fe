#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QtGui>

#include "projectwidget.h"
#include "scriptengine.h"

class ScriptEngine;

namespace Ui {
class ScriptWidget;
}

class ScriptWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ScriptWidget(QWidget *parent = 0);
    virtual ~ScriptWidget();

public slots:
    void on_toolButtonRun_clicked();
    void on_toolButtonStop_clicked();
    void log(QString);

private:
    Ui::ScriptWidget *ui;
    ScriptEngine *scriptEngine;
};

#endif // SCRIPTWIDGET_H
