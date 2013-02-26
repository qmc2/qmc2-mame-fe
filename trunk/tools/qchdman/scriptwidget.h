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

public slots:
    // Callbacks
    void on_toolButtonInputOutput_toggled(bool enable);
    void on_toolButtonRun_clicked();
    void on_toolButtonStop_clicked();
    void on_toolButtonAddGroup_clicked();
    void on_toolButtonRemoveGroup_clicked();
    void on_toolButtonAddProject_clicked();
    void on_toolButtonRemoveProject_clicked();
    void on_toolButtonAddCommand_clicked();
    void on_toolButtonRemoveCommand_clicked();

    // Other
    void doCleanUp();

protected:
    void resizeEvent(QResizeEvent *);

private:
    Ui::ScriptWidget *ui;
    int groupSeqNum, projectSeqNum, commandSeqNum;
    bool inputOutputTableShownInitially;
};

#endif // SCRIPTWIDGET_H
