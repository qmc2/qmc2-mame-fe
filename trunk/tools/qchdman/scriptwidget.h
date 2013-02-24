#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QWidget>
#include "projectwidget.h"

namespace Ui {
class ScriptWidget;
}

class ScriptWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ScriptWidget(QWidget *parent = 0);
    ~ScriptWidget();

    void doCleanUp();
    
private:
    Ui::ScriptWidget *ui;
};

#endif // SCRIPTWIDGET_H
